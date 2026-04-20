#CMAKE编译标志
add_definitions(-DBUILD_IN_CAMKE)

include_directories(${PROJECT_SOURCE_DIR}/src/panel/src)

#Qt工程需要用到的编译开关
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
 
#匹配相关源文件与资源文件
file(GLOB SRC_FILES "src/*.cpp" "src/fakekey/*.c" "src/sqlite3/*.c" "src/aes/*.c")
file(GLOB UI_FILES "src/*.ui")
file(GLOB QRC_FILES "src/*.qrc")

#寻找Qt依赖库
find_package(QT NAMES Qt5 REQUIRED)
find_package(Qt5 ${QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets Multimedia DBus)
if (NOT Qt${QT_VERSION_MAJOR}_FOUND)
    message(FATAL_ERROR "Qt${QT_VERSION_MAJOR} not found!")
    return()
endif()

#包含头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/src/../tools
    ${PROJECT_SOURCE_DIR}/src/../utils
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/fakekey
    ${PROJECT_BINARY_DIR} )


#链接外部动态库
set(EXTERN_LIBS -ldl -lX11 -lXtst -lXext -lpthread freewbConversionTool)

#编译
add_executable(${PROJECT_NAME}-panel ${SRC_FILES} ${UI_FILES} ${QRC_FILES})

#链接库
target_link_libraries(${PROJECT_NAME}-panel PRIVATE
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::DBus
    ${EXTERN_LIBS}
    freewb-utils
    )


#安装可执行文件
install(TARGETS ${PROJECT_NAME}-panel DESTINATION ${FREEWB_INSTALL_BINDIR})
