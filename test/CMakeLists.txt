find_package(Fcitx5Utils REQUIRED)

add_executable(freewb-sdbusproxy-test test.cpp)
target_link_libraries(freewb-sdbusproxy-test PRIVATE freewb-ipc Fcitx5::Utils)
