#pragma once

#include "console/ciheader.h"
#include "GlfLoggerGlobalTypes.h"

namespace GLF
{
/*!
 * \class CompressDataRecord
 * \brief A base class containing all the information related to number of beams,
 *        number of range lines, range compression used for this ping and ping data
 */
class AFX_EXT_GLF_INTERFACE CompressDataRecord {
public:
    CompressDataRecord();     //!< \brief Constructor
    ~CompressDataRecord();     //!< \brief Destructor
    CompressDataRecord& operator =( const CompressDataRecord& compDataRecord );
    CompressDataRecord( const CompressDataRecord& compDataRecord );

    UInt16 m_usImageVersion;    //!< Version number, in case if there is a change in the strcuture
    UInt32 m_compressionType;   //!< Type of cpmression used
    std::vector<UInt8>* m_compBinaryData;//!< Compressed binary data
};

/*!
 * \class GLogCompressRecord
 * \brief This class encapsulate compressed data, this can be any binary data.
 * Currently, this class has been used for recording video data
 */
struct AFX_EXT_GLF_INTERFACE GLogCompressRecord {
    console::PublicMessageHeader    m_header;     // common interface header
    CompressDataRecord              m_compRec;

    GLogCompressRecord& operator =( const GLogCompressRecord& compDataRecord )
    {
        m_header.m_ciHeader = compDataRecord.m_header.m_ciHeader;
        m_compRec           = compDataRecord.m_compRec;
        return *this;
    }
};

}
