#pragma once
#include "types.h"

namespace Processor
{

typedef std::vector<tdi::UInt8> ProcessorDataType;

const int kREMV4_DevInstanceStart = 500;
const int kBAROMETER_DevInstanceStart = 505; //there is 1 barometer input

//Processor type used in the CreateProcessor API
enum EProcessorType{
    PROCESSOR_TYPE_GPS,
    PROCESSOR_TYPE_COMPASS,
    PROCESSOR_TYPE_V4,          // V4 reply message starts with '@',
                                // V4 'HS' reply message starts with '!'
    PROCESSOR_TYPE_ALTIMETER,
    PROCESSOR_TYPE_BAROMETER,
    PROCESSOR_TYPE_VLBV,        // SeaBotix vLBV vehicle controls
    PROCESSOR_TYPE_ASCII,
    PROCESSOR_TYPE_DEPTH,
    PROCESSOR_TYPE_GLOBAL,
    PROCESSOR_TYPE_ALTIMETER_MK2,

    PROCESSOR_TYPE_3DP,

    PROCESSOR_TYPE_NONE,        // Leave it at the end for default initialization
};

//type of data passed into V4Processor
enum EV4ProcessorDataType{
    V4_PROCESSOR_DATA_TYPE_ASCII,
    V4_PROCESSOR_DATA_TYPE_V4,
    V4_PROCESSOR_DATA_TYPE_HS,
    V4_PROCESSOR_DATA_TYPE_COMPASS,
    V4_PROCESSOR_DATA_TYPE_GPS,
    V4_PROCESSOR_DATA_TYPE_AHRS,
    V4_PROCESSOR_DATA_TYPE_V4ALT,
    V4_PROCESSOR_DATA_TYPE_GEMINI_REMV4,
    V4_PROCESSOR_DATA_TYPE_GEMINI_AUX,
    V4_PROCESSOR_DATA_TYPE_GEMINI_INPUT,
    V4_PROCESSOR_DATA_TYPE_BAROMETER,
    V4_PROCESSOR_DATA_TYPE_PRESS_DEPTH
};

} // namespace Processor

