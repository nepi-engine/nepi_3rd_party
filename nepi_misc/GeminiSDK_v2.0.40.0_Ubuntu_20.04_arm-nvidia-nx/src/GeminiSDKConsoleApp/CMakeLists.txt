PROJECT(GeminiSDKConsoleApp)

file(GLOB_RECURSE Source *.cpp)
file(GLOB_RECURSE Header *.h )

# hotkey class only supported on Windows platform
if( NOT "${DATA_LOGGER}" STREQUAL "yes" OR NOT WIN32)
    file(GLOB_RECURSE To_Remove hotkeys.cpp)
    list(REMOVE_ITEM Source ${To_Remove})
endif()

#For Raspberry PI Builds
if (NOT WIN32)
    if( "${ARCH}" STREQUAL "AARCH64" )	
        Set (RaspPi_LIBS ${DataProcessor_LIB})	
    endif()
endif()

add_executable(${PROJECT_NAME} ${Source} ${Header})

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	${RaspPi_LIBS}
    ${GenesisSerializer_LIB}
    ${Svs5SeqLib_LIB}
    ${GeminiComms_LIB}
    ${SYSTEM_LIBRARIES}
)


