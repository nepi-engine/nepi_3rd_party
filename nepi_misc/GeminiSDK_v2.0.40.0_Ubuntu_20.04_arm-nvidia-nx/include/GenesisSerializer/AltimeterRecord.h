#pragma once

#include "types.h"

namespace GLF
{
/*!
 * \class AltimeterDataRecord
 * \brief A class containing all the information related to Compass
 */

//Note: 1473.0 is VOS used in PA Altimeter
struct AltimeterDataRecord
{
    AltimeterDataRecord()
    {
        Reset();
    }

    void Reset()
    {
        m_altitudeMetres=0.0;
        m_vosUsed=0.0;
        m_altValid=false;
        m_slantRangeMetres=0.0;
        m_time=0;
        memset(m_sourceName, '\0', sizeof(m_sourceName));

        for( tdi::Int8 i = 0; i < sizeof(m_id); ++i)
        {
            m_id[ i ] = '\0';
        }
    }

    double      m_altitudeMetres;
    double      m_vosUsed;
    bool        m_altValid;
    double      m_slantRangeMetres; // i.e. if PAPA-AHRS then we have AHRS to then calculate altitude
    tdi::UInt64 m_time;             // milliseconds into day
    tdi::Int8   m_id[16];           //e.g. selected: A..H, 0..7 for addressable, else blanks
    char        m_sourceName[16];   //Name label for the Altimeter
};

struct Mk2RecordAppend
{
    Mk2RecordAppend(): m_inclination(0.0), m_binCount(0),
                       m_startRange_mm(0), m_endRange_mm(100000),
                       asnData(nullptr) {}

    // copy constructor: copy asnData pointer for thread safety
    Mk2RecordAppend(const Mk2RecordAppend& rec)
    {
        m_inclination = rec.m_inclination;
        m_binCount = rec.m_binCount;
        m_startRange_mm = rec.m_startRange_mm;
        m_endRange_mm = rec.m_endRange_mm;
        asnData = new unsigned char[m_binCount];
        if( rec.asnData )
        {            
            memcpy(asnData, rec.asnData, sizeof(unsigned char) * m_binCount);
        }
    }

    ~Mk2RecordAppend()
    {
        if (asnData)
            delete []asnData;
    }

    double          m_inclination;
    unsigned int    m_binCount;
    unsigned int    m_startRange_mm;
    unsigned int    m_endRange_mm;
    unsigned char   *asnData;         // placeholder for data array
};

struct AltimeterDataRecordMk2
{
    AltimeterDataRecordMk2()
    {
        Reset();
    }

    void Reset()
    {
        altRec.m_altitudeMetres=0.0;
        altRec.m_vosUsed=0.0;
        altRec.m_altValid=false;
        altRec.m_slantRangeMetres=0.0;
        altRec.m_time=0;

        memset(altRec.m_sourceName, '\0', sizeof(altRec.m_sourceName));

        for( tdi::Int8 i = 0; i < sizeof(altRec.m_id); ++i)
        {
            altRec.m_id[ i ] = '\0';
        }

        mk2RecAppend.m_inclination = 0.0;
        mk2RecAppend.m_binCount = 0;
        mk2RecAppend.m_startRange_mm = 0;
        mk2RecAppend.m_endRange_mm = 100000;
        mk2RecAppend.asnData = nullptr;
    }

    AltimeterDataRecord altRec;
    Mk2RecordAppend     mk2RecAppend;
};

}
