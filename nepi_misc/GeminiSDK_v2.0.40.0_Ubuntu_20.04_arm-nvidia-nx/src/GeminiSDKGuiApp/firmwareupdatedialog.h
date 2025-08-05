/******************************************************************************************
 *
 * \file    firmwareupdatedialog.h
 * \brief   This class presents the user with a firmware update widget.
 *
 ******************************************************************************************/

#ifndef FIRMWAREUPDATEDIALOG_H
#define FIRMWAREUPDATEDIALOG_H

#include <QtWidgets/QDialog>
#include <QtCore/QTimer>

#include "console/FirmwareUpgradeDef.h"


namespace Ui {
class FirmwareUpdateDialog;
}

class FirmwareUpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FirmwareUpdateDialog( const QString& message, QWidget *parent = 0, bool nologo = false,
        unsigned int rebootTimeout = 30000, unsigned int rebootDelay = 5000);
    ~FirmwareUpdateDialog();

public slots:
    void onAccept();
    void onCancel();
    void onTimer();

    void onFirmwareStatus( const QString& status, unsigned int percent, int state);
    void onRebootComplete();

signals:
    void requestReboot();
    void upgradeFirmware(unsigned short deviceID);
    void abortFirmwareUpgrade(unsigned short deviceID);

private:
    bool updateFirmware();
    void rebootFailed();
    void showControls(bool step, bool progress, bool status);
    void showVersion(bool from, bool to);

private:
    Ui::FirmwareUpdateDialog *ui;
    QString m_label;                                ///< Device label for display.

    QTimer m_timer;                                 ///< Timeout and progress update timer.
    unsigned int m_rebootDelay;                     ///< Wait time until device reboot after programming success.
    unsigned int m_rebootTimeout;                   ///< Reboot timeout
    bool         m_noLogo;                          ///< Force to update without user interaction

    static const int kProgrammingTimeout = 15000;   ///< Programming step timeout.
    static const int kRebootIntervalTimeout = 100;  ///< Reboot progress bar update timer

    unsigned int m_step;                            ///< Current firmware update step.
    unsigned int m_updateSteps;                     ///< Total firmware update steps.
    console::FirmwareUpgradeStatus::FW_UG_STATUS m_status;  ///< Current firmware update status
    bool m_isCancelled;                             ///< Flag to indicate user cancellation.
    unsigned short m_deviceID;                      ///< Sonar device ID
};

#endif // FIRMWAREUPDATEDIALOG_H
