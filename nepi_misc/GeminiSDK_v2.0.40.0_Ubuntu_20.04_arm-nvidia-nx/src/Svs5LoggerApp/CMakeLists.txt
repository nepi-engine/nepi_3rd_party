PROJECT(Svs5LoggerApp)

add_executable(Svs5LogReader Svs5LogReader.cpp )
add_executable(Svs5LogWriter Svs5LogWriter.cpp)

#Raspberry PI
if (NOT WIN32)
    if( "${ARCH}" STREQUAL "AARCH64" )	
        Set (RaspPi_LIBS ${DataProcessor_LIB})	
    endif()
endif()

TARGET_LINK_LIBRARIES(Svs5LogReader
			${RaspPi_LIBS}
                        ${GenesisSerializer_LIB}
                        ${Svs5SeqLib_LIB}
                        ${GeminiComms_LIB}
                        ${SYSTEM_LIBRARIES}
                        )

TARGET_LINK_LIBRARIES(Svs5LogWriter
			${RaspPi_LIBS}
                        ${GenesisSerializer_LIB}
                        ${Svs5SeqLib_LIB}
                        ${GeminiComms_LIB}
                        ${SYSTEM_LIBRARIES}
                        )