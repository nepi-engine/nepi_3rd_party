#include "GenesisSerializer/GlfApi.h"
#include "GenesisSerializer/GenericDataTypes.h"
#include "GenesisSerializer/GpsRecord.h"
#include "GenesisSerializer/CompassRecord.h"
#include <iostream>

static void updateLoggerPlaybackInfo(const GLF::SInputFileListInfo sInputFileListInfo)
{
    // Print total records (frames) 100 frames means ( 0 -99 )
    std::cout<< "Number of Records " << sInputFileListInfo.m_uiNumberOfRecords
        << " Percentage completed " << sInputFileListInfo.m_uiPercentProcessed <<std::endl;
    // Print filenames
    for ( unsigned int i = 0; i < sInputFileListInfo.m_uiNumberOfFiles; ++i )
    {
        std::cout<< "Playingback filename " << sInputFileListInfo.m_filenames[ i ] << std::endl;
    }
}

static void updateFileIndexInformation (const UInt32 uiFileIndex )
{
    std::cout<< "Playing File Index..." << uiFileIndex << std::endl;
}

static void updateStatusString (GLF::ErrorType errType, const char* str )
{
    std::cout<< "Logger status info...\r\n" << errType << str << std::endl;
}

static void PlayRecord(const void* recData, UInt32 frame, UInt32 pingIntervalMSecs)
{
    console::PublicMessageHeader* ciHeader = (console::PublicMessageHeader*)recData;
    // Gemini Record
    if( ciHeader->m_ciHeader.m_dataType == console::DATA_TYPE_SVS5 )  // Gemini Record
    {
        GLF::GenesisPlaybackTargetImage gnsPlaybackTargetImage;
        gnsPlaybackTargetImage.m_pLogTgtImage = (GLF::GLogTargetImage*)recData;
        gnsPlaybackTargetImage.m_frame  = frame;
    }
    /*V4 Reply Record*/
    else if(
        ( ciHeader->m_ciHeader.m_dataType == console::DATA_TYPE_V4 ) ||
        ( ciHeader->m_ciHeader.m_dataType == console::DATA_TYPE_GENERIC )
        )
    {
       GLF::GLogV4ReplyMessage* gnsV4ReplyMsg = (GLF::GLogV4ReplyMessage*)recData;
       gnsV4ReplyMsg->m_uiFrame = frame;
       std::vector<unsigned char>& data = *gnsV4ReplyMsg->m_v4GenericRec.m_vecData;

        if( gnsV4ReplyMsg->m_v4GenericRec.m_ucMsgType == GLF::GenericTypeRecord::GT_RECORD_RAW_ASCII_STRING )
        {
            for( size_t i = 0; i < data.size(); ++i )
            {
                std::cout<< data[ i ];
            }
            std::cout<< std::endl;
        }
        else if( gnsV4ReplyMsg->m_v4GenericRec.m_ucMsgType == GLF::GenericTypeRecord::GT_RECORD_GPS )
        {
            //GLF::Gps
            GLF::GpsDataRecord* pNewFixRec = (GLF::GpsDataRecord*)&data[0];
            printf("GPS Fix rec (Lat = %.4f, Lon = %.4f, E = %.3f, N = %.3f, SOG = %.3f, COG = %.3f)\n",
                   pNewFixRec->m_llRec.m_latDegrees, pNewFixRec->m_llRec.m_longDegrees,
                   pNewFixRec->m_enRec.m_easting, pNewFixRec->m_enRec.m_northing,
                   pNewFixRec->m_sog, pNewFixRec->m_cog
                   );
        }
        else if( gnsV4ReplyMsg->m_v4GenericRec.m_ucMsgType == GLF::GenericTypeRecord::GT_RECORD_COMPASS )
        {
            GLF::CompassDataRecord* pNewRec = (GLF::CompassDataRecord*)&data[0];
            // output Heading, Pitch and Roll to test...
            printf("Compass Rec (Hdg = %.2f, Pitch = %.2f, Roll = %.2f)\n",
                   pNewRec->m_heading, pNewRec->m_pitch, pNewRec->m_roll);
        }
        std::cout<< "\t Frame Number " << gnsV4ReplyMsg->m_uiFrame <<std::endl;

    }
    else if ( ciHeader->m_ciHeader.m_dataType == console::DATA_TYPE_RAW_SERIAL )
    {
        /*Raw GPS/Compass data*/
        GLF::GLogV4ReplyMessage* gnsV4ReplyMsg = (GLF::GLogV4ReplyMessage*)recData;
        gnsV4ReplyMsg->m_uiFrame = frame;
        // This could either GPS or Compass record based on the node type
        // UInt32 msgType = ( ciHeader->m_ciHeader.m_nodeID == 245 ) ? GPS Record : Compass Record
        std::cout<< ( ciHeader->m_ciHeader.m_nodeID == 245 ) ? "GPS " : "Compas ";
        std::vector<unsigned char>& data = *gnsV4ReplyMsg->m_v4GenericRec.m_vecData;
        for( size_t i = 0; i < data.size(); ++i )
        {
            std::cout<< data[ i ];
        }
        std::cout<< "\t Frame Number " << gnsV4ReplyMsg->m_uiFrame <<std::endl;
    }

}


