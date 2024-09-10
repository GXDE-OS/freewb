cmake_minimum_required(VERSION 2.6)
project(freewbConversionTool)

set(CONVERSION_SOURCES
  freewbConversionTool.c
)

add_library(${PROJECT_NAME} SHARED ${CONVERSION_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION 1.0 SOVERSION 1)
target_link_libraries(${PROJECT_NAME} fcitx-config fcitx-utils)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
