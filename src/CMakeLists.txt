set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN;${FREEWB_INSTALL_LIBDIR};")

add_subdirectory(utils)
add_subdirectory(tools)
add_subdirectory(panel)
add_subdirectory(core)
add_subdirectory(ipc)

if("${OS_TYPE}" STREQUAL "Linux" AND ENABLE_FCITX4)
    add_subdirectory(fcitx4)
endif()

if("${OS_TYPE}" STREQUAL "Linux" AND ENABLE_FCITX5)
    add_subdirectory(fcitx5)
endif()