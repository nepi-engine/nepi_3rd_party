#include "gemininetwork.h"
#include "Svs5Seq/Svs5SequencerApi.h"
#include "GenesisSerializer/GlfLoggerGeminiStructure.h"
#include "Gemini/GeminiStructuresPublic.h"
#include "console/FirmwareUpgradeDef.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>


GeminiNetwork* GeminiNetwork::s_network = nullptr;

int  GeminiNetwork::m_framesPerSeconds = 0;
bool GeminiNetwork::m_highFrequency = false;
bool GeminiNetwork::m_chirpEnabled = false;

GeminiNetwork::GeminiNetwork(QObject *parent)
:   QObject(parent)
,   m_isEthernet(false)
,   m_isMicron_720im(false)
,   m_recording(false)
,   m_replaying(false)

{
    m_quality.m_performance  = SequencerApi::UL_HIGH_CPU;
    m_quality.m_screenPixels = 2048; //initialise to max range lines

    Q_ASSERT(nullptr == GeminiNetwork::s_network);
    GeminiNetwork::s_network = this;

    SequencerApi::StartSvs5 (std::bind(
                    &GeminiNetwork::onGeminiMessageReceived,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3 ) );

#ifdef _ECD_LOGGER_
        bool glfLogger = true; // true for GLF, false for ECD. Default is GLF
        SequencerApi::Svs5SetConfiguration(
                                SequencerApi::SVS5_CONFIG_LOGGER,
                                sizeof(bool),
                                &glfLogger
                                );
#endif
    configOnline(true);
}

GeminiNetwork::~GeminiNetwork()
{
    SequencerApi::StopSvs5();
    GeminiNetwork::s_network = nullptr;
}

void GeminiNetwork::startRecord(const QString& directory)
{
    std::string dir = directory.toStdString();

    // set the record location
    SequencerApi::Svs5SetConfiguration(
                SequencerApi::SVS5_CONFIG_FILE_LOCATION, dir.size(), dir.c_str());
    // start recording
    bool record = true;
    SequencerApi::Svs5SetConfiguration(SequencerApi::SVS5_CONFIG_REC, sizeof(bool), &record);

    m_recording = true;
}

void GeminiNetwork::stopRecord()
{
    // stop the log file recording
    bool record = false;
    SequencerApi::Svs5SetConfiguration(SequencerApi::SVS5_CONFIG_REC, sizeof(bool), &(record));
    m_recording = false;
}

void GeminiNetwork::startReplay(const QStringList &filenames)
{
    // start replay of filename
    std::vector<std::string> files;

    for (int i=0; i<filenames.count(); i++)
    {
        files.push_back(filenames[i].toStdString());
    }

    SequencerApi::ListOfFileNames listOfFileNames( files );
    SequencerApi::Svs5SetConfiguration(
                        SequencerApi::SVS5_CONFIG_PLAY_START,
                        sizeof(listOfFileNames), &listOfFileNames);

    if( !m_replaying )
    {
        m_replaying = true;
        emit(clearImage());
    }
}

void GeminiNetwork::stopReplay()
{
    // stop the current replay
    SequencerApi::Svs5SetConfiguration(SequencerApi::SVS5_CONFIG_PLAY_STOP, 0, nullptr);

    if( m_replaying )
    {
        m_replaying = false;
        emit(clearImage());
    }
}

