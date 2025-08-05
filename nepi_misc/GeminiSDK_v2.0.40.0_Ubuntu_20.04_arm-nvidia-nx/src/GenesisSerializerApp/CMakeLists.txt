PROJECT(GenesisSerializerApp)

if (NOT WIN32)
    if( "${ARCH}" STREQUAL "AARCH64" )
        Set (RaspPi_LIBS ${DataProcessor_LIB} )	#For Raspberry PI
    endif()
endif()

add_executable(FastLogReader FastLogReader.cpp )
TARGET_LINK_LIBRARIES( FastLogReader
                        ${GenesisSerializer_LIB}
			${RaspPi_LIBS}
                        ${SYSTEM_LIBRARIES}
                        )

add_executable(SynchronousLogReader SynchronousLogReader.cpp )
TARGET_LINK_LIBRARIES( SynchronousLogReader
                        ${GenesisSerializer_LIB}
			${RaspPi_LIBS}
                        ${SYSTEM_LIBRARIES}
                        )

add_executable(AsynchronousLogReader AsynchronousLogReader.cpp )
TARGET_LINK_LIBRARIES( AsynchronousLogReader
                        ${GenesisSerializer_LIB}
			${RaspPi_LIBS}
                        ${SYSTEM_LIBRARIES}
                        )

