# This is a template of a simple template of a fcitx module with name "example"
# Please remember to replace all "[Ee]xample"'s in the files and file names
# to your addon's real name.
cmake_minimum_required(VERSION 2.6)

project(fcitx-freewubi)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(OS_TYPE "Linux")
    message(STATUS "Detected operating system is : ${OS_TYPE}")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(OS_TYPE "Windows")
    message(STATUS "Detected operating system is : ${OS_TYPE}")
else()
    message(STATUS "Unknown operating system.")
endif()

include(FindPkgConfig)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_package(Fcitx 4.2.7 REQUIRED)
find_package(Libintl REQUIRED)
find_package(Libiconv REQUIRED)
find_package(DBus1 REQUIRED)
# This will add a uninstall target. Comment out this line if other cmake packages
# you are using already provides such target (e.g. KDE).

_fcitx_add_uninstall_target()
include_directories(${FCITX4_FCITX_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_UTILS_INCLUDE_DIRS})
include_directories(${LIBINTL_INCLUDE_DIR})
include_directories(${LIBICONV_INCLUDE_DIR})
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_BINARY_DIR}/src")
include_directories(${DBus1_INCLUDE_DIRS})
  
link_directories(${DBus1_LIBRARY_DIRS})

message(STATUS "fcitx install prefix dir is:" ${FCITX4_PREFIX})

add_definitions("-DLOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/share/locale\"")

install( DIRECTORY ${PROJECT_SOURCE_DIR}/help
         DESTINATION /usr/share/freewb )
 
configure_file(config.h.in ${CMAKE_SOURCE_DIR}/include/config.h
  IMMEDIATE @ONLY)

# The po dir (and targets, if any) should be added before any other translate
# related sources are processed.
add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(data)