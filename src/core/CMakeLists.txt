message(STATUS "Building core module")

add_library(${PROJECT_NAME}-core SHARED freewb.cpp candidatelist.cpp enginemanager.cpp committer.cpp chttrans.cpp)
target_include_directories(${PROJECT_NAME}-core PUBLIC ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/ipc ${CMAKE_SOURCE_DIR}/src/engine)
target_link_libraries(${PROJECT_NAME}-core PUBLIC ${PROJECT_NAME}-utils ${PROJECT_NAME}-ipc ${PROJECT_NAME}-engine-wbzx ${PROJECT_NAME}-engine-py ${PROJECT_NAME}-engine-wbpy ${PROJECT_NAME}-engine-en)

find_package(OpenCC REQUIRED)
target_link_libraries(${PROJECT_NAME}-core PUBLIC OpenCC::OpenCC)

install(TARGETS ${PROJECT_NAME}-core DESTINATION ${FREEWB_INSTALL_LIBDIR})