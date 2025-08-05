#pragma once

#include "errorcodes.h"
#include "DataProcessorDef.h"

#include <functional>

using namespace tdi;

#ifdef DATA_PROCESSOR_STATIC_LIB
  #define AFX_EXT_DATA_PROCESSOR_INTERFACE
#else
  #if defined WIN32 || defined __CYGWIN__
    #ifdef EXPORT_DATA_PROCESSOR_INTERFACE
      #ifdef __GNUC__
        #define AFX_EXT_DATA_PROCESSOR_INTERFACE __attribute__((dllexport))
      #else
        #define AFX_EXT_DATA_PROCESSOR_INTERFACE extern "C" __declspec(dllexport)
      #endif
    #else
      #ifdef __GNUC__
        #define AFX_EXT_DATA_PROCESSOR_INTERFACE __attribute__((dllimport))
      #else
        #define AFX_EXT_DATA_PROCESSOR_INTERFACE extern "C" __declspec(dllimport)
      #endif
    #endif
  #else
    #if __GNUC__ >= 4
      #define AFX_EXT_DATA_PROCESSOR_INTERFACE __attribute__((visibility("default")))
    #else
      #define AFX_EXT_DATA_PROCESSOR_INTERFACE
    #endif
  #endif
#endif

#define DATA_PROCESSOR_OK               ( DATA_PROCESSOR_ERROR_CODE | STATUS_OK )
#define DATA_PROCESSOR_INVALID_TYPE     ( DATA_PROCESSOR_ERROR_CODE | 1 )
#define DATA_PROCESSOR_INVALID_HANDLE   ( DATA_PROCESSOR_ERROR_CODE | 2 )
#define DATA_PROCESSOR_INVALID_ARGUMENT ( DATA_PROCESSOR_ERROR_CODE | 3 )
#define DATA_PROCESSOR_FAILED_TO_CREATE ( DATA_PROCESSOR_ERROR_CODE | 4 )
#define DATA_PROCESSOR_INVALID_PROPERTY ( DATA_PROCESSOR_ERROR_CODE | 5 )


namespace Processor
{

// Callback Data Types defined here:
// These callbacks are only for internal to the console and should be used within the GUI or the SDK
// See ConsoleGenericDataTypes.h for external messages outside the console

enum EProcessorDecodedType{
    RAW_ASCII_STRING,
    GPS_FIX_RECORD,
    COMPASS_DATA_RECORD,
    RAW_V4_REPLY_RECORD,
    PROC_V4_REPLY_RECORD,
    DEVICE_V4_COMMAND,
    RAW_HS_REPLY_RECORD,
    NAV_DIAGNOSTIC_RECORD,
    NAV_PING_ID,
    ALTIMETER_DATA_RECORD,
    STATUS_V4_REPLY_RECORD,
    DEVICE_V4_COMMAND_NAV_E,        //to route Nav Attitude commands out on Port E
    MSG_V4_FIRMWARE_UPGRADE_STATUS, //Status info for GUI during programming
    BATHY_DATA_RECORD,
    NAV_STATUS_RECORD,
    DEVICE_V4_COMMAND_SONARRESP,
    NAV_DIVER_RECORD,
    REQUEST_DEVICE_NODE,
    DEVICE_NODE_CHANGE,
    DEVICE_AHRS_COMMAND,
    REMV4_INPUT,
    AHRS_MESSAGE,
    BATHY_PROFILE_RECORD,
    BAROMETER_DATA_RECORD,
    VLBV_DATA_RECORD,
    DMD_HAND_CONTROLLER,
    RAT_REPLY_RECORD,
    DEPTH_DATA_RECORD,
    AHRS_RAWDATA_RECORD,
    CURSOR_COORD_OUTPUT,
    ALTIMETER_DATA_RECORD_MK2,      //Extended for MK2 to include bin data and other fields
    ALTIMETER_CONFIG_MK2,
    DEVICE_ALTMK2_COMMAND,

    //IMPORTANT : must always be at bottom, add new above
    REMV4_OUTPUT_1 = 501,           //i.e. each V4 Node can output on 4 x slots
    REMV4_OUTPUT_2 = 502,
    REMV4_OUTPUT_3 = 503,
    REMV4_OUTPUT_4 = 504,    
    REMV4_BAROMETER_INPUT = 505,
    REMV4_OUTPUT_UDP = 506
};


enum EProcessorProperty{
    NODE_INFO,
    GET_PROCESSOR_TYPE,
    GET_PROCESSOR_NAV_INTERFACE,
    GET_PROCESSOR_BATHY_INTERFACE
};

typedef std::function<void ( HPROCESSOR pHProcessor, EProcessorDecodedType type, void* data, size_t size ) > ProcessorCallback;

AFX_EXT_DATA_PROCESSOR_INTERFACE ErrorCode CreateProcessor( const EProcessorType processorType, HPROCESSOR& pHProcessor, ProcessorCallback processorCallback );

AFX_EXT_DATA_PROCESSOR_INTERFACE ErrorCode GetNavProcessor( const EProcessorType processorType, HPROCESSOR& pHProcessor );

AFX_EXT_DATA_PROCESSOR_INTERFACE ErrorCode GetBathyProcessor( const EProcessorType processorType, HPROCESSOR& pHProcessor );

AFX_EXT_DATA_PROCESSOR_INTERFACE ErrorCode ConfigProcessor( HPROCESSOR pHProcessor, UInt32 size, const void* );

AFX_EXT_DATA_PROCESSOR_INTERFACE ErrorCode SetProperty(HPROCESSOR pHProcessor, UInt32 propertyId, UInt32 size, void const * const value);

AFX_EXT_DATA_PROCESSOR_INTERFACE ErrorCode GetProperty(HPROCESSOR pHProcessor, UInt32 propertyId, UInt32 size, void * value);

AFX_EXT_DATA_PROCESSOR_INTERFACE ErrorCode ProcessString( HPROCESSOR pHProcessor, const UInt8* inputData, UInt32 dataLength, EV4ProcessorDataType eType );

AFX_EXT_DATA_PROCESSOR_INTERFACE ErrorCode PollProcessor( HPROCESSOR pHProcessor );

AFX_EXT_DATA_PROCESSOR_INTERFACE ErrorCode DestroyProcessor( HPROCESSOR pHProcessor );

/////////////////////////////////////////////////////////////
} // namespace Processor

