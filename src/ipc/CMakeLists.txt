message(STATUS "building ipc module")

set(IPC_SOURCES)
set(IPC_LINK_LIBS)

find_package(DBus1)
if (DBUS1_FOUND)
    message(STATUS "libdbus found, will build ${PROJECT_NAME}-ipc")
    set(IPC_SOURCES libdbus_proxy.cpp)
    link_directories(${DBus1_LIBRARY_DIRS})
    list(APPEND IPC_LINK_LIBS ${DBus1_LIBRARIES})
endif()

find_package(PkgConfig)
pkg_check_modules(SDBus libsystemd)
if(SDBus_FOUND)
    message(STATUS "${PROJECT_NAME}-ipc: SDBus (libsystemd)")
    list(APPEND IPC_SOURCES sdbus_proxy.cpp)
    list(APPEND IPC_LINK_LIBS ${SDBus_LIBRARIES})
endif()

find_package(Qt5 COMPONENTS DBus Gui)
if(Qt5DBus_FOUND)
    message(STATUS "${PROJECT_NAME}-ipc: Qt5DBus")
    set(CMAKE_AUTOMOC ON)
    list(APPEND IPC_SOURCES qdbus_panel_service.cpp qdbus_settings_service.cpp)
    list(APPEND IPC_LINK_LIBS Qt5::DBus Qt5::Gui)
endif()

if(NOT DBUS1_FOUND AND NOT SDBus_FOUND AND NOT Qt5DBus_FOUND)
    message(WARNING "freewb-ipc: neither DBus1 nor systemd (sd-bus) was found; ${PROJECT_NAME}-ipc will be built with no IPC sources. Install libdbus-1 and/or systemd development packages.")
    return()
endif()

add_library(${PROJECT_NAME}-ipc SHARED ${IPC_SOURCES})
target_include_directories(${PROJECT_NAME}-ipc PUBLIC ${CMAKE_SOURCE_DIR}/src/ipc)
if(DBUS1_FOUND)
    target_include_directories(${PROJECT_NAME}-ipc PRIVATE ${DBus1_INCLUDE_DIRS})
endif()

if(SDBus_FOUND)
    target_include_directories(${PROJECT_NAME}-ipc PRIVATE ${SDBus_INCLUDE_DIRS})
endif()

if (Qt5DBus_FOUND)
    target_include_directories(${PROJECT_NAME}-ipc PRIVATE ${Qt5DBus_INCLUDE_DIRS})
endif()

target_link_libraries(${PROJECT_NAME}-ipc PUBLIC ${PROJECT_NAME}-utils PRIVATE ${IPC_LINK_LIBS})

install(TARGETS ${PROJECT_NAME}-ipc LIBRARY DESTINATION ${FREEWB_INSTALL_LIBDIR})
