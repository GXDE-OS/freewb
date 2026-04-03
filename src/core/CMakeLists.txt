message(STATUS "Building core module")

add_library(${PROJECT_NAME}-core SHARED freewb.cpp)
target_link_libraries(${PROJECT_NAME}-core freewb-utils)

install(TARGETS ${PROJECT_NAME}-core DESTINATION ${FREEWB_INSTALL_LIBDIR})