PROJECT(GeminiSDKGuiApp)

message( "GeminiSDKGuiApp" )

# Turn on automatic invocation of the MOC
set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE Source *.cpp)
file(GLOB_RECURSE Header *.h )
file(GLOB_RECURSE Ui "*.ui")

if (NOT WIN32)
    if (EXISTS "$ENV{OECORE_TARGET_SYSROOT}/usr/include/GLES3/gl3ext.h")
        add_definitions("-DUSE_GLES")
    endif()
	if ( ARCH STREQUAL "AARCH64")
		add_definitions("-DRASPI")
	endif()
endif()

if (WIN32)
    # Note : Not specifically allowing to select Qt from the environment path,
    # On the developer machine, QT path is selected for QT5.13.0 using VS2017
    # For VS2010, set the environment path at the command line
    # User can modify according to the developer environment

    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19)
        if( "${ARCH}" STREQUAL "ARCH32" AND NOT $ENV{QTDIR} STREQUAL "" )
            Set( CMAKE_PREFIX_PATH $ENV{QTDIR}\\lib\\cmake)
            Set( WINDEPLOYQT "$ENV{QTDIR}\\bin\\windeployqt.exe")
        elseif( "${ARCH}" STREQUAL "ARCH64" AND NOT $ENV{QTDIR_64} STREQUAL "" )
            Set( CMAKE_PREFIX_PATH $ENV{QTDIR_64}\\lib\\cmake)
            Set( WINDEPLOYQT "$ENV{QTDIR_64}\\bin\\windeployqt.exe")
        endif()
    endif()

	if("${ARCH}" STREQUAL "ARCH32")
	  # FreeType OpenGL font library
	  set(FreeType_DIR "${DEV_TOOLS_PATH}/freetype_2.3.5/freetype/")
	  SET (FreeType_DLL "${FreeType_DIR}/bin/freetype6.dll")
	  SET (FreeType_ZLib_DLL "${FreeType_DIR}/bin/zlib1.dll")

	else()
		# FreeType OpenGL font library
		set(FreeType_DIR "${DEV_TOOLS_PATH}/freetype/freetype_2.10.0")
		SET (FreeType_DLL "${FreeType_DIR}/bin/freetype.dll")
	endif()

	#FreeType OpenGL font library
	set(FreeType_LIB "${FreeType_DIR}/lib/freetype.lib")
	
	#To align with Linux below.
	set(FREETYPE_LIBRARIES ${FreeType_LIB} )
	include_directories("${FreeType_DIR}/include")
	include_directories("${FreeType_DIR}/include/freetype2")	

	
    # Add Windows specific openGL libraries
    SET(SYSTEM_LIBRARIES "opengl32.lib glu32.lib" )
else()
    # First check, if configuration exist ?
    if ( EXISTS $ENV{OECORE_TARGET_SYSROOT} )
        Set( CMAKE_PREFIX_PATH $ENV{OECORE_TARGET_SYSROOT}/usr/lib/cmake)
    endif()
    if (EXISTS $ENV{OE_QMAKE_HOST_BINDIR_QT})
        set(OE_QMAKE_PATH_EXTERNAL_HOST_BINS "$ENV{OE_QMAKE_HOST_BINDIR_QT}")
    elseif (EXISTS $ENV{OE_QMAKE_PATH_HOST_BINS})
        set(OE_QMAKE_PATH_EXTERNAL_HOST_BINS "$ENV{OE_QMAKE_PATH_HOST_BINS}")
    endif()	    

	#CMAKE version on Linux allows this
	include(FindFreetype)

	if( FREETYPE_FOUND )
			include_directories(${FREETYPE_INCLUDE_DIRS})

			message("FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS}")
			message("FREETYPE_LIBRARIES ${FREETYPE_LIBRARIES}")
		else()
			message("FREETYPE FONTS NOT FOUND!  Please install.")

	endif()	
endif()


find_package(Qt5 OPTIONAL_COMPONENTS Core Widgets Gui)

if( NOT Qt5Widgets_FOUND )
    message( "Qt libraries not found, ${PROJECT_NAME} not compiling !!" )
    return()
endif()


# There's no otherway to find that TI (06.00.00.07)SDK has been used except this is the unique flag set in the
# environment settings
# Note:: Latest TI SDK does not support '::'. :: means further down in the directory
if( DEFINED ENV{PS1} )
    set(QT5_APP_LIBS    "Qt5Core;Qt5Widgets;Qt5Gui;GLESv2" )
else()
    set(QT5_APP_LIBS    "Qt5::Core;Qt5::Widgets;Qt5::Gui;GLESv2;" )
endif()

# Generate code from ui files
qt5_wrap_ui(UI_HEADERS ${Ui})
qt5_add_resources(RESOURCES resources.qrc)

add_executable(${PROJECT_NAME} ${UI_HEADERS} ${Source} ${Header} ${RESOURCES})

if(WIN32)
    # suppress PDB missing warnings
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
    set(QT5_APP_LIBS    "Qt5::Widgets" )
endif()


TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	${ECDLogDataTypes_LIB}
	${MathsLib_LIB}
	${GenesisSerializer_LIB}
	${Svs5SeqLib_LIB}
	${GeminiComms_LIB}
	${FREETYPE_LIBRARIES}
	${SYSTEM_LIBRARIES}
	${QT5_APP_LIBS}
	${Boost_LIBRARIES}
)


add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E make_directory ${GEMINIS_SDK_BIN_PATH}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}> ${GEMINIS_SDK_BIN_PATH}
                   )
#
# Run WINDEPLOYQT to generate Qt dependency files
#
message( "GeminiSDKGuiApp WINDEPLOYQT........${WINDEPLOYQT}" )
if( WINDEPLOYQT )
	message( "==========================Running WINDEPLOYQT==================================" )
    #check if file exists, if not then run deployment
    if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/Qt5Quick${CMAKE_DEBUG_POSTFIX}.dll)
        # execute windeployqt in a tmp directory after build
        add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMENT "Deploying QT dependency files ..."
            COMMAND ${WINDEPLOYQT} -quick  "$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        )
    endif()
endif()

message( "End GeminiSDKGuiApp" )