message(STATUS "building utils module")

add_library(${PROJECT_NAME}-utils SHARED
    ConvertUTF.c
    settings.cpp
    key.cpp
    utils.cpp
    sound.cpp
)

target_include_directories(${PROJECT_NAME}-utils PUBLIC ${CMAKE_SOURCE_DIR}/src/utils)

find_package(spdlog QUIET)
if (spdlog_FOUND)
    target_sources(${PROJECT_NAME}-utils PRIVATE log.cpp)
    target_link_libraries(${PROJECT_NAME}-utils PUBLIC spdlog::spdlog)
endif()

install(TARGETS ${PROJECT_NAME}-utils LIBRARY DESTINATION ${FREEWB_INSTALL_LIBDIR})
