message(STATUS "Building fcitx4 module")

include_directories(${CMAKE_SOURCE_DIR}/include)

set(fcitx_freewubi_sources
  freewubi-config.c
  freewubi.c
  ini.c
  freedict.c
  freespecial.c
  freeinterface.c
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
  freeinterface.h
  virtual_keyboard.h
  panelproxy.h
  inputstate.h
)

fcitx_add_addon(fcitx-${PROJECT_NAME} SHARED ${fcitx_freewubi_sources} ${fcitx_freewubi_headers})

target_link_libraries(fcitx-${PROJECT_NAME}
    ${LIBINTL_LIBRARIES}
    ${LIBICONV_LIBRARIES}
    ${DBus1_LIBRARIES}
)

configure_file(freewb.conf.in ${CMAKE_CURRENT_BINARY_DIR}/freewb.conf IMMEDIATE @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/freewb.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/fcitx/inputmethod)

configure_file(fcitx-freewb.conf.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx-freewb.conf IMMEDIATE @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-freewb.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/fcitx/addon)
