#include "common.cpp"

/*
** This utility reads a GLF log file in a forward direction without creating index table.
*/
int main()
{
    const char* logFile = "GLF File Name.glf";

    GLF_HANDLE hReader      = NULL;
    UInt32 uiRecordInfoPos  = 0;
    GLF::GlfRecord glfRecord;

    // Create logger to read single log file. No index table will be generated and user can only
    // go in forward direction
    if( GLF::CreateSingleFileLogReader( &hReader, logFile ) != GLF_OK )
    {
        std::cout << "Invalid or corrupted GLF log file \n ";
        return -1;
    }

    // Starts reading log data frame
    while ( GLF::GetNextRecord( hReader, glfRecord ) == GLF_OK )
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


