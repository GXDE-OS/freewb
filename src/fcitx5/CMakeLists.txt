message(STATUS "Building fcitx5 module")

add_definitions(-D__FCITX5__)

find_package(Fcitx5Core REQUIRED)
if (NOT Fcitx5Core_FOUND)
    message(FATAL_ERROR "Fcitx5Core not found,will not build fcitx5 im module.")
    return()
endif()

include_directories(${CMAKE_SOURCE_DIR}/src/utils)
include_directories(${CMAKE_SOURCE_DIR}/src/core)
include_directories(${CMAKE_SOURCE_DIR}/src/ipc)

include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" FCITX5_MAJOR_VERSION "${Fcitx5Core_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.([0-9]+)\\.[0-9]+.*" "\\1" FCITX5_MINOR_VERSION "${Fcitx5Core_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" FCITX5_PATCH_VERSION "${Fcitx5Core_VERSION}")
message(NOTICE "FCITX_INSTALL_CMAKECONFIG_DIR: [${FCITX_INSTALL_CMAKECONFIG_DIR}], Fcitx5Core_VERSION: [${Fcitx5Core_VERSION}], FCITX5_MAJOR_VERSION: [${FCITX5_MAJOR_VERSION}], FCITX5_MINOR_VERSION: [${FCITX5_MINOR_VERSION}], FCITX5_PATCH_VERSION: [${FCITX5_PATCH_VERSION}]")

add_library(${PROJECT_NAME}-fcitx5 SHARED module.cpp)

target_link_libraries("${PROJECT_NAME}-fcitx5" Fcitx5::Core ${PROJECT_NAME}-utils ${PROJECT_NAME}-core ${PROJECT_NAME}-ipc pthread)

target_compile_definitions(${PROJECT_NAME}-fcitx5 PRIVATE __FCITX5_MAJOR_VERSION__=${FCITX5_MAJOR_VERSION})
target_compile_definitions(${PROJECT_NAME}-fcitx5 PRIVATE __FCITX5_MINOR_VERSION__=${FCITX5_MINOR_VERSION})
target_compile_definitions(${PROJECT_NAME}-fcitx5 PRIVATE __FCITX5_PATCH_VERSION__=${FCITX5_PATCH_VERSION})

#
# Wayland 协议：编译时用当前系统的 wayland-scanner++ 生成 C++ 绑定（输出到 build 目录）
#
pkg_check_modules(PKG_WAYLAND_CLIENTPP wayland-client++)
if(NOT PKG_WAYLAND_CLIENTPP_FOUND)
  message(STATUS "wayland-client++ not found, fcitx5 module will be built without wayland protocol support.")
else()
  find_file(UKUI_WINDOW_MANAGEMENT_XML
    NAMES ukui-window-management.xml
    PATHS
      /usr/share/wayland-protocols
      ${CMAKE_CURRENT_SOURCE_DIR}
    NO_DEFAULT_PATH
  )
  if(UKUI_WINDOW_MANAGEMENT_XML)
    message(STATUS "ukui-window-management.xml: [${UKUI_WINDOW_MANAGEMENT_XML}]")
  else()
    message(FATAL_ERROR
      "Could not find ukui-window-management.xml\n"
      "Please install ukui-wayland-protocols or place ukui-window-management.xml in ${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  find_file(KDE_OUTPUT_DEVICE_V2_XML
    NAMES kde-output-device-v2.xml
    PATHS
      /usr/share/wayland-protocols
      ${CMAKE_CURRENT_SOURCE_DIR}
    NO_DEFAULT_PATH
  )
  if(KDE_OUTPUT_DEVICE_V2_XML)
    message(STATUS "kde-output-device-v2.xml: [${KDE_OUTPUT_DEVICE_V2_XML}]")
  else()
    message(FATAL_ERROR
      "Could not find kde-output-device-v2.xml\n"
      "Please install kde-wayland-protocols or place kde-output-device-v2.xml in ${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  find_program(WAYLAND_SCANNERPP NAMES wayland-scanner++)
  if(NOT WAYLAND_SCANNERPP)
    message(FATAL_ERROR "wayland-scanner++ not found. Please install wayland-scanner++.")
  endif()
  message(STATUS "wayland-scanner++ FOUND: [${WAYLAND_SCANNERPP}]")

  set(WAYLAND_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
  set(UKUI_WINDOW_MANAGEMENT_H "${WAYLAND_GENERATED_DIR}/ukui-window-management.h")
  set(UKUI_WINDOW_MANAGEMENT_CPP "${WAYLAND_GENERATED_DIR}/ukui-window-management.cpp")
  set(KDE_OUTPUT_DEVICE_V2_H "${WAYLAND_GENERATED_DIR}/kde-output-device-v2.h")
  set(KDE_OUTPUT_DEVICE_V2_CPP "${WAYLAND_GENERATED_DIR}/kde-output-device-v2.cpp")

  add_custom_command(
    OUTPUT ${UKUI_WINDOW_MANAGEMENT_H} ${UKUI_WINDOW_MANAGEMENT_CPP}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${WAYLAND_GENERATED_DIR}
    COMMAND ${WAYLAND_SCANNERPP} ${UKUI_WINDOW_MANAGEMENT_XML} ${UKUI_WINDOW_MANAGEMENT_H} ${UKUI_WINDOW_MANAGEMENT_CPP}
    DEPENDS ${WAYLAND_SCANNERPP} ${UKUI_WINDOW_MANAGEMENT_XML}
    COMMENT "Generating ukui-window-management C++ bindings"
    VERBATIM
  )

  add_custom_command(
    OUTPUT ${KDE_OUTPUT_DEVICE_V2_H} ${KDE_OUTPUT_DEVICE_V2_CPP}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${WAYLAND_GENERATED_DIR}
    COMMAND ${WAYLAND_SCANNERPP} ${KDE_OUTPUT_DEVICE_V2_XML} ${KDE_OUTPUT_DEVICE_V2_H} ${KDE_OUTPUT_DEVICE_V2_CPP}
    DEPENDS ${WAYLAND_SCANNERPP} ${KDE_OUTPUT_DEVICE_V2_XML}
    COMMENT "Generating kde-output-device-v2 C++ bindings"
    VERBATIM
  )

  set_source_files_properties(
    ${UKUI_WINDOW_MANAGEMENT_H} ${UKUI_WINDOW_MANAGEMENT_CPP}
    ${KDE_OUTPUT_DEVICE_V2_H} ${KDE_OUTPUT_DEVICE_V2_CPP}
    PROPERTIES GENERATED TRUE
  )

  target_sources("${PROJECT_NAME}-fcitx5" PRIVATE
    ${UKUI_WINDOW_MANAGEMENT_CPP}
    ${KDE_OUTPUT_DEVICE_V2_CPP}
    ukuiwaylandhelper.cpp
  )
  target_include_directories("${PROJECT_NAME}-fcitx5" PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WAYLAND_GENERATED_DIR}
    ${PKG_WAYLAND_CLIENTPP_INCLUDE_DIRS}
  )
  target_link_libraries("${PROJECT_NAME}-fcitx5" ${PKG_WAYLAND_CLIENTPP_LIBRARIES})
  target_compile_definitions("${PROJECT_NAME}-fcitx5" PRIVATE __HAS_WAYLAND__)
  message(STATUS "wayland protocol support enabled (bindings generated at build time).")
endif()

install(TARGETS "${PROJECT_NAME}-fcitx5" DESTINATION "${FCITX_INSTALL_LIBDIR}/fcitx5")

configure_file(freewb-addon.conf.in freewb-addon.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/freewb-addon.conf" RENAME freewb.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")

configure_file(freewb.conf.in freewb.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/freewb.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")
