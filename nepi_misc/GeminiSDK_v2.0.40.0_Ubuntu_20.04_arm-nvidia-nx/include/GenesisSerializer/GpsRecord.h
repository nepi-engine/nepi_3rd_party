#pragma once

#include "types.h"

#define MAX_SATELLITES 12

namespace GLF
{

/*!
 * \brief A latitude/longitude position.
 *
 * Contains a latitude/longitude (in degrees) and altitude. Valid values are
 * -90 <= latitude <= 90, and -180 <= longitude < 180. For UTM,
 * -80 <= latitude < 84.
 */
struct LatLongPosition {
    LatLongPosition()
    {
        Reset();
    }
    void Reset()
    {
        m_latDegrees    = 0.0;
        m_longDegrees   = 0.0;
        m_altitude      = 0.0;
    }
    double   m_latDegrees;     //!< latitude in degrees
    double   m_longDegrees;    //!< longitude in degrees
    double   m_altitude;       //!< altitude in metres
};

/*!
 * \brief A generalised easting/northing position, with altitude.
 *
 */
struct EastingNorthingPosition {
    EastingNorthingPosition()
    {
        Reset();
    }
    void Reset()
    {
        m_easting   = 0.0;
        m_northing  = 0.0;
        m_altitude  = 0.0;
    }
    double   m_easting;        //!< Easting
    double   m_northing;       //!< Northing
    double   m_altitude;       //!< Altitude in metres
};


struct SatelliteRecord {
    SatelliteRecord()
    {
        Reset();
    }
    void Reset()
    {
        m_satsInView    = 0;
        m_PDOP          = 0.0;
        m_HDOP          = 0.0;
        m_VDOP          = 0.0;
        m_mode2         = 0;
        for( tdi::Int32 i = 0; i < MAX_SATELLITES; ++i )
        {
            m_id[ i ]   = 0;
            m_elev[ i ] = 0;
            m_azim[ i ] = 0;
            m_snr[ i ]  = 0;
        }
    }
    tdi::Int32  m_satsInView;               //!< total number of satellites in view (from GSV)
    tdi::Int32  m_id[MAX_SATELLITES];       //!< IDs of up to 12 satellites used for fix (from GSV)
    tdi::Int32  m_elev[MAX_SATELLITES];     //!< satellite elevations in degrees (from GSV)
    tdi::Int32  m_azim[MAX_SATELLITES];     //!< satellite azimuth in degrees (ref true north, from GSV)
    tdi::Int32  m_snr[MAX_SATELLITES];      //!< satellite SNR 00-99 (from GSV)
    double      m_PDOP;                     //!< Position Dilution of Precision (from GSA)
    double      m_HDOP;                     //!< Horizontal Dilution of Precision (from GSA)
    double      m_VDOP;                     //!< Vertical Dilution of Precision (from GSA)
    tdi::Int32  m_mode2;                    //!< Fix Type: 1 = N/A, 2 = 2D, 3 = 3D
};

/*!
 * \class GpsDataRecord recorded in
 * \brief A base class containing all the information related to Lattitude, longitude, easting and northing
 */

struct GpsDataRecord {
    GpsDataRecord()
    {
        Reset();
    }

    void Reset()
    {
        m_cog       = 0.0;
        m_sog       = 0.0;
        m_heading   = 0.0;
        m_time      = 0;
        m_gpsValid  = 0;
        m_enFormat  = 'E';
        memset(m_sourceName, '\0', sizeof(m_sourceName));
    }
    void ResetAll()
    {
        m_llRec.Reset();
        m_enRec.Reset();
        m_satInfo.Reset();
        Reset();
    }

    LatLongPosition         m_llRec;            //!< degs & mins
    EastingNorthingPosition m_enRec;            //!< Easting/Northing (UTM/OSGB)
    double                  m_cog;              //!< Degrees
    double                  m_sog;              //!< Knots
    double                  m_heading;          //!< Degrees
    SatelliteRecord         m_satInfo;          //!< Sat.IDs, HDOP, Fix
    tdi::UInt64             m_time;             //!< Seconds from Jan 1, 1970 (Unix)
    tdi::Int8               m_gpsValid;         //!< bits 6..0 = Time,Hdg,Sog,Cog,Lon,Lat,EN
    char                    m_sourceName[16];   //!< Name label for the GPS (optional - may be brought in on $RATTM string for instance
    char                    m_enFormat;         //!< '' unused if blank, ‘H’ Rel Heading up, ‘N’ Rel North up, ‘E’ Abs UTM
};

}
