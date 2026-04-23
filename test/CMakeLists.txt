message(STATUS "will build test cases")

if (ENABLE_FCITX5)
    find_package(Fcitx5Utils REQUIRED)
    add_executable(freewb-sdbus-proxy-test sdbus_proxy_test.cpp)
    target_link_libraries(freewb-sdbus-proxy-test PRIVATE ${PROJECT_NAME}-ipc Fcitx5::Utils)
    add_test(NAME freewb-sdbus-proxy-test COMMAND freewb-sdbus-proxy-test)
endif()

add_executable(freewb-key-test key_test.cpp ${CMAKE_SOURCE_DIR}/src/utils/key.cpp)
target_include_directories(freewb-key-test PRIVATE ${CMAKE_SOURCE_DIR}/src/core)
target_link_libraries(freewb-key-test PRIVATE ${PROJECT_NAME}-utils)
add_test(NAME freewb-key-test COMMAND freewb-key-test)

add_executable(freewb-sound-test sound_test.cpp)
target_include_directories(freewb-sound-test PRIVATE ${CMAKE_SOURCE_DIR}/src/utils)
target_link_libraries(freewb-sound-test PRIVATE ${PROJECT_NAME}-utils)
add_test(NAME freewb-sound-test COMMAND freewb-sound-test)

add_executable(freewb-candidate-list-test candidate_list_test.cpp)
target_include_directories(freewb-candidate-list-test PRIVATE ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/utils)
target_link_libraries(freewb-candidate-list-test PRIVATE ${PROJECT_NAME}-core ${PROJECT_NAME}-utils)
add_test(NAME freewb-candidate-list-test COMMAND freewb-candidate-list-test)

add_executable(freewb-chttrans-test chttrans_test.cpp)
target_include_directories(freewb-chttrans-test PRIVATE ${CMAKE_SOURCE_DIR}/src/core)
target_link_libraries(freewb-chttrans-test PRIVATE ${PROJECT_NAME}-core)
add_test(NAME freewb-chttrans-test COMMAND freewb-chttrans-test)

add_executable(freewb-test-engine-wbzx engine_wbzx_test.cpp)
target_include_directories(freewb-test-engine-wbzx PRIVATE ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/utils ${CMAKE_SOURCE_DIR}/src/engine)
target_link_libraries(freewb-test-engine-wbzx PRIVATE ${PROJECT_NAME}-engine-wbzx ${PROJECT_NAME}-utils ${PROJECT_NAME}-core)

add_executable(freewb-test-engine-py engine_py_test.cpp)
target_include_directories(freewb-test-engine-py PRIVATE ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/utils ${CMAKE_SOURCE_DIR}/src/engine)
target_link_libraries(freewb-test-engine-py PRIVATE ${PROJECT_NAME}-engine-py ${PROJECT_NAME}-utils ${PROJECT_NAME}-core)

add_executable(freewb-user-dict-test user_dict_test.cpp)
target_include_directories(freewb-user-dict-test PRIVATE ${CMAKE_SOURCE_DIR}/src/engine ${CMAKE_SOURCE_DIR}/src/utils)
target_link_libraries(freewb-user-dict-test PRIVATE ${PROJECT_NAME}-engine-wbzx ${PROJECT_NAME}-utils)
add_test(NAME freewb-user-dict-test COMMAND freewb-user-dict-test)

