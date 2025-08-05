#pragma once

#include "types.h"
#include "console_global_types.h"

using namespace tdi;

namespace console
{

typedef enum{
    DATA_TYPE_SVS5              = 0,
    DATA_TYPE_V4                = 1,
    DATA_TYPE_ANALOG_VIDEO      = 2,
    DATA_TYPE_GEMINI_STATUS     = 3,
    DATA_TYPE_REM_V4            = 4,
    DATA_TYPE_3DP_DATA          = 5,
    DATA_TYPE_RAW_SERIAL        = 98,
    DATA_TYPE_GENERIC           = 99,
}CI_DATA_TYPES;

#pragma pack (push, 1)

//Interface class for Node table
class AFX_EXT_CONSOLE_INTERFACE CommonInterfaceHeader{
public:
    CommonInterfaceHeader()
    {
        m_idChar    = '*';
        m_version   = 2;
        m_length    = 0;
        m_timestamp = 0.0;
        m_dataType  = DATA_TYPE_SVS5;
        m_deviceID  = 0;
        m_nodeID    = 0;
        m_spare     = 0;
    }

    CommonInterfaceHeader& operator =( const CommonInterfaceHeader& cgiHeader )
    {
        m_idChar    = cgiHeader.m_idChar;
        m_version   = cgiHeader.m_version;
        m_length    = cgiHeader.m_length;
        m_timestamp = cgiHeader.m_timestamp;
        m_dataType  = cgiHeader.m_dataType;
        m_deviceID  = cgiHeader.m_deviceID;
        m_nodeID    = cgiHeader.m_nodeID;
        m_spare     = cgiHeader.m_spare;
        return *this;
    }

    UInt8   m_idChar;               // Start character used to signify start of CI header (Always = “*”)
    UInt8   m_version;              // Version number of the CI Header
    UInt32  m_length;               // Total length (number of bytes) of CI Header and Data message.
    double  m_timestamp;            // Date / Time in UTC with millisecond resolution (format TBC)

    UInt8   m_dataType;             // Type of data contained in attached data record
                                    // 0 = SVS5
                                    // 1 = V4
                                    // 2 = Analog_Video
                                    // 3 = Gemini Status message
                                    // 4 = RemV4 data
                                    // .
                                    // 98 = Raw_Serial
                                    // 99 = Generic (see Section 6.12.3)

    UInt16  m_deviceID;             // This will be:
                                    // A Sonar ID number when m_dataType = 0
                                    // An incrementing deviceID when m_dataType = 99
                                    //(e.g. 1st GPS: m_deviceID = 1, 2nd GPS: m_deviceID = 2, etc)

    UInt16  m_nodeID;               // node ID.

    UInt16  m_spare;                // Reserved for future expansion
};

struct AFX_EXT_CONSOLE_INTERFACE PublicMessageHeader
{
    CommonInterfaceHeader m_ciHeader;
};

#pragma pack (pop)

/////////////////////////////////////////////////////////////
} // namespace console



