#pragma once

#include "console/ciheader.h"
#include "GlfLoggerGlobalTypes.h"

#include <vector>

namespace GLF
{

// TODO: Could inherit this off a serializable structure instead
struct AFX_EXT_CONSOLE_INTERFACE PointDataHeader
{
    unsigned long long  m_timestamp;        // UTC timestamp in microseconds
    unsigned long       m_flags;            // Info flags (fixed SOS,
                                            // fixed gain, test modes etc.)
    unsigned long       m_ping_count;       // Ping counter
    unsigned char       m_subping;          // Subping number 1-16 (zero in
                                            // Tx test mode)
    unsigned char       m_tx_beam;          // Tx Beam number 1-16 (Rx test
                                            // only, zero in normal operation)
    signed short        m_ahrs_roll;        // AHRS roll (degrees * 100)
    signed short        m_ahrs_pitch;       // AHRS pitch (degrees * 100)
    signed short        m_ahrs_yaw;         // AHRS yaw (degrees * 100)
    unsigned long       m_sos;              // SOS used for this ping (mm/s)
    unsigned long       m_start_range;      // Start range (mm)
    unsigned long       m_end_range;        // End range (mm)
    unsigned long       m_tx_focus_range;   // Tx focus range (mm)
    unsigned long       m_absorb_coe;       // Absorption coefficient (dB/km)
    unsigned short      m_tx_length;        // Tx pulse length (cycles)
    unsigned short      m_tx_start_chan;    // Tx start channel
    unsigned short      m_tx_num_chans;     // Tx num channels
    unsigned char       m_tx_attenuation;   // Reduction in Tx source level
                                            // (dB)
    unsigned char       m_tx_shading;       // Tx array shading level
                                            // (arbitrary units)
    unsigned char       m_tx_beam_spacing;  // Tx beam spacing
    unsigned char       m_rx_beam_spacing;  // Rx beam spacing
    unsigned short      m_rx_gain;          // Rx gain (dB * 8)
};

/*!
 * \class PointDataRecord
 * \brief
 */
struct AFX_EXT_CONSOLE_INTERFACE PointDataRecord
{
    console::PublicMessageHeader m_header;     // includes common interface header (NB: could inherit from this instead)

    PointDataHeader m_pointDataHeader;
    std::vector<signed short> m_pointData;

    UInt32 m_uiFrame; // NB: only used in the playback mode.

    PointDataRecord()
    {
        m_header.m_ciHeader.m_dataType = console::DATA_TYPE_3DP_DATA;
        m_uiFrame = 0;
    }
    PointDataRecord(const PointDataRecord& in)
    {
        *this = in;
    }

    PointDataRecord& operator =(const PointDataRecord& gLogPointData)
    {
        m_header.m_ciHeader = gLogPointData.m_header.m_ciHeader;
        m_pointDataHeader = gLogPointData.m_pointDataHeader;
        // basic vector copy will do here...
        m_pointData = gLogPointData.m_pointData;
        m_uiFrame = gLogPointData.m_uiFrame;
        return *this;
    }

    size_t size() const
    {
        size_t size = 0;
        size += sizeof(m_header) +
                sizeof(m_pointDataHeader) +
                sizeof(UInt32) +
                (m_pointData.size() * sizeof(signed short));
        return size;
    }

    UInt32 CopyTo(UInt8* buffer) const
    {
        UInt32 index = 0;
        memcpy(&buffer[index], &m_header.m_ciHeader, sizeof(m_header.m_ciHeader));
        index = sizeof(m_header.m_ciHeader);

        memcpy(&buffer[index], &m_pointDataHeader, sizeof(m_pointDataHeader));
        index += sizeof(m_pointDataHeader);

        UInt32 pointDataCount = m_pointData.size();
        memcpy(&buffer[index], &pointDataCount, sizeof(UInt32));
        index += sizeof(UInt32);

        UInt32 pointDataSize = m_pointData.size() * sizeof(signed short);
        memcpy(&buffer[index], m_pointData.data(), pointDataSize);
        index += pointDataSize;
        return index;
    }

    UInt32 CopyFrom (const UInt8* buffer)
    {
        UInt32 index = 0;
        memcpy(&m_header.m_ciHeader, &buffer[index], sizeof(m_header.m_ciHeader));
        index = sizeof(m_header.m_ciHeader);

        memcpy(&m_pointDataHeader, &buffer[index], sizeof(m_pointDataHeader));
        index += sizeof(m_pointDataHeader);

        UInt32 pointDataCount = *((UInt32*)&buffer[index]);
        m_pointData.resize(pointDataCount);
        index += sizeof(UInt32);

        UInt32 pointDataSize = m_pointData.size() * sizeof(signed short);
        memcpy(m_pointData.data(), &buffer[index], pointDataSize);

        return index;
    }
};

}

