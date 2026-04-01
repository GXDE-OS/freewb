include_directories(${CMAKE_SOURCE_DIR}/src/)

add_subdirectory(utils)
add_subdirectory(tools)
add_subdirectory(panel)

if("${OS_TYPE}" STREQUAL "Linux" AND ENABLE_FCITX4)
    add_subdirectory(fcitx4)
endif()

if("${OS_TYPE}" STREQUAL "Linux" AND ENABLE_FCITX5)
    add_subdirectory(fcitx5)
endif()