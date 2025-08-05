#pragma once

#include "console/ciheader.h"
#include "GlfLoggerGlobalTypes.h"

/*
** Supported message types are currently logged
*/

namespace GLF
{

#pragma pack(push, 1)

/*!
 * \struct GeminiStatusRecord
 * \brief A base class containing status message received from the Gemini sonar
 */
struct AFX_EXT_GLF_INTERFACE GeminiStatusRecord
{
    //default constructor
    GeminiStatusRecord()
    {
        reset();
    }

    void reset()
    {
        m_bfVersion          = 0;
        m_daVer              = 0;
        m_flags              = 0;
        m_deviceID           = 0;
        m_xdSelected         = 0xFF;

        m_vgaT1              = 0.0;
        m_vgaT2              = 0.0;
        m_vgaT3              = 0.0;
        m_vgaT4              = 0.0;
        m_psuT               = 0.0;
        m_dieT               = 0.0;
        m_txT                = 0.0;

        m_afe0TopTemp        = 0.0;
        m_afe0BotTemp        = 0.0;
        m_afe1TopTemp        = 0.0;
        m_afe1BotTemp        = 0.0;
        m_afe2TopTemp        = 0.0;
        m_afe2BotTemp        = 0.0;
        m_afe3TopTemp        = 0.0;
        m_afe3BotTemp        = 0.0;

        m_linkType           = 0;
        m_uplinkSpeedMbps    = 0.0;
        m_downlinkSpeedMbps  = 0.0;
        m_linkQuality        = 0;
        m_packetCount        = 0;
        m_recvErrorCount     = 0;
        m_resentPacketCount  = 0;
        m_droppedPacketCount = 0;
        m_unknownPacketCount = 0;
        m_lostLineCount      = 0;
        m_generalCount       = 0;

        m_sonarAltIp         = 0;
        m_surfaceIp          = 0;
        m_subNetMask         = 0x00ffffff;
        m_macAddress1        = 0;
        m_macAddress2        = 0;
        m_macAddress3        = 0;
        m_BOOTSTSRegister    = 0;
        m_BOOTSTSRegisterDA  = 0;
        m_fpgaTime           = 0;
        m_dipSwitch          = 0;

        m_shutdownStatus     = 0;
        m_networkAdaptorFound = false;
        m_subSeaInternalTemp = 0.0;
        m_subSeaCpuTemp      = 0.0;
    }

    GeminiStatusRecord& operator =( const GeminiStatusRecord& gStatus )
    {
        m_bfVersion          = gStatus.m_bfVersion;
        m_daVer              = gStatus.m_daVer;
        m_flags              = gStatus.m_flags;
        m_deviceID           = gStatus.m_deviceID;
        m_xdSelected         = gStatus.m_xdSelected;

        m_vgaT1              = gStatus.m_vgaT1;
        m_vgaT2              = gStatus.m_vgaT2;
        m_vgaT3              = gStatus.m_vgaT3;
        m_vgaT4              = gStatus.m_vgaT4;
        m_psuT               = gStatus.m_psuT;
        m_dieT               = gStatus.m_dieT;
        m_txT                = gStatus.m_txT;

        m_afe0TopTemp        = gStatus.m_afe0TopTemp;
        m_afe0BotTemp        = gStatus.m_afe0BotTemp;
        m_afe1TopTemp        = gStatus.m_afe1TopTemp;
        m_afe1BotTemp        = gStatus.m_afe1BotTemp;
        m_afe2TopTemp        = gStatus.m_afe2TopTemp;
        m_afe2BotTemp        = gStatus.m_afe2BotTemp;
        m_afe3TopTemp        = gStatus.m_afe3TopTemp;
        m_afe3BotTemp        = gStatus.m_afe3BotTemp;

        m_linkType           = gStatus.m_linkType;
        m_uplinkSpeedMbps    = gStatus.m_uplinkSpeedMbps;
        m_downlinkSpeedMbps  = gStatus.m_downlinkSpeedMbps;
        m_linkQuality        = gStatus.m_linkQuality;
        m_packetCount        = gStatus.m_packetCount;
        m_recvErrorCount     = gStatus.m_recvErrorCount;
        m_resentPacketCount  = gStatus.m_resentPacketCount;
        m_droppedPacketCount = gStatus.m_droppedPacketCount;
        m_unknownPacketCount = gStatus.m_unknownPacketCount;
        m_lostLineCount      = gStatus.m_lostLineCount;
        m_generalCount       = gStatus.m_generalCount;

        m_sonarAltIp         = gStatus.m_sonarAltIp;
        m_surfaceIp          = gStatus.m_surfaceIp;
        m_subNetMask         = gStatus.m_subNetMask;
        m_macAddress1        = gStatus.m_macAddress1;
        m_macAddress2        = gStatus.m_macAddress2;
        m_macAddress3        = gStatus.m_macAddress3;
        m_BOOTSTSRegister    = gStatus.m_BOOTSTSRegister;
        m_BOOTSTSRegisterDA  = gStatus.m_BOOTSTSRegisterDA;
        m_fpgaTime           = gStatus.m_fpgaTime;
        m_dipSwitch          = gStatus.m_dipSwitch;

        m_shutdownStatus     = gStatus.m_shutdownStatus;
        m_networkAdaptorFound = gStatus.m_networkAdaptorFound;
        m_subSeaInternalTemp = gStatus.m_subSeaInternalTemp;
        m_subSeaCpuTemp      = gStatus.m_subSeaCpuTemp;

        return *this;
    }

