#include "common.cpp"

/*
** This utility reads GLF log files in asynchronous mode.
*/
int main()
{
    std::vector<std::string> glfFilesList;
    //glfFilesList.push_back("GLF File Name 1.glf");
    //glfFilesList.push_back("GLF File Name 2.glf");

    if( !glfFilesList.size() )
    {
        std::cout<< "Please specify filename !!" << std::endl;
        return -1;
    }
    const char* chFileList[ 255 ];
    UInt32 numberofFiles = 0;
    for (auto it = glfFilesList.begin(); it != glfFilesList.end(); it++)
    {
        chFileList[ numberofFiles++ ] = (*it).c_str();
    }

    GLF_HANDLE hReader      = NULL;
    UInt32 uiRecordInfoPos  = 0;
    GLF::GlfRecord glfRecord;
    UInt32 errorCode;

    // Create GLF logger asynchronous mode. This call will return immediately
    // user can set wait or do some processing if required
    errorCode = GLF::CreateLogFileReader(
                    &hReader,
                    chFileList,
                    numberofFiles,
                    std::bind(&updateFileIndexInformation, std::placeholders::_1),
                    std::bind(&updateLoggerPlaybackInfo, std::placeholders::_1),
                    std::bind(&updateStatusString, std::placeholders::_1, std::placeholders::_2),
                    true
                    );

    if (errorCode != GLF_OK)
    {
        std::cout << "Failed to create logger !!" << std::endl;
        return -1;
    }

    while ( true )
    {
        errorCode = GLF::GetRecord( hReader, uiRecordInfoPos, glfRecord );
        if( errorCode == GLF_BUSY_PROCESSING_INDEX_TABLE )
        {
            // Wait for some time or do somethhing else
            continue;
        }
        if( errorCode == GLF_DATA_NOT_AVAILABLE )
        {
            break;
        }
        // play the recorded frame
        PlayRecord( glfRecord.m_glfRecord, glfRecord.m_frame, glfRecord.m_frameIntervalMSecs );
        ++uiRecordInfoPos;
    }
    std::cout<< "Total record accessed..." << uiRecordInfoPos << std::endl;

    // Close logger
    GLF::CloseLogFileHandler( hReader );

    return 0;
}

