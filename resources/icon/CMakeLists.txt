if (ENABLE_FCITX4)
    install(FILES freewb.svg DESTINATION ${FREEWB_INSTALL_DATADIR}/fcitx/imicon)
    install(FILES freewb.svg DESTINATION RENAME fcitx-freewb.svg ${FREEWB_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
endif()

install(FILES freewb.svg DESTINATION ${FREEWB_INSTALL_DATADIR}/icons/hicolor/scalable/apps)