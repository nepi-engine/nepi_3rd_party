#pragma once

#include "console/ciheader.h"
#include "GlfLoggerGlobalTypes.h"

namespace GLF
{

/*!
 * \class GImage
 * \brief A base class containing all the information related to number of beams,
 *        number of range lines, range compression used for this ping and ping data
 */
class AFX_EXT_GLF_INTERFACE GImage {
public:
    GImage();     //!< \brief Constructor
    ~GImage();     //!< \brief Desstructor
    GImage& operator =( const GImage& gImage );
    GImage( const GImage& gImage );

    UInt32 CopyTo ( UInt8*buffer ) const;
    UInt32 CopyFrom ( const UInt8*buffer );

    UInt32 CompressData( UInt8* outData ) const;
    void CompressData( const UInt16 compressionType, std::vector<UInt8>& inData, std::vector<UInt8>& outData ) const;
    void UncompressData( const UInt16 compressionType, std::vector<UInt8>& inData, std::vector<UInt8>& outData ) const;
    void UncompressData( const UInt8* compressData, UInt32 inSize );
    // Flip data
    void FlipData();

    enum {
        ZLIB_COMPRESSION,   // zLib compression and used by default
        NO_COMPRESSION,     // Data will be archived without any compression applied
        H264_COMPRESSION,   // User will compress data in H264 format before passing to the library
                            // Library will archive data without any compression
    }ECompressionType;

    UInt16 m_usImageVersion;    //!< Version number, in case if there is a change in the strcuture
    UInt16 m_usRangeCompUsed;   //!< Range compression used by the sonar
    UInt16 m_usCompressionType; //!< The compression types used in archiving log data ( See ECompressionType )
    UInt32 m_uiStartBearing;
    UInt32 m_uiEndBearing;
    UInt32 m_uiStartRange;
    UInt32 m_uiEndRange;
    std::vector<UInt8>* m_vecData;
};

/*!
 * \class GMainImage
 * \brief Inherited from GImage contains bearing table, sonar frequency and user selected configuration e.g. chirp etc..
 */
class AFX_EXT_GLF_INTERFACE GMainImage : public GImage{
public:
    GMainImage();     //!< \brief Constructor
    ~GMainImage();     //!< \brief Destructor
    GMainImage& operator =( const GMainImage& mImage );
    GMainImage( const GMainImage& mImage );
    size_t size() const;

    UInt32 CopyTo ( UInt8*buffer ) const;
    UInt32 CopyFrom ( const UInt8*buffer );

    /*!
     * \enum  EGeminiType
     * \brief Different types of Gemini..
     */
    enum EGeminiType
    {
        eGeminiMk1Imager            = 0,    //!< Gemini Mk1 imager (spreading)
        eGeminiMk1Profiler          = 1,    //!< Gemini Mk1 profiler (software gain)
        eGeminiMk2Imager            = 2     //!< Gemini Mk2 (BMG? rescale)
    };

    /*!
     * \enum  ESonarType
     * \brief Gives further information on the exact type of sonar
     * Imager and Profiler are main sonar types. This gives information on subtypes,
     * 720ik, 720im, 720is etc.
     */
    enum ESonarType
    {
        eSonarTypeNone             = 0,    //!< No further information on the type of sonar
        eGemini720is               = 1,    //!< Gemini 720is
        eGemini720ik               = 2,    //!< Gemini 720ik
        eGemini720im               = 3,    //!< Gemini 720im, Micron Gemini
        eGemini1200ik              = 4,    //!< Gemini 1200ik
        eGemini720ik360Sector1     = 5,    //!< Gemini 720ik 360 degrees sector 1
        eGemini720ik360Sector2     = 6,    //!< Gemini 720ik 360 degrees sector 2
        eGemini720ik360Sector3     = 7,    //!< Gemini 720ik 360 degrees sector 3
        eGemini1200nbik            = 8,    //!< Gemini 1200ik NBI
        eGemini1200id              = 9,    //!< Gemini 1200id
        eMicronGemini1200d         = 10    //!< Micron Gemini 1200d
    };

