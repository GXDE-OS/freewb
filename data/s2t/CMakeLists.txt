set(TABLE_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/gbks2t.tab"
  "${CMAKE_CURRENT_SOURCE_DIR}/phrase.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/single.txt"  
  )

#安装路径
# set( INSTALL_ROOT_DIR  $ENV{HOME}/.local/freewb )
#add_custom_target(table_data ALL DEPENDS ${TABLE_DATA})
set( INSTALL_ROOT_DIR  /usr/share/freewb )

install(FILES ${TABLE_DATA} DESTINATION ${INSTALL_ROOT_DIR}/data/s2t )
