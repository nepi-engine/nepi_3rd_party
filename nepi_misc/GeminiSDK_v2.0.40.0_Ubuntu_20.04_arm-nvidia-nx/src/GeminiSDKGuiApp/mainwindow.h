#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "GenesisSerializer/CompassRecord.h"

class GeminiNetwork;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(bool transparent = false, bool frameless = false, QWidget *parent = 0);
    ~MainWindow();

    bool eventFilter(QObject* obj, QEvent* ev);

public slots:
    void on_doubleSpinBox_Range_valueChanged(double value); // set the sonar range
    void on_spinBox_Gain_valueChanged(int value);           // set the gain value
    void on_toolButton_Record_toggled(bool checked);        // toggle the log file recording
    void on_toolButton_Open_clicked();                      // open log file open dialog
    void on_toolButton_Stop_clicked();                      // stop and close the replay
    void on_toolButton_Online_clicked(bool checked);        // online/offline sonar pinging
    void on_toolButton_Text_clicked(bool checked);          // toggle range labels
    void on_toolButton_Grid_clicked(bool checked);          // toggle grid overlay
    void onFirmwareUpgrade(bool isTransmitting, const QString& message); // Display doalog for the firmware upgrade
    void on_toolButton_firmwareFiles_clicked();
    void onConnectionChanged(bool isEthernet, bool isMicronOr720im);
    void onAHRSRawXYZData(GLF::AHRSRawDataRecord* ahrs_rawdata_record);
    void onAHRSRawHPRData(GLF::CompassDataRecord* ahrs_compass_record);
    void onRangeChanged(double);                            //When the Range has changed, update the Range Spinbox, used when in Broadcast Client Mode.
#ifdef _DATA_LOGGER_
    virtual bool nativeEvent(const QByteArray& eventType, void* message, long* result);

    void onRangeUp();
    void onRangeDown();
    void onStartStoplog();
#endif

private slots:
    void on_comboBox_Performance_currentIndexChanged(int index);

    void on_spinBox_RLECompression_valueChanged(int arg1);

    void on_checkBox_H264_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;   

    GeminiNetwork* m_network;           // Gemini data network handler
    QString m_logDirectory;             // log file name for recording

    bool m_mouseDown;

};

#endif // MAINWINDOW_H
