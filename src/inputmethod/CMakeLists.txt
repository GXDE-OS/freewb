# Source files
set(fcitx_freewubi_sources
  freewubi-config.c
  freewubi.c
  ini.c
  freedict.c
  freespecial.c
  freeinterface.c
  virtual_keyboard.c
  )
set(fcitx_freewubi_headers
    ini.h
    freewubi-config.h
    freewubi.h
    freewubi-internal.h
    freedict.h
    utf8_in_gb18030.h
    freespecial.h
    freeinterface.h
    virtual_keyboard.h
    )

set(ENABLE_FREEWUBI TRUE)
if(ENABLE_FREEWUBI)
  set(table_noinstall)
else()
  set(table_noinstall NO_INSTALL)
endif()

fcitx_add_addon_full(freewubi DESC ${table_noinstall}
  SOURCES ${fcitx_freewubi_sources}
  EXTRA_PO ${fcitx_freewubi_headers}
  IM_CONFIG data/freewubi.conf
  LINK_LIBS ${LIBINTL_LIBRARIES}
  ${LIBINTL_LIBRARIES}
  ${DBus1_LIBRARIES})