void GeminiNetwork::onGeminiMessageReceived (unsigned int msgType, unsigned int size, const char* const value)
{
    switch( msgType)
    {
    case SequencerApi::GEMINI_STATUS:       // handle gemini sonar status messages
    {
        const GLF::GeminiSonarStatusMessage* const statusMsg = (const GLF::GeminiSonarStatusMessage* const)value;
        const GLF::GeminiStatusRecord* const status = &statusMsg->m_geminiSonarStatus;


        /* Ignore all network status messages coming from the DA */
        if (GeminiNetwork::s_network && status)
        {
            GeminiNetwork::s_network->handleStatus(*status, statusMsg->m_uiFrame );
        }
        break;
    }
#ifdef _ECD_LOGGER_
    case SequencerApi::ECD_LIVE_TARGET_IMAGE:        // handle Gemini ping image
    {
        CTgtImg* image = (CTgtImg*)value;

        if (GeminiNetwork::s_network && image)
        {
            GeminiNetwork::s_network->handleECDImage(*image);
        }

        break;
    }
#endif
    case SequencerApi::GLF_LIVE_TARGET_IMAGE:        // handle Gemini ping image
    {
        GLF::GLogTargetImage* image = (GLF::GLogTargetImage*)value;

        if (GeminiNetwork::s_network && image)
        {
            GeminiNetwork::s_network->handleGLFImage(*image);
        }

        break;
    }
    case SequencerApi::TGT_IMG_PLAYBACK:    // handle Gemini replay image
    {
        GLF::GenesisPlaybackTargetImage* image = (GLF::GenesisPlaybackTargetImage*)value;

        if (GeminiNetwork::s_network && image)
        {
            GeminiNetwork::s_network->handleReplayGLFImage(*image->m_pLogTgtImage, image->m_frame );
        }

        break;
    }
#ifdef _ECD_LOGGER_
    case SequencerApi::ECD_IMG_PLAYBACK:    // handle Gemini replay image
    {
        CTgtImg* image = (CTgtImg*)value;

        if (GeminiNetwork::s_network && image)
        {
            GeminiNetwork::s_network->handleReplayECDImage(*image);
        }
        break;
    }
#endif
    case SequencerApi::LOGGER_REC_UPDATE:   // handle log file recording information
    {
        const GLF::SOutputFileInfo* info = (const GLF::SOutputFileInfo*)value;

        if (GeminiNetwork::s_network && info)
        {
            QString message = QString("Recording: %1 (%2MB)")
                    .arg(QString(info->m_strFileName ))
                    .arg(info->m_fileSizeBytes / 1000000);

            GeminiNetwork::s_network->handleMessage(message);
        }

        break;
    }
    case SequencerApi::LOGGER_FILE_INDEX:   // handle replay file record index information
    {
        emit(GeminiNetwork::s_network->clearImage());

        break;
    }
    case SequencerApi::LOGGER_STATUS_INFO:  // handle logging status information
    {
        GLF::GnsLoggerStatusInfo* statusInfo = (GLF::GnsLoggerStatusInfo*)value;

        if (GeminiNetwork::s_network)
        {
            GeminiNetwork::s_network->handleMessage(QString (statusInfo->m_loggerStatusInfo ));
        }

        break;
    }
    case SequencerApi::FRAME_RATE:
    {
        m_framesPerSeconds = *(unsigned int*)value;
    }
    break;
    case SequencerApi::GPS_RECORD:
    {
        GLF::GLogV4ReplyMessage* gnsV4ReplyMsg = (GLF::GLogV4ReplyMessage*)value;
        std::cout<< "GPS data received ..." << std::endl;
        std::vector<unsigned char>& vecGps = *gnsV4ReplyMsg->m_v4GenericRec.m_vecData;
        for( size_t i = 0; i < vecGps.size(); ++i )
        {
            std::cout<< vecGps[ i ];
        }
        std::cout<< "\t Frame Number " << gnsV4ReplyMsg->m_uiFrame <<std::endl;
    }
    break;
    case SequencerApi::COMPASS_RECORD:
    {
        GLF::GLogV4ReplyMessage* gnsV4ReplyMsg = const_cast<GLF::GLogV4ReplyMessage*>(reinterpret_cast<const GLF::GLogV4ReplyMessage *>(value));
        std::vector<unsigned char>& vecCompass = *gnsV4ReplyMsg->m_v4GenericRec.m_vecData;

        GLF::CompassDataRecord *record = reinterpret_cast<GLF::CompassDataRecord*>(reinterpret_cast<char*>(vecCompass.data()));

        emit(GeminiNetwork::s_network->handleAHRSHPRData(record));
    }
    break;
    case SequencerApi::AHRS_HPR_DATA:
    {
        GLF::CompassDataRecord *record = const_cast<GLF::CompassDataRecord*>(reinterpret_cast<const GLF::CompassDataRecord *>(value));
        emit(GeminiNetwork::s_network->handleAHRSHPRData(record));
    }
    break;
    case SequencerApi::AHRS_RAW_DATA:
    {
        GLF::AHRSRawDataRecord *record = const_cast<GLF::AHRSRawDataRecord*>(reinterpret_cast<const GLF::AHRSRawDataRecord *>(value));
        emit(GeminiNetwork::s_network->handleAHRSRawData(record));
    }
    break;
    case SequencerApi::AUXPORT1_DATA:
    {
        std::cout<< "Gemini AUX Port 1 data received " << std::endl;
        for ( unsigned int i = 0; i < size; ++i )
        {
            std::cout<< std::hex << (value[ i ] & 0xFF) << " ";
        }
        std::cout<< std::endl;
    }
    break;
    case SequencerApi::AUXPORT2_DATA:
    {
        std::cout<< "Gemini AUX Port 2 data received " << std::endl;
        for ( unsigned int i = 0; i < size; ++i )
        {
            std::cout<< std::hex << (value[ i ] & 0xFF) << " ";
        }
        std::cout<< std::endl;
    }
    break;
    case SequencerApi::UPGRADE_AVAILABLE:
    {
        std::cout<< "Firmware Upgrade is available %s"<< value <<std::endl;
        if (GeminiNetwork::s_network)
        {
            GeminiNetwork::s_network->handleFirmwareUpgrade( value );
        }
    }
    break;
    case SequencerApi::FIRMWARE_UPGRADE_INFO:
    {
        console::FirmwareUpgradeStatus* fwUpgradeInfo = (console::FirmwareUpgradeStatus*)value;
        emit( GeminiNetwork::s_network->firmwareStatus(
                            QString::fromStdString( fwUpgradeInfo->m_upgradeStatusInfo ),
                            fwUpgradeInfo->m_percentComplete,
                            fwUpgradeInfo->m_programStatus )
                            );
        if( fwUpgradeInfo )
        {
            switch(fwUpgradeInfo->m_programStatus)
            {
                case console::FirmwareUpgradeStatus::FW_UG_PROGRAMME:
                {
                    std::cout << "Firmware programming : "
                                << "Completed:" << fwUpgradeInfo->m_percentComplete << "%\t"
                                << " desc : "<< fwUpgradeInfo->m_upgradeStatusInfo
                                << std::endl;
                }
                break;
                case console::FirmwareUpgradeStatus::FW_UG_FAILURE:
                {
                    std::cout << "Failed to upgrade firmware : " << fwUpgradeInfo->m_programStatus
                                << "Completed:" << fwUpgradeInfo->m_percentComplete << "%\t"
                                << " desc : "<< fwUpgradeInfo->m_upgradeStatusInfo
                                << std::endl;
                }
                break;
                case console::FirmwareUpgradeStatus::FW_UG_SUCCESS:
                {
                    // initiate the reboot if this was the last step
                    // std::cout<< "The firmware has updated successfully. Sonar will now reboot.\n";
                }
                break;
                case console::FirmwareUpgradeStatus::FW_UG_REBOOTING:
                {
                    std::cout << "Rebooting...\n";
                }
                break;
                default:
                    break;
            }
        }
    }
    break;
    case SequencerApi::COM_PORT_STATUS:       // COM port status
    {
        CommPortMapping comPortList = *(CommPortMapping*)value;
        for( CommPortMappingItr itr = comPortList.begin();
                                       itr != comPortList.end(); ++itr )
        {
            std::cout<< " Com port Name " << (*itr).m_comPortName <<
                ", ID " << (*itr).m_comPortID << " State " << ( (*itr).m_busy ? " Used" : " Available" )
                << std::endl;
        }
        std::cout<< std::endl;
    }
    break;

    default:
        break;
    }
}

