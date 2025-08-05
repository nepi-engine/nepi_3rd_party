#pragma once

#include "types.h"
#include "ciheader.h"

namespace console
{

struct FirmwareUpgrade{
    FirmwareUpgrade()
    : m_firmwareFileName( "" )
    , m_dstDevice( 0 )
    {
    }
    std::string m_firmwareFileName;
    UInt8       m_dstDevice;    // BF:2, DA1:4, DA2:8, DA3:16
};

struct FirmwareUpgradeStatus: public PublicMessageHeader
{
    typedef enum
    {
        FW_UG_IDLE,
        FW_UG_PROGRAMME,
        FW_UG_SUCCESS,
        FW_UG_FAILURE,
        FW_UG_REBOOTING,
        FW_UG_ERASING,
    }FW_UG_STATUS;
    FirmwareUpgradeStatus()
    {
        Reset();
    }
    std::string     m_upgradeStatusInfo;
    UInt32          m_percentComplete;
    FW_UG_STATUS    m_programStatus; // 0: Idle, 1 : Programing, 2: Success, 3: Failure

    void Reset()
    {
        m_percentComplete   = 0;
        m_programStatus     = FW_UG_IDLE;
    }
};

/////////////////////////////////////////////////////////////
} // namespace console



