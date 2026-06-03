message(STATUS "Building core module")

add_library(${PROJECT_NAME}-core SHARED
    freewb.cpp
    candidatelist.cpp
    enginemanager.cpp
    committer.cpp
    chttrans.cpp
    punc.cpp
    special.cpp
    statemanager.cpp
)
target_include_directories(${PROJECT_NAME}-core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/ipc
    ${CMAKE_SOURCE_DIR}/src/engine)
target_link_libraries(${PROJECT_NAME}-core PUBLIC ${PROJECT_NAME}-utils ${PROJECT_NAME}-ipc ${PROJECT_NAME}-engine-wbzx ${PROJECT_NAME}-engine-py ${PROJECT_NAME}-engine-wbpy ${PROJECT_NAME}-engine-en)

target_link_libraries(${PROJECT_NAME}-core PUBLIC ${PROJECT_SOURCE_DIR}/libs/${CMAKE_SYSTEM_NAME}/${ARCH_TYPE}/libopencc.so.1.3.2)

install(TARGETS ${PROJECT_NAME}-core DESTINATION ${FREEWB_INSTALL_LIBDIR})

install(FILES ${PROJECT_SOURCE_DIR}/libs/${CMAKE_SYSTEM_NAME}/${ARCH_TYPE}/libopencc.so DESTINATION ${FREEWB_INSTALL_LIBDIR})
install(FILES ${PROJECT_SOURCE_DIR}/libs/${CMAKE_SYSTEM_NAME}/${ARCH_TYPE}/libopencc.so.1.3 DESTINATION ${FREEWB_INSTALL_LIBDIR})
install(FILES ${PROJECT_SOURCE_DIR}/libs/${CMAKE_SYSTEM_NAME}/${ARCH_TYPE}/libopencc.so.1.3.2 DESTINATION ${FREEWB_INSTALL_LIBDIR})

install(FILES ${PROJECT_SOURCE_DIR}/libs/${CMAKE_SYSTEM_NAME}/${ARCH_TYPE}/STPhrases.ocd2 DESTINATION ${FREEWB_INSTALL_PKGDATADIR}/data/s2t)
install(FILES ${PROJECT_SOURCE_DIR}/libs/${CMAKE_SYSTEM_NAME}/${ARCH_TYPE}/STCharacters.ocd2 DESTINATION ${FREEWB_INSTALL_PKGDATADIR}/data/s2t)
