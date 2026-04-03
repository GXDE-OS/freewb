message(STATUS "ipc module build")

set(SOURCES)
set(IPC_LINK_LIBS)

find_package(DBus1)

if (DBUS1_FOUND)
    message(STATUS "libdbus found, will build ${PROJECT_NAME}-ipc")
    set(SOURCES freewbinterface.c)
    link_directories(${DBus1_LIBRARY_DIRS})
    list(APPEND IPC_LINK_LIBS ${DBus1_LIBRARIES})
endif()

find_package(PkgConfig)
pkg_check_modules(SD_PC libsystemd)
if(SD_PC_FOUND)
    message(STATUS "${PROJECT_NAME}-ipc: sdbus_proxy (libsystemd)")
    list(APPEND SOURCES sdbus_proxy.cpp)
    list(APPEND IPC_LINK_LIBS ${SD_PC_LIBRARIES})
endif()

if(NOT DBUS1_FOUND AND NOT SD_PC_FOUND)
    message(WARNING "freewb-ipc: neither DBus1 nor systemd (sd-bus) was found; ${PROJECT_NAME}-ipc will be built with no IPC sources. Install libdbus-1 and/or systemd development packages.")
endif()

add_library(${PROJECT_NAME}-ipc SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME}-ipc PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/ipc)
if(DBUS1_FOUND)
    target_include_directories(${PROJECT_NAME}-ipc PRIVATE ${DBus1_INCLUDE_DIRS})
endif()
if(SD_PC_FOUND)
    target_include_directories(${PROJECT_NAME}-ipc PRIVATE ${SD_PC_INCLUDE_DIRS})
endif()
target_link_libraries(${PROJECT_NAME}-ipc PUBLIC freewb-core freewb-utils PRIVATE ${IPC_LINK_LIBS})

install(TARGETS ${PROJECT_NAME}-ipc LIBRARY DESTINATION ${FREEWB_INSTALL_LIBDIR})
