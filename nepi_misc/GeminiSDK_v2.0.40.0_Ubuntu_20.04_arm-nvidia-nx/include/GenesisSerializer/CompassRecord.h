#pragma once

#include "types.h"
#include <string.h>

namespace GLF
{

/*
** Compass states sent from the console
** maximum of 32 states from 0 to 31
*/
typedef enum {
    COMPASS_STATE_NONE              = (0 <<  0),// No state
    HEADING_VALID                   = (1 <<  0),// Bit 0
    PITCH_VALID                     = (1 <<  1),// Bit 1
    ROLL_VALID                      = (1 <<  2),// Bit 2
    HEAVE_VALID                     = (1 <<  3),// Bit 3
    AHRS_CONFIG_ACK                 = (1 <<  4),// Bit 4
    AHRS_MEASUREMENT_ACK            = (1 <<  5),// Bit 5
    AHRS_MFM_SUCCESS                = (1 <<  6),// Bit 6
    AHRS_CALIBRATION_MODE           = (1 <<  7),// Bit 7  (sending calibration data)
    AHRS_CALIBRATION_RESULT_OK      = (1 <<  8),// Bit 8  (data is ok to write on MT device)
    AHRS_DEVICE_NOT_FOUND           = (1 <<  9),// Bit 9  (Failed to find device ID)
    AHRS_FAILED_TO_START_LOGGING    = (1 << 10),// Bit 10 (Failed to start logging calibration data)
    AHRS_FAILED_TO_PROCESS_DATA     = (1 << 11),// Bit 11 (Failed to process calibration data)
    AHRS_FAILED_TO_WRITE_RESULT     = (1 << 12),// Bit 12 (Failed to write result on MT device)
    AHRS_CALIBRATION_STOPPED        = (1 << 13),// Bit 13 (User requested to stop calibration)
    AHRS_ACCEL_CLIPPED              = (1 << 14),// Bit 14 (AHRS Acceleration data clipped)
    AHRS_GYRO_CLIPPED               = (1 << 15),// Bit 15 (AHRS Gyrp data clipped)
    AHRS_MAG_CLIPPED                = (1 << 16),// Bit 16 (AHRS Magnetic sensor data clipped)
    AHRS_FILTER_VALID               = (1 << 17),// Bit 17 (AHRS Filter is Valid)
    AHRS_HEADING_ZEROED             = (1 << 18),// Bit 18 (AHRS Heading Zero correction is stored in device)
    AHRS_INCLINATION_ZEROED         = (1 << 19),// Bit 19 (AHRS Inclination Zero correction is stored in device)
    AHRS_RESTOREFACTORY_ACK         = (1 << 20),// Bit 20 (AHRS Restore Factory acknowledgement for GUI awareness)
    AHRS_SETMFMRESULTS_ACK          = (1 << 21),// Bit 21 (AHRS Set MFM Results Ack - i.e. cmd used to clear calibration params to factory default
    DMD_AT_VERTICAL_MOUNT_CW        = (1 << 22),// Bit 22 (Used in Minified Mode, is set when the AccelX is >8.5 (=60deg roll) and used to auto-rotate Gemini display
    AHRS_DATA                       = (1 << 23),// Bit 23 Mark the record as being from an AHRS sensor
    AHRS_MICRON_GEMINI              = (1 << 24),// Bit 24 Mark the record as being AHRS data from a Micron Gemnini
	DMD_AT_VERTICAL_MOUNT_CCW       = (1 << 25),// Bit 25 (Used in Minified Mode, is set when the AccelX is <8.5 (=-60deg roll) and used to auto-rotate Gemini display   
    AHRS_MICRO                      = (1 << 26),// Bit 26 Mark the record as being AHRS data from a MicroAHRS
    AHRS_MICRO_MAG_OK               = (1 << 27),// Bit 27 Set if correct Mag Device ID is read back
    AHRS_MICRO_GYRO_OK              = (1 << 28),// Bit 28 Set if correct Gyro Device ID is read back
    AHRS_MICRO_ACCEL_OK             = (1 << 29),// Bit 29 Set if correct Accel Device ID is read back
    AHRS_MICRO_CLOCK_MODE           = (1 << 30),// Bit 30 Set if clock mode is correct (should be running on Ext clock / PLL rather than FRC)
    AHRS_MICRO_WATCHDOG             = (1 << 31),// Bit 31 Set if Watchdog has not been triggered
    //AHRS_MICRO_DEADMAN              = (1 << 32),// Bit 32 Set if Deadman has not been triggered
}CompassStates;

/*!
 * \class CompassDataRecord
 * \brief A class containing all the information related to Compass
  */
struct CompassDataRecord {
    CompassDataRecord()
    {
        Reset();
    }

    void Reset()
    {
        m_heading   = 0.0;
        m_roll      = 0.0;
        m_pitch     = 0.0;
        m_heave     = 0.0;
        m_x         = 0.0;
        m_y         = 0.0;
        m_z         = 0.0;
        m_temp      = 0.0;
        m_variation = 0.0;
        m_deviation = 0.0;
        m_hdgIsTrue = false;
        m_spare[0]  = 0;
        m_spare[1]  = 0;
        m_spare[2]  = 0;
        m_cmpValid  = COMPASS_STATE_NONE;
        m_time      = 0;
        memset(m_sourceName, '\0', sizeof(m_sourceName));
    }

    double m_heading;               // Degrees
    double m_roll;                  // Degrees
    double m_pitch;                 // Degrees
    double m_heave;                 // m/s?
    double m_x;                     // used in Nav Att Sen
    double m_y;                     //
    double m_z;                     //
    double m_temp;                  //
    double m_variation;             // Magnetic Variation (Degrees)
    double m_deviation;             // Magnetic Deviation (Degrees)
    bool   m_hdgIsTrue;             // Is referenced to True North, else is Magnetic
    char   m_spare[3];              // for alignment framing
    tdi::UInt32   m_cmpValid;       // CompassStates
    char m_sourceName[16];          // Name label for the Compass
    tdi::UInt64  m_time;            // milliseconds into day
};

/*!
 * \class AHRSRawDataRecord
 * \brief A class containing all the raw Magnetic, Gyro and Accelerometer data for an AHRS sensor */
struct AHRSRawDataRecord {
    AHRSRawDataRecord()
    {
        Reset();
    }

    void Reset()
    {
        m_gyroX = 0.0;
        m_gyroY = 0.0;
        m_gyroZ = 0.0;
        m_accelX = 0.0;
        m_accelY = 0.0;
        m_accelZ = 0.0;
        m_magX = 0.0;
        m_magY = 0.0;
        m_magZ = 0.0;
        m_aZeroedCorrections[0] = 0.0; //heading
        m_aZeroedCorrections[1] = 0.0; //pitch
        m_aZeroedCorrections[2] = 0.0; //roll
    }

    float m_gyroX;     // rad/s
    float m_gyroY;     // rad/s
    float m_gyroZ;     // rad/s
    float m_accelX;    // m/s2
    float m_accelY;    // m/s2
    float m_accelZ;    // m/s2
    float m_magX;      // normal
    float m_magY;      // normal
    float m_magZ;      // normal
    double m_aZeroedCorrections[3]; // v4utils::Euler m_zeroedCorrections;
};

}
