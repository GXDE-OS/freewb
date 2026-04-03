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
option(ENABLE_TEST "build freewb-test" Off)

find_package(ECM REQUIRED 1.0.0)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
include(FindPkgConfig)
include(GNUInstallDirs)
include(ECMUninstallTarget)

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

find_package(Libintl REQUIRED)
find_package(Libiconv REQUIRED)

include_directories(${LIBINTL_INCLUDE_DIR})
include_directories(${LIBICONV_INCLUDE_DIR})
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_BINARY_DIR}/src")
 
configure_file(config.h.in ${CMAKE_SOURCE_DIR}/include/config.h IMMEDIATE @ONLY)

add_subdirectory(src)

if(ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

add_subdirectory(data)
add_subdirectory(help)
add_subdirectory(resources)