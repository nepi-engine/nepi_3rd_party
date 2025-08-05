#include "Svs5Seq/Svs5SequencerApi.h"
#include "Gemini/GeminiStructuresPublic.h"
#include "GenesisSerializer/GlfApi.h"
#include "GenesisSerializer/GenericDataTypes.h"
#include "GenesisSerializer/GpsRecord.h"
#include "GenesisSerializer/CompassRecord.h"
#include "GenesisSerializer/GeminiStatusRecord.h"

#include "console/FirmwareUpgradeDef.h"

#include <iostream>

#include <iomanip>
#include <limits>

const char GPS[] = "$GPGGA,143305,4722.68807,N,00813.95741,E,2,07,1.4,0.0,M,0.0,M,2.2,0362*5C";
const char COMPASS[] = "H170.5P-1.2R-1.1T12.3D501.26B12.3A59W59LN47F0";
static unsigned short sg_sonarID = 0;

static void ConfigOnline( bool fOnline );
static void configPingMode( const SequencerApi::SequencerPingMode& sPingMode );

static void onGeminiMessageReceived(
                            unsigned int        msgType,
                            size_t              size,
                            const char* const   value
                            )
{
    switch( msgType)
    {
        case SequencerApi::GEMINI_STATUS:
        {
            const GLF::GeminiSonarStatusMessage* const statusMsg = (const GLF::GeminiSonarStatusMessage* const)value;
            const GLF::GeminiStatusRecord* const pStatus = &statusMsg->m_geminiSonarStatus;
            // If status message is coming with 0 Alternate IP adrress then don't print anything
            if( !pStatus->m_sonarAltIp )
            {
                return;
            }
            sg_sonarID = pStatus->m_deviceID;
            std::cout<<  "Status  message received from : ";
            unsigned int from = pStatus->m_sonarAltIp;
            std::cout << std::dec << std::setfill( '0' ) << std::setw( 2 )
                << (int)( (from>>24) & 0xFF ) << "." << (int)( (from>>16) & 0xFF ) << "."
                << (int)( (from>>8) & 0xFF ) << "." << (int)( (from>>0) & 0xFF ) << "\n";

            std::string s;

            if ((pStatus->m_BOOTSTSRegister & 0x000001ff) == 0x00000001)
            {
                s = "Bootloader booted";
            }
            else
            {
                if (pStatus->m_shutdownStatus & 0x0001)
                {
                    s = " (Over temp)";
                }
                else if (pStatus->m_shutdownStatus & 0x0006)
                {
                    s = " (Out of water)";
                }
            }
            if( s.size() )
            {
                std::cout<< "Sonar status " << s << std::endl;
            }
        }
        break;
        case SequencerApi::GLF_LIVE_TARGET_IMAGE:
        {
            GLF::GLogTargetImage* logTgtImage = (GLF::GLogTargetImage*)value;

#ifndef _DATA_LOGGER_
            std::cout
            << ( ( logTgtImage->m_mainImage.m_usPingFlags & 0x8000 ) ? " User Selected SOS " : "Sonar Speed of Sound " )
            << logTgtImage->m_mainImage.m_fSosAtXd
            << ", Range Compression used " << logTgtImage->m_mainImage.m_usRangeCompUsed
            << ", Width " << logTgtImage->m_mainImage.m_uiEndBearing
            << ", Height "<< logTgtImage->m_mainImage.m_uiEndRange
            << ", Frequency " << ( ( logTgtImage->m_mainImage.m_usPingFlags & 0x0100 ) ? "High" : " Low" )
            << ", Chirp " << ( ( logTgtImage->m_mainImage.m_fChirp ) ? "On" : "off" )
            << std::endl;

            // This information is provided by the application that sonar is
            // mounted inverted or not
            if( ( logTgtImage->m_mainImage.m_uiStateFlags & 0xE000 ) )
            {
                std::cout
                << "Sonar orientation is inverted "
                << std::endl;
            }
#endif
        }
        break;
        case SequencerApi::LOGGER_REC_UPDATE:
        {
            const GLF::SOutputFileInfo* sLoggerInfo = (const GLF::SOutputFileInfo*)value;
            std::cout<< "Record Info :\n"
                << "\tFileName:\t" << sLoggerInfo->m_strFileName << std::endl
                << "\tNo Of Records:\t" << sLoggerInfo->m_uiNumberOfRecords<< std::endl
                << "\tFile Size(bytes):\t" << sLoggerInfo->m_fileSizeBytes<< std::endl
                << "\tFree Disk Space:\t" << sLoggerInfo->m_diskSpaceFreeBytes<< std::endl
                << "\tPercentage Disk Space Free:\t" << sLoggerInfo->m_percentDiskSpaceFree<< std::endl
                << "\tRecording Time Left :\t" << sLoggerInfo->m_recordingTimeLeftSecs<< std::endl;
        }
        break;
        case SequencerApi::LOGGER_PLAYBACK_UPDATE:   // handle log file playback information
        {
            const GLF::SInputFileListInfo* info = (const GLF::SInputFileListInfo*)value;

            if (info && info->m_uiPercentProcessed && ( ( info->m_uiPercentProcessed % 100 ) == 0 ) )
            {
                // Print total records (frames) 100 frames means ( 0 -99 )
                std::cout<< "Number of Records " << info->m_uiNumberOfRecords << std::endl;
                // Print filenames
                for ( unsigned int i = 0; i < info->m_uiNumberOfFiles; ++i )
                {
                    std::cout<< "Playingback filename " << info->m_filenames[ i ] << std::endl;
                }
            }
        }
        break;
        case SequencerApi::TGT_IMG_PLAYBACK:
        {
            GLF::GenesisPlaybackTargetImage* logTgtImage = (GLF::GenesisPlaybackTargetImage*)value;
            std::cout
            << "Playing back..."
            << " Width " << logTgtImage->m_pLogTgtImage->m_mainImage.m_uiEndBearing
            << " Height "<< logTgtImage->m_pLogTgtImage->m_mainImage.m_uiEndRange
            << " frame "<< logTgtImage->m_frame
            << std::endl;
        }
        break;
        case SequencerApi::LOGGER_FILE_INDEX:
        {
            std::cout<< "Playing File Index..." << *(int*)value<< std::endl;
        }
        break;
        case SequencerApi::LOGGER_STATUS_INFO:
        {
            GLF::GnsLoggerStatusInfo* statusInfo = (GLF::GnsLoggerStatusInfo*)value;
            std::cout<< "Logger status info...\r\n" << statusInfo->m_errType << statusInfo->m_loggerStatusInfo<< std::endl;
        }
        break;
        case SequencerApi::FRAME_RATE:
        {
#ifndef _DATA_LOGGER_
            unsigned int fps = *(unsigned int*)value;
            std::cout<< "Frames per seconds " << fps << std::endl;
#endif
        }
        break;
        case SequencerApi::GPS_RECORD:
        {
            GLF::GLogV4ReplyMessage* gnsV4ReplyMsg = (GLF::GLogV4ReplyMessage*)value;
            std::cout<< "GPS data received at " << std::setprecision (15)
                << gnsV4ReplyMsg->m_header.m_ciHeader.m_timestamp << std::endl;
            std::vector<unsigned char>& vecGps = *gnsV4ReplyMsg->m_v4GenericRec.m_vecData;

            // Raw ascii
            if( gnsV4ReplyMsg->m_header.m_ciHeader.m_dataType == 98 )
            {
                for( size_t i = 0; i < vecGps.size(); ++i )
                {
                    std::cout<< vecGps[ i ];
                }
            } //V4 recorded message
            else if( gnsV4ReplyMsg->m_header.m_ciHeader.m_dataType == 99 )
            {
                //GLF::Gps
                GLF::GpsDataRecord* pNewFixRec = (GLF::GpsDataRecord*)&vecGps[0];
                printf("Received GPS Fix rec (Lat = %.4f, Lon = %.4f, E = %.3f, N = %.3f)\n",
                       pNewFixRec->m_llRec.m_latDegrees, pNewFixRec->m_llRec.m_longDegrees,
                       pNewFixRec->m_enRec.m_easting, pNewFixRec->m_enRec.m_northing);
            }
            std::cout<< "\t Frame Number " << gnsV4ReplyMsg->m_uiFrame <<std::endl;
        }
        break;
        case SequencerApi::COMPASS_RECORD:
        {
            GLF::GLogV4ReplyMessage* gnsV4ReplyMsg = (GLF::GLogV4ReplyMessage*)value;
            std::cout<< "Compass data received at " << std::setprecision (15) << gnsV4ReplyMsg->m_header.m_ciHeader.m_timestamp <<std::endl;
            std::vector<unsigned char>& vecCompass = *gnsV4ReplyMsg->m_v4GenericRec.m_vecData;

            if( gnsV4ReplyMsg->m_header.m_ciHeader.m_dataType == 98 ) // Raw ascii
            {
                for( size_t i = 0; i < vecCompass.size(); ++i )
                {
                    std::cout<< vecCompass[ i ];
                }
            }
            else if( gnsV4ReplyMsg->m_header.m_ciHeader.m_dataType == 99 ) //V4 Recorded message
            {
                GLF::CompassDataRecord* pNewRec = (GLF::CompassDataRecord*)&vecCompass[0];
                // output Heading, Pitch and Roll to test...
                printf("Received Compass Rec (Hdg = %.2f, Pitch = %.2f, Roll = %.2f)\n",
                       pNewRec->m_heading, pNewRec->m_pitch, pNewRec->m_roll);
            }
            std::cout<< "\t Frame Number " << gnsV4ReplyMsg->m_uiFrame <<std::endl;
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

            char input[256];
            const char* delimiters = ";";
            memset( input, '\0', sizeof(input) );
            memcpy( input, value, size );
            const char *product = std::strtok(input, delimiters);
            const char *deviceID = std::strtok(nullptr, delimiters);
            const char *currentVersion = std::strtok(nullptr, delimiters);
            const char *newVersion = std::strtok(nullptr, delimiters);
            const char *updateSteps = std::strtok(nullptr, delimiters);

            std::cout<< "Gemini " << product << " - " << deviceID << std::endl;
            std::cout<< "Current Version : " << currentVersion << std::endl;
            std::cout<< "New Version : " << newVersion << std::endl;
            std::cout<< "Number of steps : " << updateSteps << std::endl;
        }
        break;
        case SequencerApi::FIRMWARE_UPGRADE_INFO:
        {
            console::FirmwareUpgradeStatus* fwUpgradeInfo = (console::FirmwareUpgradeStatus*)value;
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
                        std::cout<< "The firmware has updated successfully. Sonar will now reboot.\n";
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
    }
}

int main( int argc, char* argv[] )
{
    char a = 'a';

    // List of playback file names
    std::vector<std::string> filesList;
//    filesList.push_back( "C:\\GeminiData\\LD20190122\\data_2019-01-22-103513.ecd");

#ifdef WIN32
    std::string recFileLocation = "C:\\GeminiData\\LD";
#ifdef _DATA_LOGGER_
    int RegisterForHotKeys();
    RegisterForHotKeys();
#endif
#else
    std::string recFileLocation( "/home/" );
    char const* home = getenv ("HOME"); // returns current home path, if run as an executable, returns null if run as a service
    if( home )
    {
        recFileLocation = home;
    }
#endif
    bool fOnline = false;

    std::cout<< "SVS5 sequencer library version :  "<< SequencerApi::GetLibraryVersionInfo() << std::endl;

    SequencerApi::StartSvs5( std::bind(
                                &onGeminiMessageReceived,
                                std::placeholders::_1,
                                std::placeholders::_2,
                                std::placeholders::_3 ) );

// Configure ECD logger and start recording by default
#ifdef _DATA_LOGGER_
#ifdef _ECD_LOGGER_
    bool glfLogger = false;; // true for GLF, false for ECD. Default is GLF
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_LOGGER,
                            sizeof(bool),
                            &glfLogger
                            );
#endif
#endif
    SequencerApi::SequencerPingMode sPingMode;
    SequencerApi::SequencerSosConfig sSosConfig;
    // SDK quality parameters
    SequencerApi::SonarImageQualityLevel quality;
    quality.m_performance   = SequencerApi::UL_HIGH_CPU;
    quality.m_screenPixels  = 2048;

    // default orientation is up
    SequencerApi::ESvs5SonarOrientation orientation = SequencerApi::SONAR_ORIENTATION_UP;
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_SONAR_ORIENTATION,
                            sizeof(SequencerApi::ESvs5SonarOrientation),
                            &orientation
                            );

    CHIRP_MODE chMode = CHIRP_AUTO; //Configure auto chirp mode
    // configure chirp auto mode
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_CHIRP_MODE,
                            sizeof(CHIRP_MODE),
                            &chMode
                            );

    // configure to switch frequency automatically between high/low @40m ( Only for 1200ik )
    RangeFrequencyConfig rangeResolution;
    rangeResolution.m_frequency        = FREQUENCY_AUTO;
    rangeResolution.m_rangeThreshold   = 40;
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_RANGE_RESOLUTION,
                            sizeof(RangeFrequencyConfig),
                            &rangeResolution
                            );

    bool highRangeResolution = true; //Enable high range resolution
    // configure high range resolution
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_HIGH_RESOLUTION,
                            sizeof(bool),
                            &highRangeResolution
                            );

