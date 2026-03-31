add_subdirectory(mb)
add_subdirectory(s2t)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/org.freewb.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/org.freewb.desktop IMMEDIATE @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freewb.desktop DESTINATION /etc/xdg/autostart )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/org.freewb.start.sh.in ${CMAKE_CURRENT_BINARY_DIR}/org.freewb.start.sh IMMEDIATE @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/org.freewb.start.sh DESTINATION ${FREEWB_INSTALL_BINDIR}/)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.ini.in ${CMAKE_CURRENT_BINARY_DIR}/config.ini IMMEDIATE @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.ini DESTINATION ${FREEWB_INSTALL_PKGDATADIR}/config)
