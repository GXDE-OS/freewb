find_package(Fcitx5Utils REQUIRED)

add_executable(freewb-sdbus-proxy-test sdbus_proxy_test.cpp)
target_link_libraries(freewb-sdbus-proxy-test PRIVATE freewb-ipc Fcitx5::Utils)
