find_package(spdlog REQUIRED)
if (NOT spdlog_FOUND)
    message(FATAL_ERROR "spdlog not found")
endif()

add_library(freewb-utils SHARED ConvertUTF.c log.cpp)

if (spdlog_FOUND)
    target_link_libraries(freewb-utils ${spdlog_LIBRARIES})
endif()

install(TARGETS freewb-utils LIBRARY DESTINATION ${FREEWB_INSTALL_LIBDIR})