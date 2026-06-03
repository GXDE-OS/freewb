message(STATUS "Building conversion tool module")

add_library(${PROJECT_NAME}-tools SHARED ConversionTool.cpp ${CMAKE_SOURCE_DIR}/src/engine/common.cpp)
target_include_directories(${PROJECT_NAME}-tools PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/engine
    ${CMAKE_SOURCE_DIR}/src/utils)
target_link_libraries(${PROJECT_NAME}-tools PUBLIC ${PROJECT_NAME}-utils)

install(TARGETS ${PROJECT_NAME}-tools LIBRARY DESTINATION ${FREEWB_INSTALL_LIBDIR})