#ifdef _LOW_PERFORMANCE_CPU_
    // Configure SDK to use lower CPU, please see SonarImageQualityLevel struct for detail
    quality.m_performance   = SequencerApi::LOW_CPU;
    quality.m_screenPixels  = 256;
#endif

    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_CPU_PERFORMANCE,
                            sizeof(quality),
                            &quality
                            );


    // Configure ping mode
    configPingMode( sPingMode );

    // default yo go online
    ConfigOnline( true );

    do
    {
        std::cin >> a;
        switch ( a )
        {
            case 'a': //online / offline
            {
                // Toggle online / offline
                ConfigOnline( fOnline );
                fOnline =! fOnline;
            }
            break;
			case 'b':
			{
                std::cout<< "Press 't' to toggle Free Run\n";
                char c_freerun;
                std::cin >> c_freerun;
                if ( c_freerun == 't')
                {
                    sPingMode.m_bFreeRun =! sPingMode.m_bFreeRun;
                }
                if(sPingMode.m_bFreeRun) {
                    sPingMode.m_extTTLTrigger=false;
                }
                else {
                    std::cout<< "Press 't' to toggle External TTL\n";
                    char c_ttl;
                    std::cin >> c_ttl;
                    if ( c_ttl == 't')
                    {
                        sPingMode.m_extTTLTrigger =! sPingMode.m_extTTLTrigger;
                    }
                }
                // Toggle ping mode state
                configPingMode( sPingMode );
            }
            break;
            break;
            case 'c':
            {
                // Configure Speed of Sound
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_SOUND_VELOCITY,
                                        sizeof(sSosConfig),
                                        &sSosConfig
                                        );
                sSosConfig.m_bUsedUserSos = !sSosConfig.m_bUsedUserSos;
            }
            break;
            case 'd': // get range configuration value already set by the user or the default value
            {
                double rangeLinesInMeter = 0.0;
                // Get range value
                SequencerApi::Svs5GetConfiguration(
                                        SequencerApi::SVS5_CONFIG_RANGE,
                                        sizeof(double),
                                        &rangeLinesInMeter
                                        );
                std::cout<< "Get:: Range line in mteres "<< std::dec << rangeLinesInMeter << std::endl;
            }
            break;
            case 'e': // get gain configuration value already set by the user or the default value
            {
                size_t gain = 0;
                // Get gain value
                SequencerApi::Svs5GetConfiguration(
                                        SequencerApi::SVS5_CONFIG_GAIN,
                                        sizeof(int),
                                        &gain
                                        );
                std::cout<< "Get:: Gain "<< std::dec << gain << std::endl;
            }
            break;
            case 'f': //Configure Record File location
            {
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_FILE_LOCATION,
                                        recFileLocation.size(),
                                        recFileLocation.c_str()
                                        );
            }
            break;
            case 'g': //Set gain settings
            {
                std::cout<< "Please enter Gain value (1-100)%% \n";
                size_t gain;
                std::cin >> gain;
                if ( gain < 1 || gain > 100 )
                {
                    std::cout<< "Invalid gain value specified "<< std::dec << gain << std::endl;
                    continue;
                }
                // Configure Gain value
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_GAIN,
                                        sizeof(int),
                                        &gain
                                        );
            }
            break;
            case 'h': // Get H264 Compression value
            {
                bool b_h264 = true;  //Default to true
                // Get gain value
                SequencerApi::Svs5GetConfiguration(
                                        SequencerApi::SVS5_CONFIG_H264_COMPRESSION,
                                        sizeof(bool),
                                        &b_h264
                                        );

                std::cout<< "Get:: H264 Compression "<< std::boolalpha << b_h264 << std::endl;

            }
            break;
            case 'i': //Toggle high range resolution
            {
                static bool highRangeResolution = true;
                // configure high range resolution
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_HIGH_RESOLUTION,
                                        sizeof(bool),
                                        &highRangeResolution
                                        );
                highRangeResolution = !highRangeResolution;
            }
            break;
            case 'j': //Configure chirp mode
            {
                CHIRP_MODE chMode = CHIRP_AUTO;
                // configure high range resolution
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_CHIRP_MODE,
                                        sizeof(CHIRP_MODE),
                                        &chMode
                                        );
            }
            break;
            case 'k':
            {
                // configure to switch frequency automatically between high/low @10m ( Only for 1200ik )
                RangeFrequencyConfig rangeResolution;
                rangeResolution.m_frequency        = FREQUENCY_AUTO;
                rangeResolution.m_rangeThreshold   = 10;
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_RANGE_RESOLUTION,
                                        sizeof(RangeFrequencyConfig),
                                        &rangeResolution
                                        );
            }
            break;
            case 'l': //Loop Playback
            {
                static bool repeat = true;
                // configure playback repeat mode
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_PLAY_REPEAT,
                                        sizeof(bool),
                                        &repeat
                                        );
                repeat = !repeat;
            }
            break;
            case 'm': //Record start
            {
                static bool recSTart = true;
                // Start recording
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_REC,
                                        sizeof(bool),
                                        &recSTart
                                        );
                recSTart = !recSTart;
            }
            break;
            case 'n': //reboot sonar
            {
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_REBOOT_SONAR,
                                        0,
                                        NULL
                                        );
            }
            break;
            case 'o': //Configure sonar orientation
            {
                SequencerApi::ESvs5SonarOrientation orientation = SequencerApi::SONAR_ORIENTATION_UP;
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_SONAR_ORIENTATION,
                                        sizeof(SequencerApi::ESvs5SonarOrientation),
                                        &orientation
                                        );
            }
            break;
            case 'p': //Playback start
            {
                if( filesList.size() == 0 )
                {
                    std::cout<< "No file selected to playback !!" << std::endl;
                    continue;
                }
                bool playbackSTart = true;
                bool pause = true;
                SequencerApi::ListOfFileNames listOfFileNames( filesList );

                // Start playback
                if( SequencerApi::Svs5SetConfiguration(
                                    SequencerApi::SVS5_CONFIG_PLAY_START,
                                    sizeof(listOfFileNames),
                                    &listOfFileNames
                                    ) != SVS5_SEQUENCER_STATUS_OK )
                {
                    continue;
                }

                while( playbackSTart )
                {
                    double speed = 0;
                    char subCommand = ' ';
                    std::cin >> subCommand;
                    switch( subCommand )
                    {
                        case 'p':
                        {
                        playbackSTart = false;
                        // Stop playback
                        SequencerApi::Svs5SetConfiguration(
                                                SequencerApi::SVS5_CONFIG_PLAY_STOP,
                                                0,
                                                NULL
                                                );
                        }
                        break;
                        case 's':
                        {
                            // Pause / resume playback
                            SequencerApi::Svs5SetConfiguration(
                                                    SequencerApi::SVS5_CONFIG_PLAY_PAUSE,
                                                    sizeof(bool),
                                                    &pause
                                                    );
                            pause = !pause;
                        }
                        break;
                        case 'f': //play brame by frame
                        {
                            UInt32 frame = 0;
                            std::cin >> frame;
                            // first pause the video then request for frame
                            SequencerApi::Svs5SetConfiguration(
                                                    SequencerApi::SVS5_CONFIG_PLAY_FRAME,
                                                    sizeof(UInt32),
                                                    &frame
                                                    );
                        }
                        break;
                        case '0': // Set speed ( run as fast as possible )
                        {
                            speed = 0.0;
                            SequencerApi::Svs5SetConfiguration(
                                                    SequencerApi::SVS5_CONFIG_PLAY_SPEED,
                                                    sizeof(double),
                                                    &speed
                                                    );
                        }
                        break;

                        case '1': // Set speed ( 1x real time )
                        {
                            speed = 1.0;
                            SequencerApi::Svs5SetConfiguration(
                                                    SequencerApi::SVS5_CONFIG_PLAY_SPEED,
                                                    sizeof(double),
                                                    &speed
                                                    );
                        }
                        break;
                    }
                }
            }
            break;
            case 'r': //Set range lines in meter
            {
                std::cout<< "Please enter range line (1-120)meter \n";
                double rangeLinesInMeter;
                std::cin >> rangeLinesInMeter;
                if ( rangeLinesInMeter < 1 || rangeLinesInMeter > 120 )
                {
                    std::cout<< "Invalid Range line specified "<< std::dec << rangeLinesInMeter << std::endl;
                    continue;
                }
                // Configure Range
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_RANGE,
                                        sizeof(double),
                                        &rangeLinesInMeter
                                        );
            }
            break;
            case 's': //Enable / disable Simulation
            {
                static bool enableSim = true;
                // configure simulation mode
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_SIMULATE_ADC,
                                        sizeof(bool),
                                        &enableSim
                                        );
                enableSim = !enableSim;
            }
            break;
            case 't': //Configure screen resolution
            {
                char q;
                std::cout<< "Please select image quality ( 0 - 3 ) " << std::endl;
                std::cin >> q;
                quality.m_screenPixels = SequencerApi::LOW_CPU;

                switch( q )
                {
                case 0:
                    quality.m_screenPixels = 256;
                break;
                case 1:
                    quality.m_screenPixels = 512;
                break;
                case 2:
                    quality.m_screenPixels = 1024;
                break;
                case 3:
                    quality.m_screenPixels = 2048;
                break;
                default:
                    std::cout<< "Error: Unknow image quality specified !! " << std::endl;
                    continue;
                }
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_CPU_PERFORMANCE,
                                        sizeof(quality),
                                        &quality
                                        );
            }
            break;
            case 'u':
            {
                bool forceUpdate = false;
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_UPGRADE_FIRMWARE,
                                        sizeof(forceUpdate),
                                        &forceUpdate,
                                        sg_sonarID
                                        );
            }
            break;
            case 'v': //Configure sonar aperture (Switch between 120/65 degrees)
            {
                static double aperture = 120.0;
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_APERTURE,
                                        sizeof(double),
                                        &aperture
                                        );
                aperture = aperture >= 120.0 ? 65.0 : 120.0;
            }
            break;
            case 'w': // Set H264 Compression value
            {
                char c_h264 = 'y';  //Default to Yes
                bool b_h264 = true;

                std::cout<< "Enable H264 Compression (y/n) \n";

                std::cin >> c_h264;

                if(c_h264 == 'y' || c_h264 == 'n') {
                    if(c_h264 == 'n') {
                        b_h264 = false;
                    }
                    // Get gain value
                    SequencerApi::Svs5SetConfiguration(
                                            SequencerApi::SVS5_CONFIG_H264_COMPRESSION,
                                            sizeof(bool),
                                            &b_h264
                                            );

                    std::cout<< "Set:: H264 Compression "<< std::boolalpha << b_h264 << std::endl;
                 }
                else  {
                    std::cout<< "Try again, press 'w' and then 'y' or 'n'." << std::endl;
                }
            }
            break;
            case 'y':
            {
#ifdef WIN32
                const char* firmwareLocationPath = "C:\\Program Files (x86)\\Tritech\\Genesis\\Application Files\\data";
#else
                const char* firmwareLocationPath = "/home/firmware/data";
#endif
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_FIRMWARE_LOCATION,
                                        strlen(firmwareLocationPath),
                                        firmwareLocationPath,
                                        sg_sonarID
                                        );
            }
            break;
            case '0':
            {
                quality.m_performance = SequencerApi::LOW_CPU;
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_CPU_PERFORMANCE,
                                        sizeof(quality),
                                        &quality
                                        );
            }
            break;
            case '1':
            {
                quality.m_performance =  SequencerApi::MEDIUM_CPU;
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_CPU_PERFORMANCE,
                                        sizeof(quality),
                                        &quality
                                        );
            }
            break;
            case '2':
            {
                quality.m_performance =  SequencerApi::HIGH_CPU;
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_CPU_PERFORMANCE,
                                        sizeof(quality),
                                        &quality
                                        );
            }
            break;
            case '3':
            {
                quality.m_performance =  SequencerApi::UL_HIGH_CPU;
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_CPU_PERFORMANCE,
                                        sizeof(quality),
                                        &quality
                                        );
            }
            break;
            case '4':
            {
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_LOG_RAW_GPS,
                                        strlen(GPS), (const char* const)GPS
                                        );
            }
            break;
            case '5':
            {
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_LOG_RAW_COMPASS,
                                        strlen(COMPASS), (const char* const)COMPASS
                                        );
            }
            break;
            case '6':
            {
                SequencerApi::AuxPortConfig auxPortConfig;
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_AUX_PORT,
                                        sizeof(SequencerApi::AuxPortConfig), (const char* const)&auxPortConfig
                                        );
            }
            break;
            case 'z':
                SequencerApi::StopSvs5();
                break;
        }
    }while ( a != 'z');

    return 0;
}
/*
** Config sonar to online / offline
*/
static void ConfigOnline( bool fOnline )
{
    // configure sonar to online / offline
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_ONLINE,
                            sizeof(bool),
                            &fOnline
                            );
}

