project( Freewb )
cmake_minimum_required( VERSION 3.1.0 )


#产生配置头文件config.h供源代码编译的时候包含
#configure_file( ${PROJECT_SOURCE_DIR}/config.h.in 
#                ${PROJECT_SOURCE_DIR}/src/config.h )

#编译发布版本
set( CMAKE_BUILD_TYPE Release ) #Debug or Release
set( QT_PATH "/opt/qt5.12.3/aarch64" CACHE PATH "qt5 cmake dir")

set( QT_VERSION 5.12.3)
set( CMAKE_PREFIX_PATH ${QT_PATH})
#set( CMAKE_INSTALL_RPATH "/usr/share/freewb/lib")
#set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

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
if ( "$ENV{DESKTOP_SESSION}" STREQUAL "mate" )
set( EXTERN_LIBS -ldl -lX11 -lXext -lpthread ${PROJECT_SOURCE_DIR}/../../lib/libfreewbConversionTool.so /usr/lib/aarch64-linux-gnu/libXtst.so.6.1.0 ) #银河麒麟系统
else()
set( EXTERN_LIBS -ldl -lX11 -lXtst -lXext -lpthread ${PROJECT_SOURCE_DIR}/../../lib/libfreewbConversionTool.so ) #Ubuntu以及其他Linux
endif()



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

#可执行文件输出目录
#set( EXECUTABLE_OUTPUT_PATH /usr/bin )

#安装路径
set( INSTALL_ROOT_DIR /usr/share/freewb )
#安装可执行文件
install( TARGETS ${PROJECT_NAME} DESTINATION /usr/bin )
#安装相关依赖目录
install( FILES ${PROJECT_SOURCE_DIR}/fcitx-freewb-panel.desktop 
         DESTINATION /etc/xdg/autostart )
install( FILES ${PROJECT_SOURCE_DIR}/72freewb 
         DESTINATION /etc/X11/Xsession.d )
install( FILES ${PROJECT_SOURCE_DIR}/freewb.sh 
         DESTINATION /usr/bin
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ  GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
install( DIRECTORY ${PROJECT_SOURCE_DIR}/skin 
         DESTINATION ${INSTALL_ROOT_DIR} )
install( DIRECTORY ${PROJECT_SOURCE_DIR}/sound 
         DESTINATION ${INSTALL_ROOT_DIR} )
install( DIRECTORY ${PROJECT_SOURCE_DIR}/data 
         DESTINATION ${INSTALL_ROOT_DIR} )
install( DIRECTORY ${PROJECT_SOURCE_DIR}/config 
         DESTINATION ${INSTALL_ROOT_DIR} )

#查看变量值
#message( ${Xtst_FOUND} ) 

