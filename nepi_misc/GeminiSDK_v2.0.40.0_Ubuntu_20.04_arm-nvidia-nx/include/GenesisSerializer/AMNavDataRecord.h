#pragma once

#include "types.h"

namespace GLF
{

#pragma pack(push, 1)

// Double Floating Point Records for Normal Calculations
struct TP3D {                       // 3D Cartesian Position(metres)
    TP3D()
    : m_x(0)
    , m_y(0)
    , m_z(0)
    {}


    // 3 dimensional coordinate E,N,Z = X,Y,Z
    double m_x;
    double m_y;
    double m_z;
};

struct AMNavData {
    AMNavData()
    : m_ping_msecs(0)
    , m_replies(30)
    , m_doax(0)
    , m_doay(0)
    , m_doaz(0)
    , m_rms(0)
    , m_usblAngQual(0)
    , m_usblRngQual(0)
    , m_reliabilityX(0)
    , m_reliabilityY(0)
    , m_reliabilityZ(0)
    , m_reliabilityR(0)
    , m_aAttitude()
    , m_xpdrMode(false)
    , m_unitID(0)
    , m_relPos()
    , m_sigmaX(0)
    , m_sigmaY(0)
    , m_sigmaZ(0)
    , m_sigmaR(0)
    , m_sigmaYX(0)
    , m_fixVOS(0)
    , m_range(100)
    , m_attCorrPos()
    , m_worldPos()
    , m_fixDatime(0)
    {}

    tdi::UInt32 m_ping_msecs;         // millisecs of Day
    tdi::UInt8  m_replies;            // Set is 'Valid Reply' Flags

    float       m_doax;
    float       m_doay;
    float       m_doaz;
    float       m_rms;

    float       m_usblAngQual;        // 1 = Max = Closed Angle Triangle
    float       m_usblRngQual;        // Spread of Range Detects, 4 Rx Xdcrs in Metres

    float       m_reliabilityX;
    float       m_reliabilityY;
    float       m_reliabilityZ;
    float       m_reliabilityR;

    double      m_aAttitude[3];       // Any Attitude Sensor Data (Roll, Pitch, Yaw)
    bool        m_xpdrMode;           // If True = Transponder False = Responder
    tdi::UInt16 m_unitID;             // A Unit Identifier, for Multiple UW Units

    TP3D        m_relPos;             // Relative XYZ Calculated Position

    float       m_sigmaX;
    float       m_sigmaY;
    float       m_sigmaZ;
    float       m_sigmaR;
    float       m_sigmaYX;                  // For Ellipse Dirn Calc

    float       m_fixVOS;
    double      m_range;                   // Metres range for Display

    TP3D        m_attCorrPos;              // RelPos corrected for Pitch, Roll, Yaw
    TP3D        m_worldPos;                // AttCorrPos + World Transducer Coords (in Degrees Lat and Lon)
    double      m_fixDatime;               // When Fix actually Happened (PingDatime as DateTime)
};
#pragma pack(pop)

}
