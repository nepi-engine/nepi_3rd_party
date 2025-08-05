cmake_minimum_required(VERSION 3.5.1)

PROJECT(GeminiSDK)

if (POLICY CMP0071)
  cmake_policy(SET CMP0071 OLD) # Policy introduced in CMake version 3.10
endif()

# default configuration
if( "${ARCH}" STREQUAL "")
    if(WIN32)
        Set(ARCH "ARCH32")
    else()
        Set(ARCH "ARCH64")
    endif()
endif()

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

message ( "Default architecture ARCH : ${ARCH}" )

add_definitions( -DCONSOLE_DLL_STATIC )
if("${ARCH}" STREQUAL "ARCH32")
    set(SDK_PLATFORM_NAME x32)
elseif("${ARCH}" STREQUAL "AARCH64")
    #arm arch64
    set(SDK_PLATFORM_NAME aarch64)
else()
    set(SDK_PLATFORM_NAME x64)
endif()

if( "${CMAKE_BUILD_TYPE}" STREQUAL ""  OR "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
    set(CMAKE_DEBUG_POSTFIX d)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

# add Maths define to use // M_E, M_LOG2E, M_LOG10E, M_LN2, M_LN10, M_PI, M_PI_2, M_PI_4, M_1_PI, M_2_PI, M_2_SQRTPI, M_SQRT2, M_SQRT1_2
add_definitions( -D_USE_MATH_DEFINES )

if( "${_LOW_PERFORMANCE_CPU_}" STREQUAL "1" )
    add_definitions(-D_LOW_PERFORMANCE_CPU_)
endif()

SET ( SHARED_DEPEND_LIST GeminiComms DataProcessor Svs5SeqLib GenesisSerializer )

#if enabled ECD logger then include ECD libraries
if( "${ECD_LOGGER}" STREQUAL "yes" )
    add_definitions( -D_ECD_LOGGER_ -DECDLOGDATATYPES_STATIC_LIBRARY)
    SET ( STATIC_DEPEND_LIST  MathsLib ECDLogDataTypes)
endif()

# Enable Data logger
if( "${DATA_LOGGER}" STREQUAL "yes" )
    add_definitions(-D_DATA_LOGGER_)
endif()

if( "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" )
    set(STATIC_LIB_PREFIX "")
    set(STATIC_LIB_SUFFIX lib)
    set(DYNAMIC_LIB_SUFFIX dll)
    set( PLATFORM_LIB_SUFFIX ${STATIC_LIB_SUFFIX} )

    set( LIB_ARCHITECTURE ${CMAKE_SYSTEM_NAME})
    SET(DEV_TOOLS_PATH "C:/devtools" )

    if( ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC )
        add_definitions(-D_WIN32_WINNT=0x0601 -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DCONSOLE_DLL_STATIC)
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19)
        message ("Visual Studio 2010 detected")

        set(BOOST_LOCAL_PATH    ${DEV_TOOLS_PATH}/boost/boost_1_61_0)
        set(FIND_BOOST_VERSION 1.61)

        if( "${ECD_LOGGER}" STREQUAL "yes" )
            if("${ARCH}" STREQUAL "ARCH32")
                set(BOOST_LIBRARYDIR    ${BOOST_LOCAL_PATH}/lib32-msvc-10.0)
            else()
                set(BOOST_LIBRARYDIR    ${BOOST_LOCAL_PATH}/lib64-msvc-10.1)
            endif()
        endif()
    else()
        message ("Visual Studio 2017 detected")
        set(BOOST_LOCAL_PATH    ${DEV_TOOLS_PATH}/boost/boost_1_68_0)
        set(FIND_BOOST_VERSION 1.68)

        if( "${ECD_LOGGER}" STREQUAL "yes" )
            if("${ARCH}" STREQUAL "ARCH32")
                set(BOOST_LIBRARYDIR    ${BOOST_LOCAL_PATH}/lib32-msvc-14.1)
            else()
                set(BOOST_LIBRARYDIR    ${BOOST_LOCAL_PATH}/lib64-msvc-14.1)
            endif()
        endif()
    endif()
    set(BOOST_STATIC_LINKING  ON)
    set(BOOST_MULTI_THREADED  ON)
