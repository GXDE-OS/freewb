set(TABLE_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/gbks2t.tab"
  "${CMAKE_CURRENT_SOURCE_DIR}/phrase.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/single.txt"  
  )

install(FILES ${TABLE_DATA} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/freewb/data/s2t )
