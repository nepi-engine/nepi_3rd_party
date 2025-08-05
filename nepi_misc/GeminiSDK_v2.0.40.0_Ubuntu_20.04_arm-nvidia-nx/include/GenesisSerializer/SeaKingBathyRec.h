#pragma once

#include "types.h"

namespace GLF
{

//for SeaKing Bathy, processed data sent to GUI
struct SeaKingBathyRec {
    SeaKingBathyRec()
    : m_node(40)
    , m_dqSN(0)
    , m_ctSN(0)
    , m_depthInMeters(0.0)
    , m_altitudeInMeters(0.0)
    , m_salinity(0.0)
    , m_conductivity(0.0)
    , m_condTemperature(0.0)
    , m_vosLocal(0.0)
    , m_vosMean(0.0)
    , m_densityLocal(0.0)
    , m_densityMean(0.0)
    , m_seaPressureBar(0)
    , m_dqPressure(0.0)
    , m_dqTemperature(0.0)
    , m_inTemperature(0.0)
    , m_usedBarometer(1000.0)
    , m_depthValid(false)
    , m_altimeterValid(false)
    , m_dqValid(false)
    , m_ctValid(false)
    , m_inTempValid(false)
    , m_salinityValid(false)
    , m_densityValid(false)
    , m_velocityOSValid(false)
    {
    }

    tdi::UInt8      m_node;
    tdi::UInt32     m_dqSN;
    tdi::UInt32     m_ctSN;

    double          m_depthInMeters;
    double          m_altitudeInMeters;

    double          m_salinity;
    double          m_conductivity;
    double          m_condTemperature;    //on CT sensor

    double          m_vosLocal;
    double          m_vosMean;
    double          m_densityLocal;
    double          m_densityMean;

    double          m_seaPressureBar;

    double          m_dqPressure;
    double          m_dqTemperature;      //on Diqiquartz sensor
    double          m_inTemperature;      //internal temp sensor (fitted to SK701)

    double          m_usedBarometer;

    bool            m_depthValid;
    bool            m_altimeterValid;
    bool            m_dqValid;
    bool            m_ctValid;
    bool            m_inTempValid;
    bool            m_salinityValid;
    bool            m_densityValid;
    bool            m_velocityOSValid;
    tdi::UInt64     m_time;               //milliseconds into day (from Bathy RTC)
};

#pragma pack(push, 1)

struct BathyData {
    unsigned short msglen;
    unsigned char hdType;     // hSTB = 07, value maps to EHeadTypes

    unsigned char status;
    //HdStat status;

    unsigned int dqSN;

    //chans = DQ_Temp and DQ_Press
    unsigned int aDQ_F[2];    // DQ_LCARR in Seanet
    unsigned int aDQ_N[2];    //
    unsigned int ctSN;
    unsigned short aCT_AD[4]; // CQ_CARR in Seanet

    unsigned int vosN;
    unsigned int vo_N;

    unsigned int echo;        // 200ns Units

    short in_Temp;

    float density;            // obsolete in SeaNet
    unsigned int atmosPress;  // obsolete in SeaNet

    int altOffset;            // obsolete in SeaNet
    short dqOffset;
    short dqZero;

    unsigned int time;        // ms FROM 00:00:00
};

struct V7BathyData {
    unsigned short msglen;

    unsigned char hdType;       // hDIGBAT = 26 (= 1AH), value maps to EHeadTypes
    unsigned char status;       // maps to HeadV3Defs.statset

    unsigned int dqSN;
    float dq_TEMP;              // DQ Temp in deg.C
    float dq_PRESS;             // DQ Pressure in dbar

    unsigned int ctSN;
    float ct_COND;              // CT Conductivity in mS/cm
    float ct_TEMP;              // CT Temp in deg.C

    float locVOS;               // Local Velocity Of Sound calc (m/s)   - BatV7 only
    float depth;                // Depth in metres                      - BatV7 only
    float salinity;             // Salinity in ppt(psu)                 - BatV7 only

    unsigned int echo;          // Altimeter Echo in 200ns Units

    short in_TEMP;              // Internal Temp in 100ths of a deg.C

    float density;              // Local Density in Seanet
    unsigned int atmosPress;    // Manual Baro Press in Seanet

    int altOffset;              // Alt Vertical Offset Seanet
    short dqOffset;             // DQ Vertical Offset in Seanet
    short dqZero;               // DQ Zero in Seanet

    unsigned char datatype;     // 1-byte bitset reporting datatypes calculated by the head

    unsigned int time;          // ms FROM 00:00:00
};

#pragma pack(pop)

}
