#pragma once

#include "types.h"

namespace GLF
{

//for Pressure/Depth sensors, processed data sent to GUI
struct DepthDataRecord {
    DepthDataRecord()
    : m_node(227)
    {
        Reset();
    }

    void Reset()
    {
        m_seaPressureBar = 0.0;
        m_depthInMetres = 0.0;
        m_altitudeInMetres = 0.0;
        m_salinity = 0.0;
        m_conductivity = 0.0;
        m_temperature = 0.0;
        m_density = 0.0;
        m_depthValid = false;
        m_altimeterValid = false;
        m_conductivityValid = false;
        m_temperatureValid = false;
        m_densityValid = false;
        memset(m_sourceName, '\0', sizeof(m_sourceName));
        m_time = 0;
    }

    tdi::UInt8      m_node;

    double          m_seaPressureBar;
    double          m_depthInMetres;
    double          m_altitudeInMetres;

    double          m_salinity;
    double          m_conductivity;
    double          m_temperature;
    double          m_density;

    bool            m_depthValid;
    bool            m_altimeterValid;
    bool            m_conductivityValid;
    bool            m_temperatureValid;
    bool            m_densityValid;

    char            m_sourceName[16];       // Name label for the Sensor
    tdi::UInt64     m_time;                 // milliseconds into day Seconds from Jan 1, 1970 (Unix)
};

}
