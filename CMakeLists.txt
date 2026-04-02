# This is a template of a simple template of a fcitx module with name "example"
# Please remember to replace all "[Ee]xample"'s in the files and file names
# to your addon's real name.
cmake_minimum_required(VERSION 2.6)

project(freewb)
string(TIMESTAMP FREEWB_VERSION "v4.0 %Y.%m.%d" UTC)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(OS_TYPE "Linux")
    message(STATUS "Detected operating system is : ${OS_TYPE}")
    add_definitions(-D__LINUX__)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(OS_TYPE "Windows")
    message(STATUS "Detected operating system is : ${OS_TYPE}")
else()
    message(STATUS "Unknown operating system.")
endif()

option(ENABLE_FCITX4 "enable support fcitx4" On)
option(ENABLE_FCITX5 "enable support fcitx5" On)

include(FindPkgConfig)
include(GNUInstallDirs)

set(FREEWB_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(FREEWB_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_FULL_INCLUDEDIR})
set(FREEWB_INSTALL_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR}/freewb)
set(FREEWB_INSTALL_DATADIR ${CMAKE_INSTALL_FULL_DATADIR})
set(FREEWB_INSTALL_PKGDATADIR ${CMAKE_INSTALL_FULL_DATADIR}/freewb)
set(FREEWB_INSTALL_BINDIR ${CMAKE_INSTALL_FULL_BINDIR})

message(STATUS "FREEWB_INSTALL_PREFIX: ${FREEWB_INSTALL_PREFIX}")
message(STATUS "FREEWB_INSTALL_INCLUDEDIR: ${FREEWB_INSTALL_INCLUDEDIR}")
message(STATUS "FREEWB_INSTALL_LIBDIR: ${FREEWB_INSTALL_LIBDIR}")
message(STATUS "FREEWB_INSTALL_DATADIR: ${FREEWB_INSTALL_DATADIR}")
message(STATUS "FREEWB_INSTALL_PKGDATADIR: ${FREEWB_INSTALL_PKGDATADIR}")
message(STATUS "FREEWB_INSTALL_BINDIR: ${FREEWB_INSTALL_BINDIR}")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_package(Libintl REQUIRED)
find_package(Libiconv REQUIRED)

include_directories(${LIBINTL_INCLUDE_DIR})
include_directories(${LIBICONV_INCLUDE_DIR})
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_BINARY_DIR}/src")

if(ENABLE_FCITX4)
    find_package(Fcitx 4.2.7 REQUIRED)
    include_directories(${FCITX4_FCITX_INCLUDE_DIRS})
    include_directories(${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
    include_directories(${FCITX4_FCITX_UTILS_INCLUDE_DIRS})
    # This will add a uninstall target. Comment out this line if other cmake packages
    # you are using already provides such target (e.g. KDE).
    _fcitx_add_uninstall_target()
    add_definitions(-D__FCITX4__)
    message(STATUS "fcitx install prefix dir is:" ${FCITX4_PREFIX})
endif()
 
configure_file(config.h.in ${CMAKE_SOURCE_DIR}/include/config.h IMMEDIATE @ONLY)

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(help)
add_subdirectory(resources)