else()
    set(STATIC_LIB_PREFIX lib)
    set(STATIC_LIB_SUFFIX a)
    set(DYNAMIC_LIB_SUFFIX so)
    Set(PLATFORM_LIB_SUFFIX ${DYNAMIC_LIB_SUFFIX} )

    if( "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv7at2hf-neon-angstrom" )
        Set (CMAKE_CXX_LIBRARY_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )
    elseif( "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv7l" ) #Rasp PI 3
        Set (CMAKE_CXX_LIBRARY_ARCHITECTURE ${CMAKE_SYSTEM_NAME} )
    endif()

    if( "${CMAKE_CXX_LIBRARY_ARCHITECTURE}" STREQUAL "" )
        set(CMAKE_CXX_LIBRARY_ARCHITECTURE ${CMAKE_SYSTEM_NAME})
    endif()

    Set( LIB_ARCHITECTURE ${CMAKE_CXX_LIBRARY_ARCHITECTURE})

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

    # Enable static linking for gcc and stdc++
    set(CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++ -static-libgcc ${CMAKE_SHARED_LINKER_FLAGS}")

    if( "${ECD_LOGGER}" STREQUAL "yes" )
        set(FIND_BOOST_VERSION "")
        set(BOOST_MULTI_THREADED OFF)

        if("${LINK_TYPE}" STREQUAL "Static")
            set(BOOST_STATIC_LINKING ON)
			
			#This is to cater for the new style buil location on the Build Server and MS VM's
			if( "$ENV{TRITECH_DIR}" STREQUAL "" )
				set(BOOST_ROOT       "$ENV{HOME}/devTools/boost")           #Original
			else()
				set(BOOST_ROOT       "$ENV{TRITECH_DIR}/devtools/boost")    #New
			endif()			
            
            set(BOOST_LOCAL_PATH "${BOOST_ROOT}/boost_1_68_0/")

            if("${ARCH}" STREQUAL "ARCH32")
                set(BOOST_LIBRARYDIR "${BOOST_LOCAL_PATH}/lib_i386")
            elseif("${ARCH}" STREQUAL "ARCH64")
                set(BOOST_LIBRARYDIR "${BOOST_LOCAL_PATH}/lib_x86_64")
            else()
                message("Fatal error, Static linking not supported for the target platform !!!!")
            endif()
        else()
            add_definitions(-DBOOST_ALL_DYN_LINK)
            set(BOOST_STATIC_LINKING OFF)
            if (EXISTS $ENV{OECORE_TARGET_SYSROOT} )
                Set( BOOST_LOCAL_PATH "$ENV{OECORE_TARGET_SYSROOT}/usr")
                set( BOOST_LIBRARYDIR "${BOOST_LOCAL_PATH}/lib")
            elseif (EXISTS $ENV{TE0726_TARGET_SYSROOT})
                Set( BOOST_LOCAL_PATH "$ENV{OECORE_TARGET_SYSROOT}/usr")
                set( BOOST_LIBRARYDIR "${BOOST_LOCAL_PATH}/lib")
            else()
                set(BOOST_LOCAL_PATH "/usr/include")
                set(BOOST_LIBRARYDIR "${BOOST_LOCAL_PATH}/lib")
            endif()
        endif()
    endif()
endif()

foreach(EXT_LIBS ${SHARED_DEPEND_LIST})
   set( ${EXT_LIBS}_LIB     ${CMAKE_CURRENT_SOURCE_DIR}/libs/${EXT_LIBS}/${LIB_ARCHITECTURE}/${SDK_PLATFORM_NAME}/${CMAKE_BUILD_TYPE}/${STATIC_LIB_PREFIX}${EXT_LIBS}${CMAKE_DEBUG_POSTFIX}.${PLATFORM_LIB_SUFFIX})
   set( ${EXT_LIBS}_DLL     ${CMAKE_CURRENT_SOURCE_DIR}/libs/${EXT_LIBS}/${LIB_ARCHITECTURE}/${SDK_PLATFORM_NAME}/${CMAKE_BUILD_TYPE}/${STATIC_LIB_PREFIX}${EXT_LIBS}${CMAKE_DEBUG_POSTFIX}.${DYNAMIC_LIB_SUFFIX})
endforeach()

foreach(EXT_LIBS ${STATIC_DEPEND_LIST})
   set( ${EXT_LIBS}_LIB     ${CMAKE_CURRENT_SOURCE_DIR}/libs/${EXT_LIBS}/${LIB_ARCHITECTURE}/${SDK_PLATFORM_NAME}/${CMAKE_BUILD_TYPE}/${STATIC_LIB_PREFIX}${EXT_LIBS}${CMAKE_DEBUG_POSTFIX}.${STATIC_LIB_SUFFIX})
endforeach()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if( "${ECD_LOGGER}" STREQUAL "yes" )
    set(BOOST_ROOT                  ${BOOST_LOCAL_PATH})
    set(Boost_USE_STATIC_LIBS       ${BOOST_STATIC_LINKING})
    set(Boost_USE_MULTITHREADED     ${BOOST_MULTI_THREADED})

    #######################################################################################################
    # Find boost componenets based on the path specified above
    #######################################################################################################

    FIND_PACKAGE(Boost ${FIND_BOOST_VERSION})
    IF (Boost_FOUND)
        INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
        ADD_DEFINITIONS( "-DHAS_BOOST" )

        set(BoostComponents date_time filesystem system iostreams thread program_options serialization regex locale)

        FIND_PACKAGE(Boost REQUIRED COMPONENTS ${BoostComponents}  OPTIONAL_COMPONENTS zlib)

    #message(${Boost_INCLUDE_DIRS})
    #message(${Boost_LIBRARIES})
    endif()
endif()

if(WIN32)
    if( "${ARCH}" STREQUAL "ARCH32")
        Set( GEMINIS_SDK_BIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/x32/" )
    else()
        Set( GEMINIS_SDK_BIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/x64/" )
    endif()
else()
    Set( GEMINIS_SDK_BIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/" )
endif()

add_subdirectory (src/GeminiSDKConsoleApp)
add_subdirectory (src/Svs5LoggerApp)
add_subdirectory (src/GenesisSerializerApp)
add_subdirectory (src/GeminiSDKGuiApp)


# installing library file (.exe )
Set( EXPORT_EXE_FILES
${CMAKE_CURRENT_BINARY_DIR}/src/GeminiSDKConsoleApp/GeminiSDKConsoleApp
${CMAKE_CURRENT_BINARY_DIR}/src/Svs5LoggerApp/Svs5LogReader
${CMAKE_CURRENT_BINARY_DIR}/src/Svs5LoggerApp/Svs5LogWriter
${CMAKE_CURRENT_BINARY_DIR}/src/GenesisSerializerApp/SynchronousLogReader
${CMAKE_CURRENT_BINARY_DIR}/src/GenesisSerializerApp/AsynchronousLogReader
${CMAKE_CURRENT_BINARY_DIR}/src/GenesisSerializerApp/FastLogReader
)


# copy library directories
foreach ( exe_files ${EXPORT_EXE_FILES} )
    if(WIN32)
        install( FILES ${exe_files}.exe
                PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                DESTINATION "${GEMINIS_SDK_BIN_PATH}"
                )
    else()
        install( FILES ${exe_files}
                PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                DESTINATION "${GEMINIS_SDK_BIN_PATH}"
                )
    endif()
endforeach()

if(NOT WIN32)
    if( NOT "${DISABLE_SERIAL}" STREQUAL "yes")
        install( FILES "${CMAKE_CURRENT_SOURCE_DIR}/Tritech-USB-Serial.sh"
                    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                    DESTINATION "${GEMINIS_SDK_BIN_PATH}"
                    )
    endif()
endif()

