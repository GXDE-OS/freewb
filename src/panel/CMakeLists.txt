project( Freewb )
cmake_minimum_required( VERSION 3.1.0 )

#CMAKE编译标志
add_definitions( -DBUILD_IN_CAMKE )

#Qt工程需要用到的编译开关
set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )
 
#匹配相关源文件与资源文件
file( GLOB SRC_FILES "src/*.cpp" "src/fakekey/*.c" "src/sqlite3/*.c" "src/aes/*.c")
file( GLOB UI_FILES "src/*.ui" )
file( GLOB QRC_FILES "src/*.qrc" )

#寻找Qt依赖库
find_package( Qt5Core ${QT_VERSION} REQUIRED )
find_package( Qt5Gui ${QT_VERSION} REQUIRED )
find_package( Qt5Widgets ${QT_VERSION} REQUIRED )
find_package( Qt5Multimedia ${QT_VERSION} REQUIRED )
find_package( Qt5DBus ${QT_VERSION} REQUIRED )


#包含头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/src/../tools
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/fakekey  
    ${PROJECT_BINARY_DIR}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Multimedia_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS} )


#链接外部动态库
set( EXTERN_LIBS -ldl -lX11 -lXtst -lXext -lpthread freewbConversionTool )

#编译
add_executable( ${PROJECT_NAME} ${SRC_FILES} ${UI_FILES} ${QRC_FILES} )

#链接库
target_link_libraries( ${PROJECT_NAME} 
                        ${Qt5Core_LIBRARIES} 
                        ${Qt5Gui_LIBRARIES} 
                        ${Qt5Widgets_LIBRARIES} 
                        ${Qt5Multimedia_LIBRARIES}
                        ${Qt5DBus_LIBRARIES}
                        ${EXTERN_LIBS} )

#安装可执行文件
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
#安装相关依赖目录
install(FILES ${PROJECT_SOURCE_DIR}/fcitx-freewb-panel.desktop DESTINATION /etc/xdg/autostart )

configure_file(freewb.sh.in ${CMAKE_CURRENT_BINARY_DIR}/freewb.sh IMMEDIATE @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/freewb.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ  GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )

install(DIRECTORY ${PROJECT_SOURCE_DIR}/skin DESTINATION ${CMAKE_INSTALL_PREFIX}/share/freewb)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/sound DESTINATION ${CMAKE_INSTALL_PREFIX}/share/freewb)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data DESTINATION ${CMAKE_INSTALL_PREFIX}/share/freewb)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/config DESTINATION ${CMAKE_INSTALL_PREFIX}/share/freewb)

