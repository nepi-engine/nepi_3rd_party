#include "common.cpp"

/*
** This utility reads GLF log files in synchronous mode.
*/
int main()
{
    std::vector<std::string> glfFilesList;
    //glfFilesList.push_back("GLF File Name 1.glf");
    //glfFilesList.push_back("GLF File Name 2.glf");

    const char* chFileList[ 255 ];
    UInt32 numberofFiles = 0;
    for (auto it = glfFilesList.begin(); it != glfFilesList.end(); it++)
    {
        chFileList[ numberofFiles++ ] = (*it).c_str();
    }

    GLF_HANDLE hReader      = NULL;
    UInt32 uiRecordInfoPos  = 0;
    GLF::GlfRecord glfRecord;

    // Create GLF logger synchronous mode. This call will only return after generating index table
    GLF::CreateLogFileReader(
                    &hReader,
                    chFileList,
                    numberofFiles,
                    std::bind(&updateFileIndexInformation, std::placeholders::_1),
                    std::bind(&updateLoggerPlaybackInfo, std::placeholders::_1),
                    std::bind(&updateStatusString, std::placeholders::_1, std::placeholders::_2),
                    false
                    );
    // Starts reading log data frame
    while ( GLF::GetRecord( hReader, uiRecordInfoPos, glfRecord ) == GLF_OK )
    {
        // play the recorded frame
        PlayRecord( glfRecord.m_glfRecord, glfRecord.m_frame, glfRecord.m_frameIntervalMSecs );

        ++uiRecordInfoPos;
    }
    std::cout<< "Total record accessed..." << uiRecordInfoPos << std::endl;

    // Close logger
    GLF::CloseLogFileHandler( hReader );

    return 0;
}


