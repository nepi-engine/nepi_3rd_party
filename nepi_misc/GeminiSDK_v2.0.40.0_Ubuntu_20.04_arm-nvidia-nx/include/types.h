#pragma once

#include <cstdint>
#include "DataTypes.h"

#include <string>
#include <vector>

namespace tdi{

typedef std::uint64_t UInt64;
typedef std::uint32_t UInt32;
typedef std::uint16_t UInt16;
typedef std::uint8_t  UInt8;

typedef std::int64_t  Int64;
typedef std::int32_t  Int32;
typedef std::int16_t  Int16;
typedef std::int8_t   Int8;


typedef std::uint8_t  Byte;
typedef std::int8_t   Char;

typedef UInt32      HAL_STATUS;
typedef void*       HAL_HANDLE;
typedef void*       CONSOLE_HANDLE;
typedef void*       HPROCESSOR;
typedef void*       HFTDI;
typedef void*       COM_HANDLE;

typedef UInt32      ErrorCode;

struct DevicePortInfo
{
    std::string m_comPortName;
    UInt16      m_comPortID;
    bool        m_busy; // true: in Use, false : available
    bool        m_reserved;// true: reserved can not be selected by the user, false : can be used

    inline DevicePortInfo()
    : m_comPortName("")
    , m_comPortID( -1 )
    , m_busy( false )
    , m_reserved( false )
    {
    }

    inline DevicePortInfo( std::string name, UInt16 id, bool busy )
    : m_comPortName(name)
    , m_comPortID( id )
    , m_busy( busy )
    , m_reserved(false)
    {
    }

    inline DevicePortInfo& operator = ( const DevicePortInfo& other )
    {
        if( this != &other )
        {
            m_comPortName     = other.m_comPortName;
            m_comPortID       = other.m_comPortID;
            m_busy            = other.m_busy;
            m_reserved        = other.m_reserved;
        }
        return *this;
    }

    inline bool operator == ( const DevicePortInfo& other )
    {
        return ( ( m_comPortName == other.m_comPortName ) &&
            ( m_comPortID   == other.m_comPortID ) &&
            ( m_busy        == other.m_busy ) && 
            ( m_reserved    == other.m_reserved ) );
    }

    inline DevicePortInfo(const DevicePortInfo& other )
    {
        *this = other;
    }
};

typedef std::vector<DevicePortInfo> CommPortMapping;
typedef CommPortMapping::iterator CommPortMappingItr;

typedef std::vector<std::string>ListDevices;
typedef ListDevices::iterator DevListItr;

typedef std::vector<std::string> VectorString;

}