void GeminiNetwork::handleStatus(const GLF::GeminiStatusRecord& status, unsigned int frame)
{
    //Local vars to only emit when there has been a change
    bool localIsEthernet = ((status.m_linkType & 0x0C00) == 0x0400);
    bool localIsMicron_720im = isMicronOr720im(status.m_flags);

    // send out status message if the system is not recording or replaying
    if(!m_replaying && !m_recording)
    {
        QString s;
        if ((status.m_BOOTSTSRegister & 0x000001ff) == 0x00000001)
        {
            s = "Bootloader booted";
        }
        else
        {
            if (status.m_shutdownStatus & 0x0001)
            {
                s = " (Over temp)";
            }
            else if (status.m_shutdownStatus & 0x0006)
            {
                s = " (Out of water)";
            }
        }

        s += (m_highFrequency ? ", High Frequency" : ", Low Frequency ");
        s += (m_chirpEnabled ? ", Chirp On" : ", Chirp Off ");

        if ((status.m_linkType & 0x0C00) == 0x0800)
        {
            s += ", RS232 : " + QString::number( status.m_uplinkSpeedMbps, 'f', 0 ) + "bps";
        }
        else if ((status.m_linkType & 0x0C00) == 0x0C00)
        {
            s += ", RS485 : " + QString::number( status.m_uplinkSpeedMbps, 'f', 0 ) + "bps";
        }
        infoMessage(QString("Sonar %1 %2, FPS:%3")
            .arg(status.m_deviceID) .arg(s) .arg(m_framesPerSeconds) );
    }

    // Replaying status message
    if( m_replaying )
    {
        infoMessage(QString("Replaying Sonar %1 : Frame: %2")
            .arg(status.m_deviceID) .arg(frame) );
    }

    // Add only new device in the list
    if ( !m_geminiDevice.count(status.m_deviceID ) )
    {
        m_geminiDevice.insert( std::make_pair( status.m_deviceID, status ) );
    }

    //Has the connection changed?
    if(localIsEthernet != m_isEthernet || localIsMicron_720im != m_isMicron_720im) {
        //Set both member variables
        m_isEthernet = localIsEthernet;
        m_isMicron_720im = localIsMicron_720im;

        emit(geminiConnectionChanged(m_isEthernet, m_isMicron_720im));      //Let the GUI pick this up and display the Widgets as required.
    }
    emit( geminiStatusChanged() );

}

