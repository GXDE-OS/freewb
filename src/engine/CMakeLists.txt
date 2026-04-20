message(STATUS "Building engine module")

add_library(${PROJECT_NAME}-engine-wbzx SHARED wbzx.cpp userdict.cpp)
target_include_directories(${PROJECT_NAME}-engine-wbzx PUBLIC ${CMAKE_SOURCE_DIR}/src/engine)
target_link_libraries(${PROJECT_NAME}-engine-wbzx PUBLIC ${PROJECT_NAME}-utils)
install(TARGETS ${PROJECT_NAME}-engine-wbzx DESTINATION ${FREEWB_INSTALL_LIBDIR})

add_library(${PROJECT_NAME}-engine-py SHARED py.cpp userdict.cpp)
target_include_directories(${PROJECT_NAME}-engine-py PUBLIC ${CMAKE_SOURCE_DIR}/src/engine)
target_link_libraries(${PROJECT_NAME}-engine-py PUBLIC ${PROJECT_NAME}-utils)
install(TARGETS ${PROJECT_NAME}-engine-py DESTINATION ${FREEWB_INSTALL_LIBDIR})

add_library(${PROJECT_NAME}-engine-en SHARED en.cpp)
target_include_directories(${PROJECT_NAME}-engine-en PUBLIC ${CMAKE_SOURCE_DIR}/src/engine)
target_link_libraries(${PROJECT_NAME}-engine-en PUBLIC ${PROJECT_NAME}-utils)
install(TARGETS ${PROJECT_NAME}-engine-en DESTINATION ${FREEWB_INSTALL_LIBDIR})

add_library(${PROJECT_NAME}-engine-wbpy SHARED wbpy.cpp)
target_include_directories(${PROJECT_NAME}-engine-wbpy PUBLIC ${CMAKE_SOURCE_DIR}/src/engine)
target_link_libraries(${PROJECT_NAME}-engine-wbpy PUBLIC ${PROJECT_NAME}-engine-wbzx ${PROJECT_NAME}-engine-py)
install(TARGETS ${PROJECT_NAME}-engine-wbpy DESTINATION ${FREEWB_INSTALL_LIBDIR})