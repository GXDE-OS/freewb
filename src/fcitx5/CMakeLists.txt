message(STATUS "Building fcitx5 module")

add_definitions(-D__FCITX5__)

find_package(Fcitx5Core REQUIRED)
if (NOT Fcitx5Core_FOUND)
    message(FATAL_ERROR "Fcitx5Core not found,will not build fcitx5 im module.")
    return()
endif()

include_directories(${CMAKE_SOURCE_DIR}/src/utils)

add_library("${PROJECT_NAME}-fcitx5" SHARED module.cpp)

target_link_libraries("${PROJECT_NAME}-fcitx5" Fcitx5::Core freewb-utils)

install(TARGETS "${PROJECT_NAME}-fcitx5" DESTINATION "${FCITX_INSTALL_LIBDIR}/fcitx5")

configure_file(freewb-addon.conf.in freewb-addon.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/freewb-addon.conf" RENAME freewb.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")

configure_file(freewb.conf.in freewb.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/freewb.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")