void GeminiNetwork::handleGLFImage(const GLF::GLogTargetImage& image)
{
    if (!m_replaying)
    {
        m_highFrequency = ( image.m_mainImage.m_usPingFlags & 0x0100 );
        m_chirpEnabled  = image.m_mainImage.m_fChirp;
        emit(glfImageReceived(image));
    }
}
#ifdef _ECD_LOGGER_
void GeminiNetwork::handleECDImage(const CTgtImg& image)
{
    if (!m_replaying)
    {
        m_highFrequency = ( ( image.m_clTgtRec.m_usPingId >> 8 ) & 1 );
        m_chirpEnabled  = image.m_clTgtRec.m_clPing.m_bChirp;
        emit(ecdImageReceived(image));
    }
}
#endif

void GeminiNetwork::handleReplayGLFImage(const GLF::GLogTargetImage& image, unsigned int frame)
{
    if( m_replaying )
    {
        QString s;
        m_highFrequency = ( image.m_mainImage.m_usPingFlags & 0x0100 );
        m_chirpEnabled  = image.m_mainImage.m_fChirp;

        s = QString("Replaying Sonar %1").arg(image.m_header.m_ciHeader.m_deviceID);
        s += QString(" : Frame %1 ").arg(frame);
        s += (m_highFrequency ? ", High Frequency" : ", Low Frequency ");
        s += (m_chirpEnabled ? ", Chirp On" : ", Chirp Off ");

        infoMessage(s);
        emit(glfImageReceived(image));
    }

}

#ifdef _ECD_LOGGER_
void GeminiNetwork::handleReplayECDImage(const CTgtImg& image)
{
    if( m_replaying )
    {
        QString s;
        m_highFrequency = ( ( image.m_clTgtRec.m_usPingId >> 8 ) & 1 );
        m_chirpEnabled  = image.m_clTgtRec.m_clPing.m_bChirp;

        s = QString("Replaying Sonar %1").arg(image.m_clTgtRec.m_clPing.m_sRx1);
        s += (m_highFrequency ? ", High Frequency" : ", Low Frequency ");
        s += (m_chirpEnabled ? ", Chirp On" : ", Chirp Off ");

        infoMessage(s);
        emit(ecdImageReceived(image));
    }
}
#endif

