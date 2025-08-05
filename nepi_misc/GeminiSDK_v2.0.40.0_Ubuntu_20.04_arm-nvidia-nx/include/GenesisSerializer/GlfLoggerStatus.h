#pragma once

#include "GlfLoggerStatusStructure.h"
#include <functional>

namespace GLF
{

/*!
 * \brief SigFileListInfo
 * Slot type definition for an event giving information on the file list as it is being processed.
 */
typedef std::function<void (SInputFileListInfo) > SigFileListInfo;


/*!
 * \brief SigOutputFileInfo
 * slot type for an event giving information on the file being recorded.
 */
typedef std::function<void (const SOutputFileInfo*) > SigOutputFileInfo;


/*!
 * \brief SigActiveFileIndex
 * slot type for an event giving information on the index
 * of the file currently being replayed.
 */
typedef std::function<void (UInt32) > SigActiveFileIndex;


/*!
 * \brief SigInformationString
 * Slot type definition for an event giving information in a string.
 */
typedef std::function<void (ErrorType, const char*) > SigInformationString;

/*!
 * \brief struct GlfRecord
 * Return in response of the GetRecord
 */
struct GlfRecord{
    GlfRecord()
    : m_glfRecord (NULL )
    , m_frame( 0 )
    , m_frameIntervalMSecs( 0 )
    {
    }
    void*       m_glfRecord;            // record type indicate the type of the data added as a payload
    UInt32      m_frame;                // returning frame number
    UInt32      m_frameIntervalMSecs;   // interval from the last frame
};

}