/*
** Configure application ping mode ( Run as fast as possible or configure sonar to run at a fixed interval )
*/
static void configPingMode( const SequencerApi::SequencerPingMode& sPingMode )
{
    // Configure ping mode
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_PING_MODE,
                            sizeof(sPingMode),
                            &sPingMode
                            );
}

#ifdef _DATA_LOGGER_
void onRangeUp()
{
    double range = 1.0;
    // Configure Range
    SequencerApi::Svs5GetConfiguration(
                            SequencerApi::SVS5_CONFIG_RANGE,
                            sizeof(double),
                            &range
                            );


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
    // Configure Range
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_RANGE,
                            sizeof(double),
                            &range
                            );
    std::cout << "Range configured to " << std::setprecision(2)<< range << std::endl;
}
void onRangeDown()
{
    double range = 1.0;
    // Configure Range
    SequencerApi::Svs5GetConfiguration(
                            SequencerApi::SVS5_CONFIG_RANGE,
                            sizeof(double),
                            &range
                            );

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
    // Configure Range
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_RANGE,
                            sizeof(double),
                            &range
                            );
    std::cout << "Range configured to " << std::setprecision(2)<< range << std::endl;
}
void onRange( double range )
{
    // Configure Range
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_RANGE,
                            sizeof(double),
                            &range
                            );
    std::cout << "Range configured to " << std::setprecision(2) << range << std::endl;
}

void onSetFrameRateInterval( unsigned short int msInterval )
{
    SequencerApi::SequencerPingMode sPingMode;
    sPingMode.m_msInterval = msInterval;
    sPingMode.m_bFreeRun = false; // Fixed interval

    // Configure ping mode
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_PING_MODE,
                            sizeof(sPingMode),
                            &sPingMode
                            );

}

void onStartStoplog( bool recSTart )
{
    // recording
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_REC,
                            sizeof(bool),
                            &recSTart
                            );

    std::cout << "Logging " << (recSTart ? "on" : "off") << std::endl;
}
#endif

