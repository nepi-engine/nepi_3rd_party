#include "geometry.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gemininetwork.h"
#include "firmwareupdatedialog.h"

#include <QtCore/QDebug>
#include <QtCore/QMetaType>
#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>


Q_DECLARE_METATYPE(GLF::GLogTargetImage)
#ifdef _ECD_LOGGER_
Q_DECLARE_METATYPE(CTgtImg)
#endif

MainWindow::MainWindow(bool transparent, bool frameless, QWidget *parent)
:   QMainWindow(parent)
,   ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if (frameless)
    {
        setWindowFlags(Qt::FramelessWindowHint);
    }
    if (transparent)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground, false);

        ui->statusBar->hide();
    }

    // register data types to use in Qt signals
    qRegisterMetaType<GLF::GLogTargetImage>();
#ifdef _ECD_LOGGER_
    qRegisterMetaType<CTgtImg>();
#endif
    ui->toolButton_Stop->setEnabled(false);
    ui->toolButton_Online->setEnabled(true);
    ui->toolButton_Open->setEnabled(false);

    m_logDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    m_network = new GeminiNetwork;

    ui->openGLWidget->setRange(ui->doubleSpinBox_Range->value());
    ui->openGLWidget->enableText(ui->toolButton_Text->isChecked());
    ui->openGLWidget->enableGrid(ui->toolButton_Grid->isChecked());

    // initialise the network from GUI values
    m_network->onSetRange(ui->doubleSpinBox_Range->value());
    m_network->onSetGain(ui->spinBox_Gain->value());

    connect(m_network, &GeminiNetwork::infoMessage, ui->statusBar, &QStatusBar::showMessage);
    connect(ui->openGLWidget, &OpenGLWidget::infoMessage, ui->statusBar, &QStatusBar::showMessage);

    // connect GUI to gemini data signals
    connect(m_network, &GeminiNetwork::glfImageReceived, ui->openGLWidget, &OpenGLWidget::onGlfGeminiImage);
#ifdef _ECD_LOGGER_
    connect(m_network, &GeminiNetwork::ecdImageReceived, ui->openGLWidget, &OpenGLWidget::onEcdGeminiImage);
#endif
    connect(ui->openGLWidget, &OpenGLWidget::rangeChanged, this, &MainWindow::onRangeChanged);
    connect(ui->openGLWidget, &OpenGLWidget::sizeChanged, m_network, &GeminiNetwork::onSizeChanged);
    connect(ui->compassWidget, &CompassWidget::sizeChanged, m_network, &GeminiNetwork::onSizeChanged);
    connect(m_network, &GeminiNetwork::clearImage, ui->openGLWidget, &OpenGLWidget::clearImage);    
    connect(m_network, &GeminiNetwork::firmwareUpgrade, this, &MainWindow::onFirmwareUpgrade);
    connect(m_network, &GeminiNetwork::geminiConnectionChanged, this, &MainWindow::onConnectionChanged);
    connect(m_network, &GeminiNetwork::handleAHRSRawData, this, &MainWindow::onAHRSRawXYZData);
    connect(m_network, &GeminiNetwork::handleAHRSHPRData, this, &MainWindow::onAHRSRawHPRData);
    connect(m_network, &GeminiNetwork::handleAHRSHPRData, ui->compassWidget, &CompassWidget::onCompassData);
#ifdef _DATA_LOGGER_
    // Start logging by default
    onStartStoplog();

#ifdef WIN32
    if(!RegisterHotKey(HWND(winId()), 0, MOD_CONTROL, Qt::Key_C))
    {
        qDebug() << "Can't register hotkey Qt::Key_C pressed";
    }

    if(!RegisterHotKey(HWND(winId()), 1, MOD_CONTROL, Qt::Key_D))
    {
        qDebug() << "Can't register hotkey Qt::Key_D pressed";
    }

    if(!RegisterHotKey(HWND(winId()), 2, MOD_CONTROL, Qt::Key_L))
    {
        qDebug() << "Can't register hotkey Qt::Key_L pressed";
    }
#endif
#endif
    showMaximized();

    ui->doubleSpinBox_Range->installEventFilter(this);
    m_mouseDown = false;
}

MainWindow::~MainWindow()
{
    delete m_network;
    delete ui;
}

bool MainWindow::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == ui->doubleSpinBox_Range)
    {
        switch (ev->type())
        {
            case QEvent::MouseButtonPress:
            {
                m_mouseDown = true;
            }
            break;
            case QEvent::MouseButtonRelease:
            {
                m_mouseDown = false;
            }
            break;
            default:
            break;
        }
    }

    return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::on_doubleSpinBox_Range_valueChanged(double value)
{
    m_network->onSetRange(value);
    ui->openGLWidget->setRange(value);
}

