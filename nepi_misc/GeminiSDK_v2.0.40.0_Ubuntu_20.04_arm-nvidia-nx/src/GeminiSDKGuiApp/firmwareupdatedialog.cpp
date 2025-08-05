#include "firmwareupdatedialog.h"
#include "ui_firmwareupdatedialog.h"

#include <QtWidgets/QPushButton>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <QtCore/QDebug>


FirmwareUpdateDialog::FirmwareUpdateDialog(const QString& message,
                                           QWidget     *parent,
                                           bool         nologo,
                                           unsigned int rebootTimeout,
                                           unsigned int rebootDelay)
:   QDialog(parent)
,   ui(new Ui::FirmwareUpdateDialog)
,   m_noLogo( nologo )
,   m_rebootTimeout(rebootTimeout)
,   m_rebootDelay(rebootDelay)
,   m_step(0)
,   m_updateSteps (0)
,   m_status (console::FirmwareUpgradeStatus::FW_UG_IDLE)
,   m_isCancelled(false)
,   m_deviceID( 0 )
{
    ui->setupUi(this);


    // message comes in the form
    // ProductName; Device ID; current firmware version: New firmware version; update steps
    QStringList formats = QString(message).split(';');

    m_label = "Gemini " + formats[ 0 ] + " - " + formats[ 1 ];
    m_deviceID = formats[ 1 ].toUShort();
    m_updateSteps = formats[ 4 ].toInt();


    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowCloseButtonHint);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &FirmwareUpdateDialog::onAccept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &FirmwareUpdateDialog::onCancel);

    connect(&m_timer, &QTimer::timeout, this, &FirmwareUpdateDialog::onTimer);
    m_timer.setSingleShot(true);

    if ( m_step >= m_updateSteps)
    {
        QString msg = "No firmware update is available for: " + m_label + ".";
        ui->label_Message->setText(msg);

        showControls(false, false, false);
        showVersion(false, false);

        ui->buttonBox->button(QDialogButtonBox::Cancel)->hide();
    }
    else
    {
        QString msg ("Would you like to update the firmware for: " + m_label + "?");
        ui->label_Message->setText(msg);

        showControls(false, false, false);

        ui->label_FromVersion->setText(formats[ 2 ]);// Current Version
        ui->label_ToVersion->setText(formats[ 3 ]);//  New Version
        showVersion(ui->label_FromVersion->text() != "0.0",
                    ui->label_ToVersion->text() != "0.0");

        // Not asking for confirmation and doing the firmware upgrade
        if( m_noLogo )
        {
            m_timer.start(1000);
        }
    }
}

FirmwareUpdateDialog::~FirmwareUpdateDialog()
{
    delete ui;
}

void FirmwareUpdateDialog::onAccept()
{
    if (!updateFirmware())
    {
        done(QDialog::Accepted);
    }
}

void FirmwareUpdateDialog::onCancel()
{
    switch(m_status)
    {
    case console::FirmwareUpgradeStatus::FW_UG_IDLE:
    {
        done(QDialog::Rejected);
        break;
    }
    case console::FirmwareUpgradeStatus::FW_UG_PROGRAMME:
    {
        m_isCancelled = true;

        // cancel during an update
        emit(abortFirmwareUpgrade(m_deviceID));
        m_status = console::FirmwareUpgradeStatus::FW_UG_IDLE;

        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
        break;
    }
    default:
        break;
    };
}

void FirmwareUpdateDialog::onTimer()
{
    switch(m_status)
    {
    case console::FirmwareUpgradeStatus::FW_UG_IDLE:
    {
        emit(ui->buttonBox->accepted());
        break;
    }
    case console::FirmwareUpgradeStatus::FW_UG_PROGRAMME:
    {
        // console communication has timed out
        emit(abortFirmwareUpgrade(m_deviceID));

        ui->label_Message->setText("The firmware update has timed out and cannot be completed.");
        ui->progressBar->hide();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        break;
    }
    case console::FirmwareUpgradeStatus::FW_UG_SUCCESS:
    {
        break;
    }
    case console::FirmwareUpgradeStatus::FW_UG_REBOOTING:
    {
        if (ui->progressBar->value() < ui->progressBar->maximum())
        {
            ui->progressBar->setValue(ui->progressBar->value() + kRebootIntervalTimeout);
            m_timer.start(kRebootIntervalTimeout);
        }
        else
        {
            rebootFailed();
        }

        break;
    }
    default:
        break;
    };
}

