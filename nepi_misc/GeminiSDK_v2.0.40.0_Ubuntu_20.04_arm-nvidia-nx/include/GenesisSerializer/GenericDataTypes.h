#pragma once

/*
** Supported message types are currently logged
*/

namespace GLF
{

typedef struct {
    // Callback Data Types defined here:
    enum
    {
        GT_RECORD_RAW_ASCII_STRING,
        GT_RECORD_GPS,
        GT_RECORD_COMPASS,
        GT_RECORD_RAW_V4_REPLY,      // SeaKing sonar
        GT_RECORD_PROC_V4_REPLY,     // Micronnav
        GT_RECORD_BATHYPROFILE,
        GT_RECORD_VLBV,              // SeaBotix VLBV vehicle control data
        GT_RECORD_BAROMETER,         // Barometer input for Bathy
        GT_RECORD_RAT_REPLY_RECORD,  // RAT button/dial data
        GT_RECORD_ALTIMETER     = 9, // Don't change this number, required to suport old mapping
        GT_RECORD_DEPTH,             // Generic Pressure/Depth Sensor
        GT_RECORD_BATHY         = 13,// Don't change this number, required to suport old mapping
        GT_RECORD_NAV_DIVER     = 16,// Don't change this number, required to suport old mapping
        GT_RECORD_RAWAHRS,           // AHRS Gyro, Mag, Accel data
        GT_RECORD_ALTIMETER_MK2,
        GT_RECORD_ALTIMETER_CONFIG_MK2,

        GT_RECORD_REMV4_OUTPUT_1 = 106, // Don't change this number
        GT_RECORD_REMV4_OUTPUT_2 = 107, // Don't change this number
        GT_RECORD_REMV4_OUTPUT_3 = 108, // Don't change this number
        GT_RECORD_REMV4_OUTPUT_4 = 109, // Don't change this number
    };
}GenericTypeRecord;

}
