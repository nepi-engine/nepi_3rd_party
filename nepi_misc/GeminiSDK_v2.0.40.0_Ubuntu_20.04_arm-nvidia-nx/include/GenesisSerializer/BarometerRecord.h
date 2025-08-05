#pragma once

#include "types.h"

namespace GLF
{
/*!
 * \class BarometerDataRecord
 * \brief A class containing all the information related to Barometer
 */

struct BarometerDataRecord {
    BarometerDataRecord()
    {
        Reset();
    }
    void Reset()
    {
        m_baroPressure=1000.0;
        m_time=0;
    }

    double      m_baroPressure; //mbar
    tdi::UInt64 m_time;         //milliseconds into day
};

}
