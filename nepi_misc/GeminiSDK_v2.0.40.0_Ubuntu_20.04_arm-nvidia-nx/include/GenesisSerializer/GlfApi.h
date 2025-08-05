#pragma once

#include "GlfLoggerGlobalTypes.h"

#ifdef _GENESIS_BUILD_
#include "errorcodes.h"
#else
#define GLF_ERROR_CODE              ( 0x01000000L )
#endif
#include "GlfLoggerStatus.h"
#include "GlfLoggerGeminiStructure.h"
#include "GlfLoggerV4Structure.h"
#include "GlfLoggerVideoStructure.h"
#include "GeminiStatusRecord.h"
#include "3dprecord.h"


typedef void*       GLF_HANDLE;

#define GLF_OK                          ( 0 )
#define GLF_LOGGER_NOT_CREATED          ( GLF_ERROR_CODE | 1 )
#define GLF_INVALID_HANDLE              ( GLF_ERROR_CODE | 2 )
#define GLF_DATA_NOT_AVAILABLE          ( GLF_ERROR_CODE | 3 )
#define GLF_FAILED_TO_WRITE             ( GLF_ERROR_CODE | 4 )
#define GLF_INVALID_PARAM               ( GLF_ERROR_CODE | 5 )
#define GLF_BUSY_PROCESSING_INDEX_TABLE ( GLF_ERROR_CODE | 6 )

