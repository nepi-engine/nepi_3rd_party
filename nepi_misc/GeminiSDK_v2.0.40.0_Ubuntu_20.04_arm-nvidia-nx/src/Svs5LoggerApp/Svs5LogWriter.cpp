/*
** This is the very simple example of providing functionality of creating Glf log file
** At the start of the application,  will start printing status message, if any one of soar is
** active on the network
*/


#include "Svs5Seq/Svs5SequencerApi.h"
#include "Gemini/GeminiStructuresPublic.h"
#include "GenesisSerializer/GlfApi.h"
#include "GenesisSerializer/GpsRecord.h"
#include "GenesisSerializer/CompassRecord.h"

#include <iostream>


const char TEST_GPS[] = "$GPGGA,143305,4722.68807,N,00813.95741,E,2,07,1.4,0.0,M,0.0,M,2.2,0362*5C";
const char TEST_COMPASS[] = "H170.5P-1.2R-1.1T12.3D501.26B12.3A59W59LN47F0";


// Callback function, installed in the SVS5 Sequencer library
static void onMessageReceived(
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
            std::cout<<  "Status  message received  : "
                << pStatus->m_sonarAltIp << std::endl;

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
            std::cout<< "Sonar status " << s << std::endl;
        }
        break;
        case SequencerApi::GLF_LIVE_TARGET_IMAGE:
        {
            GLF::GLogTargetImage* logTgtImage = (GLF::GLogTargetImage*)value;

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
        case SequencerApi::FRAME_RATE:
        {
            unsigned int fps = *(unsigned int*)value;
            std::cout<< "Frames per seconds " << fps << std::endl;
        }
        break;
    }
}

int main( int argc, char* argv[] )
{
    char a = 'a';

#ifdef WIN32
    std::string recFileLocation = "C:/GlfDataRecord";
#else
    std::string recFileLocation( "/home/" );
    char const* home = getenv ("HOME"); // returns current home path, if run as an executable, returns null if run as a service
    if( home )
    {
        recFileLocation = home;
    }
#endif
    bool fOnline = true;

    std::cout<< "SVS5 sequencer library version :  "<< SequencerApi::GetLibraryVersionInfo() << std::endl;

    SequencerApi::StartSvs5( std::bind(
                                &onMessageReceived,
                                std::placeholders::_1,
                                std::placeholders::_2,
                                std::placeholders::_3 ) );

    // First of all configure the record location
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_FILE_LOCATION,
                            sizeof(std::string),
                            &recFileLocation
                            );

    // configure sonar to online / offline
    SequencerApi::Svs5SetConfiguration(
                            SequencerApi::SVS5_CONFIG_ONLINE,
                            sizeof(bool),
                            &fOnline
                            );

    do
    {
        std::cin >> a;
        switch ( a )
        {
            case 'a': //Record GLF::GpsDataRecord
            {
                GLF::GpsDataRecord gpsDataRec;
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_LOG_GPS,
                                        sizeof(gpsDataRec), (const void* const)&gpsDataRec
                                        );
            }
            break;
            case 'b': //Record  GLF::CompassDataRecord
            {
                GLF::CompassDataRecord compassDataRec;
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_LOG_COMPASS,
                                        sizeof(compassDataRec), (const void* const)&compassDataRec
                                        );
            }
            break;

            case 'm': //Toggle Record start / stop
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

            case 'g': //record raw GPS data
            {
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_LOG_RAW_GPS,
                                        strlen(TEST_GPS),
                                        TEST_GPS
                                        );
            }
            break;
            case 'c': //record raw Compass data
            {
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_LOG_RAW_COMPASS,
                                        strlen(TEST_COMPASS),
                                        TEST_COMPASS
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


