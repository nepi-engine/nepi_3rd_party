#include "Svs5Seq/Svs5SequencerApi.h"
#include "GenesisSerializer/GlfApi.h"
#include "GenesisSerializer/GenericDataTypes.h"
#include "GenesisSerializer/GpsRecord.h"
#include "GenesisSerializer/CompassRecord.h"
#include <iostream>

#include <iomanip>
#include <limits>

static void onMessageReceived(
                            unsigned int        msgType,
                            size_t              size,
                            const char* const   value
                            )
{
    switch( msgType)
    {
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
            << " Frame  " << logTgtImage->m_frame
            << std::endl;
        }
        break;
        case SequencerApi::LOGGER_FILE_INDEX:
        {
            // If multiple files have been passed to the library then this
            // message contains the playback file index
            std::cout<< "Playing file index..." << *(int*)value<< std::endl;
        }
        break;
        // This is to report error message e.g. failed to open file etc..
        case SequencerApi::LOGGER_STATUS_INFO:
        {
            GLF::GnsLoggerStatusInfo* statusInfo = (GLF::GnsLoggerStatusInfo*)value;
            std::cout<< "Logger status info...\r\n" << statusInfo->m_errType << statusInfo->m_loggerStatusInfo<< std::endl;
        }
        break;
        case SequencerApi::GPS_RECORD:
        {
            GLF::GLogV4ReplyMessage* gnsV4ReplyMsg = (GLF::GLogV4ReplyMessage*)value;
            std::cout<< "GPS data received at " << std::setprecision (15)
                << gnsV4ReplyMsg->m_header.m_ciHeader.m_timestamp << std::endl;
            std::vector<unsigned char>& vecGps = *gnsV4ReplyMsg->m_v4GenericRec.m_vecData;

            // Raw ascii
            if( gnsV4ReplyMsg->m_header.m_ciHeader.m_dataType == console::DATA_TYPE_RAW_SERIAL )
            {
                for( size_t i = 0; i < vecGps.size(); ++i )
                {
                    std::cout<< vecGps[ i ];
                }
            } //V4 recorded message
            else if( gnsV4ReplyMsg->m_header.m_ciHeader.m_dataType == console::DATA_TYPE_GENERIC )
            {
                //GLF::Gps
                GLF::GpsDataRecord* pNewFixRec = (GLF::GpsDataRecord*)&vecGps[0];
                printf("Received GPS Fix rec (Lat = %.4f, Lon = %.4f, E = %.3f, N = %.3f, SOG %.3f, CGOG %.3f)\n",
                       pNewFixRec->m_llRec.m_latDegrees, pNewFixRec->m_llRec.m_longDegrees,
                       pNewFixRec->m_enRec.m_easting, pNewFixRec->m_enRec.m_northing,
                        pNewFixRec->m_sog, pNewFixRec->m_cog
                    );
            }
            std::cout<< "\t Frame Number " << gnsV4ReplyMsg->m_uiFrame <<std::endl;
        }
        break;
        case SequencerApi::COMPASS_RECORD:
        {
            GLF::GLogV4ReplyMessage* gnsV4ReplyMsg = (GLF::GLogV4ReplyMessage*)value;
            std::cout<< "Compass data received at " << std::setprecision (15) << gnsV4ReplyMsg->m_header.m_ciHeader.m_timestamp <<std::endl;
            std::vector<unsigned char>& vecCompass = *gnsV4ReplyMsg->m_v4GenericRec.m_vecData;

            if( gnsV4ReplyMsg->m_header.m_ciHeader.m_dataType == console::DATA_TYPE_RAW_SERIAL ) // Raw ascii
            {
                for( size_t i = 0; i < vecCompass.size(); ++i )
                {
                    std::cout<< vecCompass[ i ];
                }
            }
            else if( gnsV4ReplyMsg->m_header.m_ciHeader.m_dataType == console::DATA_TYPE_GENERIC ) //V4 Recorded message
            {
                GLF::CompassDataRecord* pNewRec = (GLF::CompassDataRecord*)&vecCompass[0];
                // output Heading, Pitch and Roll to test...
                printf("Received Compass Rec (Hdg = %.2f, Pitch = %.2f, Roll = %.2f)\n",
                       pNewRec->m_heading, pNewRec->m_pitch, pNewRec->m_roll);
            }
            std::cout<< "\t Frame Number " << gnsV4ReplyMsg->m_uiFrame <<std::endl;
        }
        break;
    }
}

int main( int argc, char* argv[] )
{
    char a = 'a';
    bool pause = true;
    double speed = 1.0;
    // List of playback file names
    std::vector<std::string> filesList;
    filesList.push_back( "Please specify the .glf filename including full path");

    std::cout<< "SVS5 sequencer library version :  "<< SequencerApi::GetLibraryVersionInfo() << std::endl;
    SequencerApi::StartSvs5( std::bind(
                        &onMessageReceived,
                        std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3 ) );

    // This example provides playbck function
    // user can start/stop playback and also can configure in the loopback mode
    // Press 'z' to exit the application
    do
    {
        std::cin >> a;
        switch ( a )
        {
            case 'p': //Playback start
            {
                if( filesList.size() == 0 )
                {
                    std::cout<< "No file selected to playback !!" << std::endl;
                    continue;
                }
                // convect vector into char*
                SequencerApi::ListOfFileNames listOfFileNames( filesList );

                // Start playback
                SequencerApi::Svs5SetConfiguration(
                                    SequencerApi::SVS5_CONFIG_PLAY_START,
                                    sizeof(listOfFileNames),
                                    &listOfFileNames
                                    );
            }
            break;
            case 'n': //Playback stop
            {
                // Stop playback
                SequencerApi::Svs5SetConfiguration(
                                        SequencerApi::SVS5_CONFIG_PLAY_STOP,
                                        0,
                                        NULL
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

            case 'z':
                SequencerApi::StopSvs5();
                break;
        }
    }while ( a != 'z');

    return 0;
}