void GeminiNetwork::handleMessage(const QString& message)
{
    emit(infoMessage(message));
}

void GeminiNetwork::onSetRange(double range)
{
    if (0.0 < range && range <= 150.0)
    {
        // Configure range value
        SequencerApi::Svs5SetConfiguration(SequencerApi::SVS5_CONFIG_RANGE, sizeof(double), &range);
    }
}

void GeminiNetwork::onSetGain(int gain)
{
    if (0 < gain && gain <= 100)
    {
        // Configure Gain value
        SequencerApi::Svs5SetConfiguration(SequencerApi::SVS5_CONFIG_GAIN, sizeof(int), &gain);
    }
}

void GeminiNetwork::onSizeChanged(int width, int height)
{
    m_quality.m_screenPixels = height;

    // Only enable, if bandwidth is the issue
#ifdef _LOW_PERFORMANCE_CPU_
    // Configure SDK according to the height of the CPU    
    m_quality.m_performance   = SequencerApi::HIGH_CPU;
#endif

    if (m_quality.m_performance != SequencerApi::UL_HIGH_CPU) //i.e. will only effect other performance levels
    {
        SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_CPU_PERFORMANCE,
                            sizeof(m_quality),
                            &m_quality
                            );
    }
}

void GeminiNetwork::configOnline(bool online)
{
    if( online )
    {
        ////////////////////////////////////////////////////////////////////////////////////////
        // For Linux users, make sure that the device does not have root only permission
        // Change permission using sudo usermod -a -G dialout MY_USER_NAME
        //
        // Note: Only maximum of 16 (720im) devices supported to open on either regular COM port or
        //       using Tritech USB-To_Serial converter with device ID 65535 to 65520
        //       To open the first device ID, use device ID 65535
        //       To open the second device ID, use device ID 65534
        //       To open the third device ID, use device ID 65533
        ////////////////////////////////////////////////////////////////////////////////////////
#if 0
        //For Windows : use COM1, COM2.....
        //For Linux : use /dev/ttyUSB0, /dev/ttyUSB1.....

        ComPortConfig comPort4(false, false, 921600, "COM4");
        SequencerApi::Svs5SetConfiguration(
                                SequencerApi::SVS5_CONFIG_OPEN_720IM_COM_PORT,
                                sizeof(ComPortConfig),
                                &comPort4,
                                0xFFFF
                                );

        ComPortConfig comPort6(false, false, 921600, "COM6");
        SequencerApi::Svs5SetConfiguration(
                                SequencerApi::SVS5_CONFIG_OPEN_720IM_COM_PORT,
                                sizeof(ComPortConfig),
                                &comPort6,
                                0xFFFE
                                );

        ComPortConfig comPort10(false, false, 921600, "/dev/ttyUSB0");
        SequencerApi::Svs5SetConfiguration(
                                SequencerApi::SVS5_CONFIG_OPEN_720IM_COM_PORT,
                                sizeof(ComPortConfig),
                                &comPort10,
                                0xFFFD
                                );
#endif
    }

    // Has not received any status message from any device
    if( !m_geminiDevice.size() )
    {
        return;
    }

    // Test code to switch between 120/65 degrees
/*
    if( online )
    {
    static double aperture = 120.0;
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_APERTURE,
                            sizeof(double),
                            &aperture
                            );
    aperture = aperture >= 120.0 ? 65.0 : 120.0;
    }
*/

    // SDK quality parameters
    SequencerApi::SonarImageQualityLevel quality;
    quality.m_performance   = m_quality.m_performance;
    quality.m_screenPixels  = m_quality.m_screenPixels;

    // default orientation is up
    SequencerApi::ESvs5SonarOrientation orientation = SequencerApi::SONAR_ORIENTATION_UP;
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_SONAR_ORIENTATION,
                            sizeof(SequencerApi::ESvs5SonarOrientation),
                            &orientation
                            );


    CHIRP_MODE chMode = CHIRP_AUTO; //Configure auto chirp mode
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_CHIRP_MODE,
                            sizeof(CHIRP_MODE),
                            &chMode
                            );

    // configure to switch frequency automatically between high/low @10m ( Only for 1200ik )
    RangeFrequencyConfig rangeResolution;
    rangeResolution.m_frequency        = FREQUENCY_AUTO;
    rangeResolution.m_rangeThreshold   = 40;
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_RANGE_RESOLUTION,
                            sizeof(RangeFrequencyConfig),
                            &rangeResolution
                            );

    // For crisp image quality in low range, enable this option
    // For lower bandwidth, disable this option
    bool highRangeResolution = true; //Enable high range resolution
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_HIGH_RESOLUTION,
                            sizeof(bool),
                            &highRangeResolution
                            );

