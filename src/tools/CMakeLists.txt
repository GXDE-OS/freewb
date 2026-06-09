message(STATUS "Building conversion tool module")

add_library(${PROJECT_NAME}-tools SHARED ConversionTool.cpp ${CMAKE_SOURCE_DIR}/src/engine/common.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/gb2312filter.cpp)
target_include_directories(${PROJECT_NAME}-tools PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/engine
    ${CMAKE_SOURCE_DIR}/src/utils)
target_link_libraries(${PROJECT_NAME}-tools PUBLIC ${PROJECT_NAME}-utils)
if(ICONV_LIBRARIES)
    target_link_libraries(${PROJECT_NAME}-tools PRIVATE ${ICONV_LIBRARIES})
endif()

install(TARGETS ${PROJECT_NAME}-tools LIBRARY DESTINATION ${FREEWB_INSTALL_LIBDIR})