    Int16           m_bfVersion;
    Int16           m_daVer;
    Int16           m_flags;
    Int16           m_deviceID;
    UInt8           m_xdSelected;  // Selected transducer, 0xFF: None, 0: Alba, 1: Neptune

    double          m_vgaT1; // Mk2 : FPGA PCB
    double          m_vgaT2; // Mk2 : HSC PCB
    double          m_vgaT3; // Mk2 : DA FPGA
    double          m_vgaT4; // 1200ik : used transducer temperature
    double          m_psuT;  // PSU
    double          m_dieT;  // FPGA ( Die)
    double          m_txT;   // Tx

    double          m_afe0TopTemp;
    double          m_afe0BotTemp;
    double          m_afe1TopTemp;
    double          m_afe1BotTemp;
    double          m_afe2TopTemp;
    double          m_afe2BotTemp;
    double          m_afe3TopTemp;
    double          m_afe3BotTemp;

    // Network communications
    UInt16          m_linkType;
    double          m_uplinkSpeedMbps;
    double          m_downlinkSpeedMbps;
    UInt16          m_linkQuality;
    UInt32          m_packetCount;
    UInt32          m_recvErrorCount;
    UInt32          m_resentPacketCount;
    UInt32          m_droppedPacketCount;
    UInt32          m_unknownPacketCount;
    UInt32          m_lostLineCount;
    UInt32          m_generalCount;

    UInt32          m_sonarAltIp;
    UInt32          m_surfaceIp;
    UInt32          m_subNetMask;
    UInt16          m_macAddress1;
    UInt16          m_macAddress2;
    UInt16          m_macAddress3;
    UInt32          m_BOOTSTSRegister;
    UInt32          m_BOOTSTSRegisterDA;
    UInt64          m_fpgaTime;
    UInt16          m_dipSwitch;

    UInt16          m_shutdownStatus;
    bool            m_networkAdaptorFound;
    double          m_subSeaInternalTemp;
    double          m_subSeaCpuTemp;
};


// Gemini status message
struct AFX_EXT_GLF_INTERFACE GeminiSonarStatusMessage : public console::PublicMessageHeader
{
    UInt16              m_usStatusMsgVersion;   //!< Version number, in case if there is a change in the strcuture
    GeminiStatusRecord  m_geminiSonarStatus;    //!< Gemini sonar status message
    UInt32              m_uiFrame;      // Not serialized : Only used in the playback mode.

    GeminiSonarStatusMessage(); // default body has already been defined in serializer library
    GeminiSonarStatusMessage& operator =( const GeminiSonarStatusMessage& gStatusMessage )
    {
        m_ciHeader              = gStatusMessage.m_ciHeader;
        m_usStatusMsgVersion    = gStatusMessage.m_usStatusMsgVersion;
        m_geminiSonarStatus     = gStatusMessage.m_geminiSonarStatus;
        return *this;
    }

    size_t size() const
    {
        return sizeof(GLF::GeminiSonarStatusMessage);   // Size of sonar status message + Common interface header
    }

};

#pragma pack(pop)

}