#ifdef _LOW_PERFORMANCE_CPU_
    // Configure SDK to start with lower performance CPU
    quality.m_performance   = SequencerApi::LOW_CPU;
    quality.m_screenPixels  = 256;
#endif
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_CPU_PERFORMANCE,
                            sizeof(quality),
                            &quality
                            );

    configPingMode();

    // configure sonar to online / offline
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_ONLINE,
                            sizeof(bool),
                            &online
                            );
}

void GeminiNetwork::configPingMode( )
{
    // Configure ping mode
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_PING_MODE,
                            sizeof(m_sPingMode),
                            &m_sPingMode
                            );
}


void GeminiNetwork::handleFirmwareUpgrade( const char* str )
{
    bool online;
    // configure sonar to online / offline
    SequencerApi::Svs5GetConfiguration(
                            SequencerApi::SVS5_CONFIG_ONLINE,
                            sizeof(bool),
                            &online
                            );

    emit(firmwareUpgrade( online, QString( str ) ) );
}

void GeminiNetwork::onUpgradeFirmware(unsigned short deviceID)
{
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_UPGRADE_FIRMWARE,
                            0,
                            NULL,
                            deviceID
                            );
}

void GeminiNetwork::onAbortFirmwareUpgrade(unsigned short deviceID)
{
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_ABORT_UPGRADE,
                            0,
                            NULL,
                            deviceID
                );
}

/*
LOW_CPU,            // Level 1 - Low image quality (256 beams)
MEDIUM_CPU,         // Level 2 - Medium image quality (256 beams)
HIGH_CPU,           // Level 3 - High image quality (512 beams)
UL_HIGH_CPU         // Level 4 - Ultra high quality
                    //           512 beams for 720is and 720ik,
                    //           1024 beams for 1200ik in higher resolution
*/
void GeminiNetwork::onSetPerformanceLevel(SequencerApi::ESdkPerformanceControl level)
{
    m_quality.m_performance = level;

    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_CPU_PERFORMANCE,
                            sizeof(m_quality),
                            &m_quality
                );
}

void GeminiNetwork::onSetRLECompression(int level)
{
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_RLE_COMPRESSION,
                            sizeof(level),
                            &level
                );
}

void  GeminiNetwork::onSetH264Compression(bool enabled)
{
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_H264_COMPRESSION,
                            sizeof(enabled),
                            &enabled
                );

}


void GeminiNetwork::handleFirmwareFileLocation( const char* folderPath )
{
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_FIRMWARE_LOCATION,
                            strlen(folderPath),
                            folderPath
                            );
}

/*********************************************************************************
Function: isMicronOr720im
Parameters: mFlags from the Status record
Returns:    whether the device is a Micron Gemini or a 720im
Notes:      The device ID's are as below:

#define  GEM_HEADTYPE_720IM           0x1F
#define  GEM_HEADTYPE_MICRON_GEMINI   0x27

To check for the above, the device_flag has to tbe shifted:  status.m_flags >> 8;
***********************************************************************************/

bool GeminiNetwork::isMicronOr720im(short device_flag)
{
    short shifted = (device_flag >> 8);       //Just so I only have to bit shift once for performance (Bare Metal Head on here...).

    return (shifted == GEM_HEADTYPE_MICRON_GEMINI || shifted == GEM_HEADTYPE_720IM);

}



