message(STATUS "Building fcitx4 module")

add_definitions(-D__FCITX4__)

find_package(Fcitx 4.2.7 REQUIRED)
if (FCITX_FOUND)
  include_directories(${FCITX4_FCITX_INCLUDE_DIRS})
  include_directories(${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
  include_directories(${FCITX4_FCITX_UTILS_INCLUDE_DIRS})
  message(STATUS "fcitx4 is found, libraries is: ${FCITX4_FCITX_LIBRARIES}")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

set(fcitx_freewubi_sources
  freewubi-config.c
  freewubi.c
  ini.c
  freedict.c
  freespecial.c
  virtual_keyboard.c
  panelproxy.c
  inputstate.c
)

set(fcitx_freewubi_headers
  ini.h
  freewubi-config.h
  freewubi-internal.h
  freedict.h
  utf8_in_gb18030.h
  freespecial.h
  virtual_keyboard.h
  panelproxy.h
  inputstate.h
)

fcitx_add_addon(lib${PROJECT_NAME}-fcitx4 SHARED ${fcitx_freewubi_sources} ${fcitx_freewubi_headers})

target_link_libraries(lib${PROJECT_NAME}-fcitx4
    ${LIBINTL_LIBRARIES}
    ${LIBICONV_LIBRARIES}
    ${PROJECT_NAME}-ipc
    ${FCITX4_FCITX_LIBRARIES}
)

configure_file(freewb.conf.in ${CMAKE_CURRENT_BINARY_DIR}/freewb.conf IMMEDIATE @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/freewb.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/fcitx/inputmethod)

configure_file(fcitx-freewb.conf.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx-freewb.conf IMMEDIATE @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-freewb.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/fcitx/addon)
