find_package(Fcitx5Utils REQUIRED)

add_executable(freewb-sdbus-proxy-test sdbus_proxy_test.cpp)
target_link_libraries(freewb-sdbus-proxy-test PRIVATE freewb-ipc Fcitx5::Utils)

add_executable(freewb-key-test key_test.cpp ${CMAKE_SOURCE_DIR}/src/utils/key.cpp)
target_include_directories(freewb-key-test PRIVATE ${CMAKE_SOURCE_DIR}/src/core)
target_link_libraries(freewb-key-test PRIVATE ${PROJECT_NAME}-utils)
add_test(NAME freewb-key-test COMMAND freewb-key-test)
