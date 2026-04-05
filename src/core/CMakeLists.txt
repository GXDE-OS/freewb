message(STATUS "Building core module")

add_library(${PROJECT_NAME}-core SHARED freewb.cpp)
target_include_directories(${PROJECT_NAME}-core PUBLIC ${CMAKE_SOURCE_DIR}/src/core)
target_link_libraries(${PROJECT_NAME}-core PUBLIC freewb-utils)

install(TARGETS ${PROJECT_NAME}-core DESTINATION ${FREEWB_INSTALL_LIBDIR})