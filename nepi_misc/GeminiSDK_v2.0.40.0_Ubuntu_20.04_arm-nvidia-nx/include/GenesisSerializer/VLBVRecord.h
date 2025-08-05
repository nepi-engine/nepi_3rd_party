#pragma once

#include "types.h"

namespace GLF
{
/*!
 * \class VLBVAuxiliaryRecord
 * \brief A class containing all the vehicle auxiliary data received from a SeaBotix vLBV vehicle for display
  */
struct VLBVAuxiliaryRecord {
    VLBVAuxiliaryRecord()
    {
        Reset();
    }
	
    void Reset()
    {
        int    m_auxTurns       = 0;
        int    m_auxAcc         = 0;
        bool   m_auxAutoDepth   = false;
        bool   m_auxAutoHdg     = false;
        int    m_auxHorizGain   = 0;
        int    m_auxVertGain    = 0;
    }

    int    m_auxTurns;
    int    m_auxAcc;
    bool   m_auxAutoDepth;
    bool   m_auxAutoHdg;
    int    m_auxHorizGain;
    int    m_auxVertGain;
};

/*!
 * \class VLBVControlsRecord
 * \brief A class containing all the vehicle control data received from a SeaBotix vLBV vehicle for display
  */
struct VLBVControlsRecord {
    VLBVControlsRecord()
    {
        Reset();
    }

    void Reset()
    {
        m_depthMetres    = 0;
        m_heading        = 0;
        m_pitch          = 0;
        m_roll           = 0;
        m_altitudeMetres = 0;
        for (unsigned int i = 0; i < 8; i++)
        {
            m_lights[i] = 0;
        }
        m_intTemp       = 0;
        m_extTemp       = 0;
        m_camAngle      = 0;
        for (unsigned int j = 0; j < 6; j++)
        {
            m_thrusterRPM[j] = 0;
            m_thrusterFault[j] = 0;
        }
        m_auxRec.Reset();
        m_DataChanged    = 0;
    }

    double m_depthMetres;
    double m_heading;
    double m_pitch;
    double m_roll;
    double m_altitudeMetres;
    int    m_lights[8];
    double m_intTemp;
    double m_extTemp;
    int    m_camAngle;
    double m_thrusterRPM[6];
    int    m_thrusterFault[6];
    VLBVAuxiliaryRecord m_auxRec;
    char   m_DataChanged;           // Bitset (Heading changed = bit0, Pitch & Roll changed = bit1,
                                    //         Depth/Alt changed = bit2, Lights changed = bit3,
                                    //         Temp changed = bit4, Camera changed = bit5,
                                    //         Thrusters changed = bit6, Aux changed = bit7
};

}
