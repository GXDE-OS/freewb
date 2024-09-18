set(TABLE_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/default/freeime.mb"
  "${CMAKE_CURRENT_SOURCE_DIR}/default/attach.mb"
  )

#安装路径
# set( INSTALL_ROOT_DIR  $ENV{HOME}/.local/freewb )
#add_custom_target(table_data ALL DEPENDS ${TABLE_DATA})
set( INSTALL_ROOT_DIR  /usr/share/freewb )

# 字库码表文件安装,当前目录需存在freeime.mb和attach.mb两个字库码表,极点五笔的码表可以通过tools目录下的工具和字库txt文件来生成
install(FILES ${TABLE_DATA} DESTINATION ${INSTALL_ROOT_DIR}/data/mb/default )
