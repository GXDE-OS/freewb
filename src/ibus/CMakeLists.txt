message(STATUS "Building ibus module")

add_definitions(-D__IBUS__)

find_package(PkgConfig REQUIRED)
pkg_check_modules(IBUS REQUIRED ibus-1.0)
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(GLIB REQUIRED glib-2.0)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/ipc
    ${IBUS_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
)

add_executable(ibus-engine-freewb
    main.cpp
    engine.cpp
    dbus_gmain.cpp
)

target_link_libraries(ibus-engine-freewb
    ${PROJECT_NAME}-utils
    ${PROJECT_NAME}-core
    ${PROJECT_NAME}-ipc
    ${IBUS_LIBRARIES}
    ${DBUS_LIBRARIES}
    ${GLIB_LIBRARIES}
)

target_compile_options(ibus-engine-freewb PRIVATE ${IBUS_CFLAGS_OTHER} ${DBUS_CFLAGS_OTHER} ${GLIB_CFLAGS_OTHER})

include(GNUInstallDirs)
set(FREEWB_IBUS_ENGINE_PATH "${CMAKE_INSTALL_FULL_LIBEXECDIR}/ibus-engine-freewb")
set(FREEWB_TEXT_DOMAIN "freewb")

configure_file(freewb.xml.in ${CMAKE_CURRENT_BINARY_DIR}/freewb.xml @ONLY)

install(TARGETS ibus-engine-freewb DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/freewb.xml DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/ibus/component)
