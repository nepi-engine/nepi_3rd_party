#pragma once

#include "console/ciheader.h"

#include <string.h>
namespace GLF
{

#define MAX_FILES_SUPPORTED         10
#define MAX_FILENAME_LENGTH         256
#define MAX_ERRORCODE_STR_LENGTH    512

/*!
 * \struct SInputFileListInfo
 * \brief Structure to give information on the contents and processing
 *        state of the files in the list.
 */
struct SInputFileListInfo {
    UInt32 m_uiNumberOfRecords;   /*!< Number of records in total */
    UInt32 m_uiPercentProcessed;  /*!< Percent of total list processed */
    UInt32 m_uiNumberOfFiles;     /*!< Number of files processed or being processed */
    char   m_filenames[MAX_FILES_SUPPORTED][MAX_FILENAME_LENGTH];           /*!< List of filenames */
    SInputFileListInfo()
    {
        m_uiNumberOfRecords = m_uiPercentProcessed = m_uiNumberOfFiles = 0;
        for ( UInt32 i = 0; i < MAX_FILES_SUPPORTED; ++i )
        {
            memset( &m_filenames[ i ], '\0', sizeof(m_filenames[ i ]) );
        }
    }
    SInputFileListInfo& operator= (const SInputFileListInfo& sInputFileListInfo )
    {
        m_uiNumberOfRecords     = sInputFileListInfo.m_uiNumberOfRecords;
        m_uiPercentProcessed    = sInputFileListInfo.m_uiPercentProcessed;
        m_uiNumberOfFiles       = sInputFileListInfo.m_uiNumberOfFiles;
        for ( UInt32 i = 0; i < m_uiNumberOfFiles; ++i )
        {
            memcpy( &m_filenames[ i ], &sInputFileListInfo.m_filenames[ i ], sizeof(m_filenames[ i ]) );
        }
        return *this;
    }
    SInputFileListInfo(const SInputFileListInfo& sInputFileListInfo )
    {
        *this = sInputFileListInfo;
    }
};


/*!
 * \struct SOutputFileInfo
 * \brief Structure to give information on the file being recorded.
 */
struct SOutputFileInfo {
    SOutputFileInfo()
    {
        memset( m_strFileName, '\0', sizeof( m_strFileName ) );
        m_uiNumberOfRecords     = 0;
        m_fileSizeBytes         = 0;
        m_diskSpaceFreeBytes    = 0;
        m_percentDiskSpaceFree  = 0;
        // Above members can be initialised as soon as the file is opened, so a scary
        // zero disk space message should never erroneously happen. m_timeLeftSecs
        // can only be calculated after the first ping is written and a file size/time taken
        // calculation is done. So, initialise it to 26 hours the first time to avoid alarm.
        m_recordingTimeLeftSecs = 26 * 60 * 60;
    }
    SOutputFileInfo& operator= (const SOutputFileInfo& sOutputFileInfo )
    {
        m_uiNumberOfRecords     = sOutputFileInfo.m_uiNumberOfRecords;
        m_fileSizeBytes         = sOutputFileInfo.m_fileSizeBytes;
        m_diskSpaceFreeBytes    = sOutputFileInfo.m_diskSpaceFreeBytes;
        m_percentDiskSpaceFree  = sOutputFileInfo.m_percentDiskSpaceFree;
        m_recordingTimeLeftSecs = sOutputFileInfo.m_recordingTimeLeftSecs;
        memcpy( m_strFileName, sOutputFileInfo.m_strFileName, sizeof( m_strFileName ) );
        return *this;
    }
    SOutputFileInfo(const SOutputFileInfo& sOutputFileInfo )
    {
        *this = sOutputFileInfo;
    }
    char        m_strFileName[ MAX_FILENAME_LENGTH ];
    UInt32      m_uiNumberOfRecords;
    UInt64      m_fileSizeBytes;
    UInt64      m_diskSpaceFreeBytes;
    double      m_percentDiskSpaceFree;
    UInt64      m_recordingTimeLeftSecs;
 };

struct GnsLoggerRecStatus: public console::PublicMessageHeader
{
    SOutputFileInfo         m_loggerRecStatus;
};

struct GnsLoggerPlaybackStatus: public console::PublicMessageHeader
{
    SInputFileListInfo      m_loggerPlayStatus;
};

struct GnsLoggerFileIndex: public console::PublicMessageHeader
{
    UInt32                  m_fileIndex;
};

// Message type reported with the callback SigInformationString.
enum ErrorType{
    INFO_TYPE,
    WARNING_TYPE,
    ERROR_TYPE
};

struct GnsLoggerStatusInfo: public console::PublicMessageHeader
{
    char                    m_loggerStatusInfo[ MAX_ERRORCODE_STR_LENGTH ];
    ErrorType               m_errType;// severity of error type
};


}