namespace GLF
{

/****************************************************************************/
/*                             Genesis logger writer                        */
/****************************************************************************/
/*!
 * \brief CreateLogFileWriter creates a logger if not already created, otherwise close the exiting and create a new.
 * \return errorcode
 */
AFX_EXT_GLF_INTERFACE ErrorCode CreateLogFileWriter(
                                                GLF_HANDLE*         hWriter,            /*! [out] Return the log file handle */
                                                const char*         relativePath,       /*! [in] Base path to create files */
                                                SigOutputFileInfo   sigOutputFileInfo,  /*! [in] Slot for when output file info sent */
                                                SigInformationString sigInfoString,     /*! [in] Slot for error message sent */
                                                const UInt32        maxFileSizeMB,      /*! [in] Maximum log file size */
                                                const char**        fileName            /*! [out] returning log file name */
                                                );

/*!
 * \brief LogGeminiData Write Gemini Target imaga data
 * \return errorcode
 */
AFX_EXT_GLF_INTERFACE ErrorCode LogGeminiData(
                                        GLF_HANDLE              hWriter,        /*! [in] handle to the file writer  */
                                        const GLogTargetImage&  gTargetImage    /*! [in] gemini target image        */
                                        );

/*!
 * \brief LogGeminiStatus Write Gemini status record
 * \return errorcode
 */
AFX_EXT_GLF_INTERFACE ErrorCode LogGeminiStatus(
                                        GLF_HANDLE                      hWriter,        /*! [in] handle to the file writer  */
                                        const GeminiSonarStatusMessage& gStatusRecord   /*! [in] gemini status record       */
                                        );

/*!
 * \brief LogV4Data log V4 data.
 * \return errorcode
 */
AFX_EXT_GLF_INTERFACE ErrorCode LogV4Data(
                                    GLF_HANDLE                  hWriter,        /*! [in] handle to the file writer  */
                                    const GLogV4ReplyMessage&   gV4Msg          /*! [in] V4 log data  */
                                    );

/*!
 * \brief LogVideoData log compressed video data.
 * \return errorcode
 */
AFX_EXT_GLF_INTERFACE ErrorCode LogVideoData(
                                    GLF_HANDLE                  hWriter,        /*! [in] handle to the file writer  */
                                    const GLogCompressRecord&   gLCompressRec   /*! [in] log compressed video data  */
                                    );

/*!
 * \brief Log3DPData log 3DP point data
 * \return errorcode
 */
AFX_EXT_GLF_INTERFACE ErrorCode Log3DPData(
                                    GLF_HANDLE                  hWriter,        /*! [in] handle to the file writer  */
                                    const PointDataRecord&          gPointData      /*! [in] log 3DP point data  */
                                    );

/*!
 * \brief GetLogFilename copies the current active recoded file name in the user specified buffer
 * \return errorcode
 */
AFX_EXT_GLF_INTERFACE ErrorCode GetLogFilename(
                                    GLF_HANDLE&     hWriter,    /*! [in] handle to the file writer  */
                                    char*           buffer,     /*! [in] input buffer for copying the current log file name */
                                    const UInt32    bufferSize  /*! [in] input buffer size of buffer argument */
                                    );

/****************************************************************************/
/*                                      Write API END                       */
/****************************************************************************/


/****************************************************************************/
/*                            Genesis logger reader                         */
/* NOTE: This API takes both .dat and .glf files as input                   */
/****************************************************************************/
AFX_EXT_GLF_INTERFACE ErrorCode CreateLogFileReader(
                                        GLF_HANDLE*                         hReader,            /*! handle to the file reader */
                                        const char*                         inputFilenames[],   /*! Full paths of input files to read */
                                        const UInt32                        noOfInputFiles,     /*! Number of input filenames         */
                                        SigActiveFileIndex                  sigFileIndex,       /*! Slot for when current file index changes */
                                        SigFileListInfo                     sigFileListInfo,    /*! Slot to update file information during processing */
                                        SigInformationString                sigInfoString,      /*! Slot to pass back information (mostly errors) */
                                        bool                                asynchronous        /*! true: immediately return and generate index database  in a separate thread*/
                                                                                                /*  false: will only once the database index in generated  */
                                        );

/*!
 * \brief GetFileStartPosition looks for the first target image record associated with a file.
 * \return index of target image record.
 */
AFX_EXT_GLF_INTERFACE ErrorCode GetFileStartPosition(
                                    GLF_HANDLE      hReader,     /*! [in] Handle reader */
                                    const UInt32    uiFileIndex, /*! [in] Index of file in list */
                                    UInt32&         filePosition /*! [out] File position */
                                    );

/*!
 * \brief GetRecord returns a GlfRecord
 * \return errorcode
 */
AFX_EXT_GLF_INTERFACE ErrorCode GetRecord(
                                        GLF_HANDLE      hReader,            /*! [in] Handle reader */
                                        const UInt32    uiRecordInfoPos,    /*! [in] Index of image in file */
                                        GlfRecord&      glfRecord           /*! [out] fill the structure with glf message*/
                                        );


/****************************************************************************/
/*                       Genesis Single File log reader                     */
/*                                                                          */
/* NOTE: This API takes both .dat and .glf files as input                   */
/****************************************************************************/

/*!
 * \brief CreateSingleFileLogReader Creates a logger to access the GLF file serializer
 *        without creating an index table.
 *
 * Note: This mechanism would not allow user to go forward / backward between the records
 *       Only GetNextRecord API can be used with this logger but logger can be closed using
 *       CloseLogFileHandler.
 *
 * \return errorcode
 */
AFX_EXT_GLF_INTERFACE ErrorCode CreateSingleFileLogReader(
                                        GLF_HANDLE*     hReader,            /*! handle to the file reader */
                                        const char*     inputFilename      /*! Full paths of input file to read */
                                        );

/*!
 * \brief GetNextRecord returns a GlfRecord
 *
 * This API can only be used with "CreateSingleFileLogReader" API when accessing the records
 * sequentially in forward direction. Close the logger using "CloseLogFileHandler" API when there's
 * error code returns
 *
 * \return errorcode
 */
AFX_EXT_GLF_INTERFACE ErrorCode GetNextRecord(
                                        GLF_HANDLE      hReader,            /*! [in] Handle reader */
                                        GlfRecord&      glfRecord           /*! [out] fill the structure with glf message*/
                                        );

/****************************************************************************/
/*                                     Reader API END                       */
/****************************************************************************/

/*!
 * \brief Close Read/Write handler
 * \return errorcode
 */
AFX_EXT_GLF_INTERFACE ErrorCode CloseLogFileHandler(
                                                GLF_HANDLE& hRdWrHandler /*! [in] Read/Write file handler */
                                                );

} // GLF
