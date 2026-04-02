message(STATUS "ipc module build")

find_package(DBus1 REQUIRED)
if (DBUS1_FOUND)
    message(STATUS "libdbus found, will build ${PROJECT_NAME}-ipc")
    set(SOURCES freewbinterface.c)
    include_directories(${DBus1_INCLUDE_DIRS})
    link_directories(${DBus1_LIBRARY_DIRS})
endif()

add_library(${PROJECT_NAME}-ipc SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME}-ipc ${DBus1_LIBRARIES})

install(TARGETS ${PROJECT_NAME}-ipc LIBRARY DESTINATION ${FREEWB_INSTALL_LIBDIR})
