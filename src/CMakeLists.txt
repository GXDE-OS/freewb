set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN;${FREEWB_INSTALL_LIBDIR};")

include_directories("${PROJECT_SOURCE_DIR}/include")

add_subdirectory(utils)
add_subdirectory(ipc)
add_subdirectory(engine)
add_subdirectory(core)
add_subdirectory(panel)
add_subdirectory(tools)

if("${OS_TYPE}" STREQUAL "Linux" AND ENABLE_FCITX4)
    add_subdirectory(fcitx4)
endif()

if("${OS_TYPE}" STREQUAL "Linux" AND ENABLE_FCITX5)
    add_subdirectory(fcitx5)
endif()