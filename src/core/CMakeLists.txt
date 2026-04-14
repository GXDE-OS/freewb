message(STATUS "Building core module")

add_library(${PROJECT_NAME}-core SHARED freewb.cpp candidatelist.cpp)
target_include_directories(${PROJECT_NAME}-core PUBLIC ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/ipc)
target_link_libraries(${PROJECT_NAME}-core PUBLIC ${PROJECT_NAME}-utils ${PROJECT_NAME}-ipc)

install(TARGETS ${PROJECT_NAME}-core DESTINATION ${FREEWB_INSTALL_LIBDIR})