void FirmwareUpdateDialog::onFirmwareStatus(const QString& status, unsigned int percent, int state)
{
    //qDebug() << "Firmware Status:" << status << percent << state;

    ui->label_Status->setText(status);
    ui->progressBar->setValue(percent);

    m_status = static_cast<console::FirmwareUpgradeStatus::FW_UG_STATUS>(state);

    switch(m_status)
    {
    case console::FirmwareUpgradeStatus::FW_UG_PROGRAMME:
    {
        m_timer.start(kProgrammingTimeout);
        if (!m_isCancelled)
        {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);
        }
        break;
    }
    case console::FirmwareUpgradeStatus::FW_UG_FAILURE:
    {
        // ignore messages if user has cancelled
        if (m_isCancelled)
        {
            ui->label_Message->setText("The firmware update for " + m_label + " has been cancelled.");
            showControls(false, false, false);
            showVersion(false, false);

            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            ui->buttonBox->button(QDialogButtonBox::Cancel)->hide();
        }
        else
        {
            ui->label_Message->setText("The firmware update has failed for " + m_label + ".");
            ui->progressBar->hide();
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            ui->buttonBox->button(QDialogButtonBox::Cancel)->hide();
        }

        break;
    }
    case console::FirmwareUpgradeStatus::FW_UG_SUCCESS:
    {
        if (m_isCancelled)
        {
            // protect against the cancel being too late in the update
            m_isCancelled = false;
        }

        // attempt the next update step
        if (!updateFirmware())
        {
            // initiate the reboot if this was the last step
            QString msg = "The firmware has updated successfully. " + m_label + " will now reboot.";
            ui->label_Message->setText(msg);

            ui->progressBar->setMaximum(m_rebootTimeout);
            ui->progressBar->setValue(0);
            ui->label_Status->setText("Preparing to reboot");
            showControls(true, true, true);

            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);

            m_timer.start(m_rebootDelay);
        }

        break;
    }
    case console::FirmwareUpgradeStatus::FW_UG_REBOOTING:
    {
        ui->label_Status->setText("Rebooting...");
        m_timer.start(kRebootIntervalTimeout);
        break;
    }
    default:
        break;
    };
}

void FirmwareUpdateDialog::onRebootComplete()
{
    if (console::FirmwareUpgradeStatus::FW_UG_REBOOTING == m_status && m_timer.isActive())
    {
        m_timer.stop();

        QString msg = "The firmware update has completed successfully for " + m_label;
        ui->label_Message->setText(msg);

        showControls(false, false, false);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);

        // Hide dialog box after 1 second
        if( m_noLogo )
        {
            QThread::sleep(1);
            emit(ui->buttonBox->accepted());
        }
    }
}

bool FirmwareUpdateDialog::updateFirmware()
{
    if (m_step < m_updateSteps && !m_isCancelled &&
        console::FirmwareUpgradeStatus::FW_UG_FAILURE != m_status)
    {
        QString msg ("Updating firmware for: " + m_label + ".");
        ui->label_Message->setText(msg);

        msg.clear();
        QTextStream (&msg) << "Step " << (m_step + 1) << " of " << m_updateSteps;
        ui->label_Step->setText(msg);

        ui->label_Status->clear();
        ui->progressBar->setValue(0);

        showControls(true, true, true);

        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);

        m_timer.start(kProgrammingTimeout);
        if( !m_step )
        {
            emit(upgradeFirmware( m_deviceID ) );
        }
        m_step++;
        return true;
    }

    return false;
}

void FirmwareUpdateDialog::rebootFailed()
{
    ui->label_Message->setText("The firmware update for " + m_label +
        " was successful but the device did not reboot properly. "
        " Please power cycle the device if it does not come online.");
    ui->label_Status->clear();
    showControls(false, false, false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void FirmwareUpdateDialog::showControls(bool step, bool progress, bool status)
{
    ui->label_Step->setVisible(step);
    ui->progressBar->setVisible(progress);
    ui->label_Status->setVisible(status);
}

void FirmwareUpdateDialog::showVersion(bool from, bool to)
{
    ui->label_From->setVisible(from);
    ui->label_FromVersion->setVisible(from);
    ui->label_To->setVisible(to);
    ui->label_ToVersion->setVisible(to);
}