void MainWindow::on_spinBox_Gain_valueChanged(int value)
{
    m_network->onSetGain(value);
}

void MainWindow::on_toolButton_Record_toggled(bool checked)
{
    if (checked)
    {
        m_network->startRecord(m_logDirectory);
        ui->toolButton_Open->setEnabled(false);
    }
    else
    {
        m_network->stopRecord();
        ui->toolButton_Open->setEnabled(true);
    }
}

void MainWindow::on_toolButton_Open_clicked()
{
#ifdef _ECD_LOGGER_
    QStringList qFileNames = QFileDialog::getOpenFileNames(this,
        tr("Tritech Log Formats (*.ecd *.glf)"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Gemini",
        tr("Tritech Log Formats (*.ecd *.glf)"));
#else
    QStringList qFileNames = QFileDialog::getOpenFileNames(this,
        tr("Tritech Log Formats (*.glf)"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Gemini",
        tr("Tritech Log Formats (*.glf)"));
#endif
    if (qFileNames.count() )
    {
        ui->toolButton_Record->setEnabled(false);
        ui->toolButton_Open->setEnabled(false);
        ui->toolButton_Stop->setEnabled(true);
        ui->doubleSpinBox_Range->setEnabled(false);
        ui->spinBox_Gain->setEnabled(false);
        ui->toolButton_Online->setEnabled(false);
        ui->toolButton_Online->setChecked(false);

        m_network->startReplay(qFileNames);
    }
}

void MainWindow::on_toolButton_Stop_clicked()
{
    m_network->stopReplay();

    ui->toolButton_Stop->setEnabled(false);
    // Only enable online button and file open dialog buttton for replay
    ui->toolButton_Open->setEnabled(true);
    ui->toolButton_Online->setEnabled(true);
}

void MainWindow::on_toolButton_Online_clicked(bool checked)
{
    m_network->configOnline(checked);

    ui->toolButton_Record->setEnabled(checked);
    ui->doubleSpinBox_Range->setEnabled(true);
    ui->spinBox_Gain->setEnabled(true);
    ui->toolButton_Open->setEnabled(!checked);

    ui->openGLWidget->clearImage();

}

void MainWindow::on_toolButton_Text_clicked(bool checked)
{
    ui->openGLWidget->enableText(checked);
}

void MainWindow::on_toolButton_Grid_clicked(bool checked)
{
    ui->openGLWidget->enableGrid(checked);
}

void MainWindow::on_toolButton_firmwareFiles_clicked()
{
    QString strDefaultFolder = QString::fromStdString("C:\\");

    strDefaultFolder = QFileDialog::getExistingDirectory(NULL,
                                      tr("Select Folder"), strDefaultFolder);

    if (strDefaultFolder.length() > 0)
    {
        m_network->handleFirmwareFileLocation( strDefaultFolder.toStdString().c_str() );
    }
}

void MainWindow::onFirmwareUpgrade(bool isTransmitting, const QString& message)
{
    FirmwareUpdateDialog dlg( message, this );

    connect(&dlg, &FirmwareUpdateDialog::upgradeFirmware, m_network, &GeminiNetwork::onUpgradeFirmware );
    connect(&dlg, &FirmwareUpdateDialog::abortFirmwareUpgrade, m_network, &GeminiNetwork::onAbortFirmwareUpgrade);
    connect(m_network, &GeminiNetwork::firmwareStatus, &dlg, &FirmwareUpdateDialog::onFirmwareStatus );
    connect(m_network, &GeminiNetwork::geminiStatusChanged, &dlg, &FirmwareUpdateDialog::onRebootComplete);
    // show dialog to upgrade option to user
    dlg.exec();

    //Once the upgrade is finished and previously it was streaming then start streaming again
    if (isTransmitting)
    {
        m_network->configOnline(true);// put the sonar online
    }
}

/************************************************************************
Method: onConnectionChanged
Parameters: isEthernet:         True = The device is connected via Ethernet
                                False = The device is connected via RS232/485
            isMicronOr720im:    True = The device is a Micron Gemini or a 720im
                                False = The device isn't a Micron Gemini or a 720im
Return:     Nothing
Notes:      This is a message from the GeminiNetwork Object when the connection (or device) has changed.
            The H264 Compression should be activated when the connection is via Ethernet AND the device is a Micron Gemini or 720im.
            The RLE Compression should be activated in all other combinations.
************************************************************************/
void MainWindow::onConnectionChanged(bool isEthernet, bool isMicronOr720im)
{
    bool enabled = (isEthernet & isMicronOr720im);  //Ethernet AND Micron Gemini/720im
    this->ui->checkBox_H264->setEnabled(enabled);
    this->ui->spinBox_RLECompression->setEnabled(!enabled);
    this->ui->label_RLECompression->setEnabled(!enabled);
}


/************************************************************************
Method:     onAHRSHPRData
Parameters: data:           A CompassDataRecord structure
Return:     Nothing
Notes:      The HPR is calculated from Raw Data within the CompassProcessor class
            The Pitch and Roll are inverted here as the AHRS device is from a Micron Gemini/720im for the SDK implementation.
************************************************************************/
void MainWindow::onAHRSRawHPRData(GLF::CompassDataRecord* ahrs_compass_record)
{
    if(ui->widget_AHRS->isEnabled()==false)
    {
        ui->widget_AHRS->setEnabled(true);
        if(ui->compassWidget->minimumHeight() == 0)
        {
            ui->compassWidget->setMinimumHeight(kCompassHeight);
        }

    }

    //Display the values...
    ui->lblHeadingValue->setText(QString("%1").arg(ahrs_compass_record->m_heading,0,'f',1));
    ui->lblPitchValue->setText(QString("%1").arg(-ahrs_compass_record->m_pitch,0,'f',1));
    ui->lblRollValue->setText(QString("%1").arg(-ahrs_compass_record->m_roll,0,'f',1));


}

/************************************************************************
Method:     onAHRSRAWData
Parameters: raw_data:           The Raw Data from the AHRS Device (Gyro, Accel, Mag X,Y,Z raw data)
Return:     Nothing
Notes:      The Raw data comes from the AHRS device via the DataProcessor API
************************************************************************/
void MainWindow::onAHRSRawXYZData(GLF::AHRSRawDataRecord* ahrs_rawdata_record)
{
    //Placeholder for RAW Data Processing....
}

void MainWindow::onRangeChanged(double range)
{
    //don't update Range from image if mouse down as Range button could be held down
    if (!m_mouseDown)
    {
        ui->doubleSpinBox_Range->blockSignals(true); //don't want this firing button event to send down a range change to device
        ui->doubleSpinBox_Range->setValue(range);
        ui->doubleSpinBox_Range->blockSignals(false);
    }
}

#ifdef _DATA_LOGGER_
bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
    MSG* msg = static_cast<MSG*>(message);
    if(msg->message == WM_HOTKEY)
    {
        switch (msg->lParam)
        {
            case 0x00430002:
            //qDebug() << "Ctrl+C pressed";
            onRangeDown();
            break;
            case 0x00440002:
            //qDebug() << "Ctrl+D pressed";
            onRangeUp();
            break;
            case 0x004C0002:
            //qDebug() << "Ctrl+L pressed";
            onStartStoplog();
            break;
            default:
                break;
        }
        return true;
    }
    return false;
}
void MainWindow::onRangeUp()
{
    double range = ui->openGLWidget->getRange();
    if( range < 2.0 )
    {
        range = 5.0;
    }
    else  if( range < 150.0 )
    {
        range += 5.0;
    }

    if( range > 150.0 )
    {
        range = 150.0;
    }

    range = ( static_cast<int>( range* 100 ) & 0xFFFF );
    range = std::ceil( range / 100.0 );
    ui->doubleSpinBox_Range->setValue(range);
    m_network->onSetRange(range);
    ui->openGLWidget->setRange(range);
}
void MainWindow::onRangeDown()
{
    double range = ui->openGLWidget->getRange();
    if( range > 1.0 )
    {
        range -= 5.0;
    }
    if( range < 1.0 )
    {
        range = 1.0;
    }

    range = ( static_cast<int>( range* 100 ) & 0xFFFF );
    range = std::ceil( range / 100.0 );
    ui->doubleSpinBox_Range->setValue(range);
    m_network->onSetRange(range);
    ui->openGLWidget->setRange(range);
}

void MainWindow::onStartStoplog()
{
    if ( !ui->toolButton_Record->isChecked() )
    {
        ui->toolButton_Record->setChecked(true);
    }
    else
    {
        ui->toolButton_Record->setChecked(false);
    }
}
#endif

//MK2 Gemini
void MainWindow::on_comboBox_Performance_currentIndexChanged(int index)
{
    m_network->onSetPerformanceLevel(static_cast<SequencerApi::ESdkPerformanceControl>(index));
}

//MK2 Gemini
void MainWindow::on_spinBox_RLECompression_valueChanged(int arg1)
{
    m_network->onSetRLECompression(arg1);
}

void MainWindow::on_checkBox_H264_stateChanged(int arg1)
{
    m_network->onSetH264Compression(static_cast<bool>(arg1));
}
