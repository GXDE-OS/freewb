message(STATUS "building utils module")

include_directories("${CMAKE_SOURCE_DIR}/include")
add_library(${PROJECT_NAME}-utils SHARED
    ConvertUTF.c
    settings.cpp
    key.cpp
    utils.cpp
    sound.cpp
    log.cpp
    sqlite3.c
)

target_include_directories(${PROJECT_NAME}-utils PUBLIC ${CMAKE_SOURCE_DIR}/src/utils)

target_link_libraries(${PROJECT_NAME}-utils PUBLIC ${PROJECT_SOURCE_DIR}/libs/${CMAKE_SYSTEM_NAME}/${ARCH_TYPE}/libspdlog.a)

find_package(Threads REQUIRED)
if (THREADS_FOUND)
    target_link_libraries(${PROJECT_NAME}-utils PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
endif()


install(TARGETS ${PROJECT_NAME}-utils LIBRARY DESTINATION ${FREEWB_INSTALL_LIBDIR})
