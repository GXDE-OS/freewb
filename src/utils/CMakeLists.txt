find_package(spdlog REQUIRED)
if (NOT spdlog_FOUND)
    message(FATAL_ERROR "spdlog not found")
endif()

add_library(${PROJECT_NAME}-utils SHARED ConvertUTF.c log.cpp settings.cpp key.cpp)

if (NOT TARGET spdlog::spdlog)
    message(FATAL_ERROR "spdlog::spdlog target not found")
endif()

target_include_directories(${PROJECT_NAME}-utils PUBLIC ${CMAKE_SOURCE_DIR}/src/utils)
target_link_libraries(${PROJECT_NAME}-utils PUBLIC spdlog::spdlog)

install(TARGETS ${PROJECT_NAME}-utils LIBRARY DESTINATION ${FREEWB_INSTALL_LIBDIR})