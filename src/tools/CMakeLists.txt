cmake_minimum_required(VERSION 2.6)
project(freewbConversionTool)

if (${OS_TYPE} MATCHES "Linux")
    set(CONVERSION_SOURCES
        freewbConversionTool_linux.c
    )
endif()

add_library(${PROJECT_NAME} SHARED ${CONVERSION_SOURCES})
if (ENABLE_FCITX4)
    target_link_libraries(${PROJECT_NAME} fcitx-config fcitx-utils)
endif()

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${FREEWB_INSTALL_LIBDIR})
