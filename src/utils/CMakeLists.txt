find_package(spdlog REQUIRED)
if (NOT spdlog_FOUND)
    message(FATAL_ERROR "spdlog not found")
endif()

add_library(freewb-utils SHARED ConvertUTF.c log.cpp)

if (NOT TARGET spdlog::spdlog)
    message(FATAL_ERROR "spdlog::spdlog target not found")
endif()

target_include_directories(freewb-utils PUBLIC ${CMAKE_SOURCE_DIR}/src/utils)
target_link_libraries(freewb-utils PUBLIC spdlog::spdlog)

install(TARGETS freewb-utils LIBRARY DESTINATION ${FREEWB_INSTALL_LIBDIR})