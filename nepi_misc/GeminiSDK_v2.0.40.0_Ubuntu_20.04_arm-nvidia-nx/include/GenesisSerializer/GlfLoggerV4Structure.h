#pragma once

#include "console/ciheader.h"
#include "GlfLoggerGlobalTypes.h"

namespace GLF
{

/*!
 * \class V4GenericDataRecord
 * \brief A base class containing all the information related to number of beams,
 *        number of range lines, range compression used for this ping and ping data
 */
class AFX_EXT_GLF_INTERFACE V4GenericDataRecord {
public:
    V4GenericDataRecord();     //!< \brief Constructor
    ~V4GenericDataRecord();     //!< \brief Destructor
    V4GenericDataRecord& operator =( const V4GenericDataRecord& v4GenericData );
    V4GenericDataRecord( const V4GenericDataRecord& v4GenericData );
    UInt16 m_usImageVersion;    //!< Version number, in case if there is a change in the strcuture
    UInt16 m_usLength;          //!< length of data that follows this header
    UInt8  m_ucMsgType;         //!< value maps to GenericTypeRecord
                                //!< Note: in V4 Reply Message record, is followed by len bytes of data
    std::vector<UInt8>*m_vecData;
};


/*!
 * \class GLogV4ReplyMessage
 * \brief This class encapsulate V4Messages.
 */
struct AFX_EXT_GLF_INTERFACE GLogV4ReplyMessage {
    console::PublicMessageHeader    m_header;       // common interface header
    V4GenericDataRecord             m_v4GenericRec; // V4GenericDataRecord
    UInt32                          m_uiFrame;      // Not serialized : Only used in the playback mode.

    GLogV4ReplyMessage& operator =( const GLogV4ReplyMessage& v4Message )
    {
        m_header.m_ciHeader = v4Message.m_header.m_ciHeader;
        m_v4GenericRec      = v4Message.m_v4GenericRec;
        m_uiFrame           = v4Message.m_uiFrame; // Not serialized
        return *this;
    }
};

}
