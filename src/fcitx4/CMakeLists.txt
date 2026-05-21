message(STATUS "Building fcitx4 module")

add_definitions(-D__FCITX4__)

find_package(Fcitx 4.2.7 REQUIRED)
find_package(DBus1 REQUIRED)

if (NOT FCITX_FOUND)
  message(WARNING "fcitx4 is not found")
  return()
endif()

include_directories(${FCITX4_FCITX_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_UTILS_INCLUDE_DIRS})
message(STATUS "fcitx4 is found, libraries is: ${FCITX4_FCITX_LIBRARIES}")

fcitx_add_addon(lib${PROJECT_NAME}-fcitx4 SHARED module.cpp)

target_link_libraries(lib${PROJECT_NAME}-fcitx4
    ${PROJECT_NAME}-ipc
    ${PROJECT_NAME}-core
    ${PROJECT_NAME}-utils
    ${FCITX4_FCITX_LIBRARIES}
    ${DBus1_LIBRARIES}
)

target_include_directories(lib${PROJECT_NAME}-fcitx4 PRIVATE
  ${DBus1_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/utils
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/ipc
)

configure_file(freewb.conf.in ${CMAKE_CURRENT_BINARY_DIR}/freewb.conf IMMEDIATE @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/freewb.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/fcitx/inputmethod)

configure_file(fcitx-freewb.conf.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx-freewb.conf IMMEDIATE @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-freewb.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/fcitx/addon)
