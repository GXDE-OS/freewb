include_directories(${CMAKE_SOURCE_DIR}/src/)

add_subdirectory(tools)
add_subdirectory(panel)

if(ENABLE_FCITX4)
    add_subdirectory(inputmethod)
endif()