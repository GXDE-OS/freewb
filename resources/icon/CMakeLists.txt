if (ENABLE_FCITX4)
    install(FILES freewb.svg DESTINATION ${FREEWB_INSTALL_DATADIR}/fcitx/imicon)
    install(FILES freewb.svg RENAME fcitx-freewb.svg DESTINATION ${FREEWB_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
endif()

if (ENABLE_UKUI)
    install(FILES freewb.svg RENAME fcitx-freewb.svg DESTINATION ${FREEWB_INSTALL_DATADIR}/icons/ukui-icon-theme-default/scalable/apps)
endif()

install(FILES freewb.svg DESTINATION ${FREEWB_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
