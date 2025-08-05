#ifndef GEMININETWORK_H
#define GEMININETWORK_H

#include <QtCore/QObject>
#ifdef _ECD_LOGGER_
#include "ECDLogDataTypes/ecdlogtargetimage.h"
#endif
#include "GenesisSerializer/GlfApi.h"
#include "Svs5Seq/Svs5SequencerApi.h"
#include "GenesisSerializer/CompassRecord.h"

#include <map>


class GeminiNetwork : public QObject
{
    Q_OBJECT
    typedef std::map<unsigned short, GLF::GeminiStatusRecord> GeminiDeviceMap;

public:
    explicit GeminiNetwork(QObject *parent = 0);
    ~GeminiNetwork();

    void startRecord(const QString& directory); // start recording a log file
    void stopRecord();                          // stop recording a log file

    void startReplay(const QStringList& filename);  // start replay of a log file
    void stopReplay();                          // stop replay of a log file
    void configOnline(bool online);             // online / offline
    void configPingMode();                      // Configure ping run mode

    static void onGeminiMessageReceived(unsigned int msgType, unsigned int size, const char* const value);

    // status and ping image handlers
    void handleStatus(const GLF::GeminiStatusRecord& status, unsigned int frame);
    void handleGLFImage(const GLF::GLogTargetImage& image);
    void handleReplayGLFImage(const GLF::GLogTargetImage& image, unsigned int frame);
    void handleFirmwareUpgrade( const char* str );
    void handleFirmwareFileLocation( const char* folderPath );

#ifdef _ECD_LOGGER_
    void handleECDImage(const CTgtImg& image);
    void handleReplayECDImage(const CTgtImg& image);
#endif
    void handleMessage(const QString& message);

signals:
    void statusReceived(unsigned short sonarID);    // send out received status information
    void glfImageReceived(const GLF::GLogTargetImage& image);       // send out received ping or replay image
#ifdef _ECD_LOGGER_
    void ecdImageReceived(const CTgtImg& image);       // send out received ping or replay image
#endif
    void infoMessage(const QString& message, int timeout = 3000); // send out status bar messages
    void clearImage();
    void firmwareUpgrade( bool online, const QString& message );
    void firmwareStatus( const QString& status, unsigned int percent, int state);
    void geminiStatusChanged();
    void geminiConnectionChanged(bool m_isEthernet, bool m_isMicron_720im);
    void handleAHRSRawData(GLF::AHRSRawDataRecord* ahrs_rawdata_record);
    void handleAHRSHPRData(GLF::CompassDataRecord* ahrs_compass_record);
public slots:
    void onSetRange(double range);              // set the sonar range in metres (1-120)
    void onSetGain(int gain);                   // set the sonar gain in % (1-100)
    void onSizeChanged(int, int);               // Called when the size is changed
    void onUpgradeFirmware(unsigned short deviceID);
    void onAbortFirmwareUpgrade(unsigned short deviceID);
    void onSetPerformanceLevel(SequencerApi::ESdkPerformanceControl level);
    void onSetRLECompression(int level);
    void onSetH264Compression(bool);

private:
    GeminiDeviceMap m_geminiDevice;
    bool m_isEthernet;                          //Flag to determine if the connected device is via Ethernet
    bool m_isMicron_720im;                      //Flag to determine if the device is Micron OR 720im
    bool m_recording;
    bool m_replaying;
    static int   m_framesPerSeconds;
    static bool  m_highFrequency;
    static bool  m_chirpEnabled;
    SequencerApi::SequencerPingMode m_sPingMode;

    SequencerApi::SonarImageQualityLevel m_quality;

    static GeminiNetwork* s_network;     // static to only GeminiNetwork, used for callback

    bool isMicronOr720im(short);
};

#endif // GEMININETWORK_H
