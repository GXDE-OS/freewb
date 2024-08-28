cmake_minimum_required(VERSION 2.6)
# set(txt2mb_SOURCES
#   txt2mb.c
#   )
# 
# add_executable(mtxt2mb ${txt2mb_SOURCES})
# 
# target_link_libraries(mtxt2mb fcitx-config fcitx-utils)
# 
# install(TARGETS mtxt2mb DESTINATION ${PROJECT_SOURCE_DIR}/src/tools )

 set(CONVERSION_SOURCES
   freewbConversionTool.c
   )
 
 # add_executable(freewbConversionTool ${CONVERSION_SOURCES})
 add_library(freewbConversionTool SHARED ${CONVERSION_SOURCES})
 set_target_properties(freewbConversionTool PROPERTIES VERSION 1.0 SOVERSION 1)
 target_link_libraries(freewbConversionTool fcitx-config fcitx-utils)
 SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)  
 # install(TARGETS freewbConversionTool LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/lib )
 install(FILES freewbConversionTool.h DESTINATION ${PROJECT_SOURCE_DIR}/include)
 
#  add_executable(freeS2T trans.c)
# # add_library(hello SHARED hello.c)
# # set_target_properties(hello PROPERTIES VERSION 1.0 SOVERSION 1)
#  target_link_libraries(freeS2T fcitx-utils)
# # 
# install(TARGETS freeS2T DESTINATION ${PROJECT_SOURCE_DIR}/src/tools )

#set(TEST_SOURCES
	#  test.c
  #  )
#
#add_executable(mtest ${TEST_SOURCES})
#include_directories(${PROJECT_SOURCE_DIR}/include)
#target_link_libraries(mtest ${PROJECT_SOURCE_DIR}/lib/libfreewbConversionTool.so)
#
#install(TARGETS mtest DESTINATION ${PROJECT_SOURCE_DIR}/test )
