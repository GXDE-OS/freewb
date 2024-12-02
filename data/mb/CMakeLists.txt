set(TABLE_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/default/freeime.mb"
  "${CMAKE_CURRENT_SOURCE_DIR}/default/attach.mb"
  )

# 字库码表文件安装,当前目录需存在freeime.mb和attach.mb两个字库码表,极点五笔的码表可以通过tools目录下的工具和字库txt文件来生成
install(FILES ${TABLE_DATA} DESTINATION ${CMAKE_INSTALL_PREFIX}/freewb/data/mb/default )