    std::vector<double>* m_vecBearingTable;      //!< bearing table pointer
    UInt32              m_uiStateFlags;         //!< State Flags
                                                //!< Bit 15-13: Sonar orientation 0: Up, 1: down
                                                //!< Bit 7: Vert Beam Mode 0: Wide, 1: Narrow
    UInt32              m_uiModulationFrequency;//!< The modulation frequency
    float               m_fBeamFormAperture;    //!< Calculate the number of beams to provide the correct aperture
    double              m_dbTxTime;             //!< Time of transmit
    UInt16              m_usPingFlags;          //!< Ping Flags
                                                //!< Bit 0:  ( 1: HF, 0 : LF )
                                                //!< Bit 15: ( 1: Manual, 0 : sonar )
    float               m_fSosAtXd;             //!< The SOS at the transducer
    Int16               m_sPercentGain;         //!< The Percentage gain used to record
    bool                m_fChirp;               //!< Chirp enabled/disabled
    UInt8               m_ucSonarType;          //!< See EGeminiType
    UInt8               m_ucPlatform;           //!< See ESonarType
};

/*!
 * \class GZoomImage
 * \brief This class will be used for acoustic zoom and will use bearing table from the main image.
 *  Acoustic zoom image will only be available if main image is available.
 */
class AFX_EXT_GLF_INTERFACE GZoomImage : public GImage {
public:
    GZoomImage ();     //!< \brief Constructor
    ~GZoomImage ();     //!< \brief Destructor
    GZoomImage& operator =( const GZoomImage& azImage );
    GZoomImage( const GZoomImage& azImage );
    size_t size() const;

    UInt32 CopyTo ( UInt8*buffer ) const;
    UInt32 CopyFrom ( const UInt8*buffer );

    bool        m_fActive;           // Active acoustic per sonar
    UInt16      m_usId;              // Acoustic zoom ID
    double      m_dMagnitude;        // Magnification Value
};


/*!
 * \class GLogTargetImage
 * \brief This class encapsulate both main image and acoustic zom image, if enabled (supported).
 */
struct AFX_EXT_GLF_INTERFACE GLogTargetImage {
    console::PublicMessageHeader    m_header;     // common interface header
    GMainImage                      m_mainImage;
    GZoomImage                      m_aczImage;

    GLogTargetImage& operator =( const GLogTargetImage& gLogTgtImage )
    {
        m_header.m_ciHeader = gLogTgtImage.m_header.m_ciHeader;
        m_mainImage         = gLogTgtImage.m_mainImage;
        m_aczImage          = gLogTgtImage.m_aczImage;
        return *this;
    }

    // Flip both main sonar image and Acoustic zoom data
    void FlipData()
    {
        m_mainImage.FlipData();
        m_aczImage.FlipData();
    }

    size_t size() const
    {
        size_t size = 0;
        size +=  sizeof(m_header)    +               // Common interface header
                 m_mainImage.size()  +               // Size of sonar data + Bearing table size
                 m_aczImage.size();                  // Acoustic zoom sonar data
        return size;
    }

    /*  Method:     CopyTo
     *  Parameters: buffer - Heap location to copy into
     *  Returns:    Total Bytes Copied
     *  Notes:
     *  3/12/2024
     *  m_ciHeader = 21 bytes
     *  m_mainImage = Variable, Copies the following:
     *
     *  buffer,m_vecBearingTable, m_uiStateFlags, m_uiModulationFrequency,
     *  m_fBeamFormAperture, m_dbTxTime, m_usPingFlags, m_fSosAtXd,
     *  m_sPercentGain, m_fChirp, m_ucSonarType, m_ucPlatform
     *
     *  m_aczImage = Variable, Copies the following:
     *
     *  m_fActive, m_usId, m_dMagnitude,
     *
     *  And, if m_fActive is true:
     *
     *  m_usImageVersion, m_usRangeCompUsed, m_usCompressionType, m_uiStartBearing,
     *  m_uiEndBearing, m_uiStartRange, m_uiEndRange, compLengthField,
     *  CompressData
     */
    UInt32 CopyTo ( UInt8*buffer ) const
    {
        UInt32 index = 0;
        memcpy( &buffer[index], &m_header.m_ciHeader, sizeof(m_header.m_ciHeader) );
        index = sizeof(m_header.m_ciHeader);

        index += m_mainImage.CopyTo( &buffer[ index ] );
        index += m_aczImage.CopyTo( &buffer[ index ] );
        return index;
    }

    UInt32 CopyFrom ( const UInt8*buffer )
    {
        UInt32 index = 0;
        memcpy( &m_header.m_ciHeader, &buffer[index], sizeof(m_header.m_ciHeader) );
        index = sizeof(m_header.m_ciHeader);

        index += m_mainImage.CopyFrom( &buffer[ index ] );
        index += m_aczImage.CopyFrom( &buffer[ index ] );
        return index;
    }
};


//Target Playback image data type
struct AFX_EXT_GLF_INTERFACE GenesisPlaybackTargetImage
{
    GLogTargetImage*    m_pLogTgtImage;
    UInt32              m_frame;
    GenesisPlaybackTargetImage()
    : m_pLogTgtImage( NULL )
    , m_frame( 0 )
    {
    }
};

}

