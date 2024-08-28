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

set(QT_PATH,"/opt/qt5.12.3/aarch64")

find_package(Libintl REQUIRED)
find_package(Libiconv REQUIRED)
find_package(DBus1 REQUIRED)
# This will add a uninstall target. Comment out this line if other cmake packages
# you are using already provides such target (e.g. KDE).

option(ENABLE_DBUS "Enable DBus Support" ON)
set(_ENABLE_DBUS 1)
# option(ENABLE_OPENCC "Enable OpenCC for Chinese Transform" ON)
# option(FORCE_OPENCC
#   "Enable opencc even if the library is not found at compile time" Off)
# if(ENABLE_OPENCC)
#      find_package(OpenCC REQUIRED)
#      if(NOT (OPENCC_FOUND OR FORCE_OPENCC))
#    	    set(ENABLE_OPENCC Off)
#      endif()
# endif()

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
#include_directories(${PROJECT_SOURCE_DIR}/src/)
message(STATUS "fcitx install prefix dir is:" ${FCITX4_PREFIX})
#message(STATUS "dbus include dir is:" ${DBUS_INCLUDE_DIRS})
# set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_C_FLAGS}")
# set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
# set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_SHARED_LINKER_FLAGS}")
# set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_MODULE_LINKER_FLAGS}")
add_definitions("-DLOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/share/locale\"")

install( DIRECTORY ${PROJECT_SOURCE_DIR}/help 
         DESTINATION /usr/share/freewb )

install( DIRECTORY ${PROJECT_SOURCE_DIR}/lib 
         DESTINATION /usr/share/freewb )

install( DIRECTORY ${QT_PATH}/plugins/platforms
         DESTINATION /usr/share/freewb/plugins )

install( DIRECTORY ${QT_PATH}/plugins/platforminputcontexts
         DESTINATION /usr/share/freewb/plugins )

install( FILES ${QT_PATH}/lib/libicudata.so
	 DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libicudata.so.56
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libicudata.so.56.1
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libicui18n.so
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libicui18n.so.56
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libicui18n.so.56.1
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libicuuc.so
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libicuuc.so.56
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libicuuc.so.56.1
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5Core.so
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5Core.so.5
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5Core.so.5.12
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5Core.so.5.12.3
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5DBus.so
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5DBus.so.5
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5DBus.so.5.12
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5DBus.so.5.12.3
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5Gui.so
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5Gui.so.5
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5Gui.so.5.12
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5Gui.so.5.12.3
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5Widgets.so
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5Widgets.so.5
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5Widgets.so.5.12
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5Widgets.so.5.12.3
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5XcbQpa.so
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5XcbQpa.so.5
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5XcbQpa.so.5.12
         DESTINATION /usr/share/freewb/lib )

install( FILES ${QT_PATH}/lib/libQt5XcbQpa.so.5.12.3
         DESTINATION /usr/share/freewb/lib )
 
configure_file(config.h.in config.h
  IMMEDIATE @ONLY)

# The po dir (and targets, if any) should be added before any other translate
# related sources are processed.
add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(data)
#add_subdirectory(config)
