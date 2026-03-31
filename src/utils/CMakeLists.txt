add_library(freewb-utils SHARED ConvertUTF.c)

install(TARGETS freewb-utils LIBRARY DESTINATION ${FREEWB_INSTALL_LIBDIR})