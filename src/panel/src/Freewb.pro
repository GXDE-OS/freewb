#-------------------------------------------------
#
# Project created by QtCreator 2019-05-17T14:38:41
#
#-------------------------------------------------

QT       += core gui dbus multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Freewb
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        backupdialog.cpp \
        candidateitem.cpp \
        contextmenu.cpp \
        customkeydialog.cpp \
        dictquery.cpp \
        dictquerywin.cpp \
        fakekey/fakekey.c \
        inputwin.cpp \
        keyboard.cpp \
        keybutton.cpp \
        kimagent.cpp \
        lexicontoolwin.cpp \
        main.cpp \
        mainprogram.cpp \
        settings.cpp \
        settingwin.cpp \
        sound.cpp \
        sqlite3/sqlite3.c \
        systraymenu.cpp \
        texteditwin.cpp \
        textfinddialog.cpp \
        toolbarwin.cpp \
        usrgenworddialog.cpp \
        wbpy.cpp \
        x11eventmonitor.cpp

HEADERS += \
        backupdialog.h \
        candidateitem.h \
        commdefine.h \
        config.h \
        contextmenu.h \
        customkeydialog.h \
        dictquery.h \
        dictquerywin.h \
        fakekey/fakekey.h \
        inputwin.h \
        keyboard.h \
        keybutton.h \
        kimagent.h \
        lexicontoolwin.h \
        mainprogram.h \
        settings.h \
        settingwin.h \
        sound.h \
        sqlite3/sqlite3.h \
        systraymenu.h \
        texteditwin.h \
        textfinddialog.h \
        toolbarwin.h \
        usrgenworddialog.h \
        x11eventmonitor.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    image.qrc \
    qss.qrc

FORMS += \
    backupdialog.ui \
    candidateitem.ui \
    customkeydialog.ui \
    dictquerywin.ui \
    inputwin.ui \
    keyboard.ui \
    lexicontoolwin.ui \
    settingwin.ui \
    texteditwin.ui \
    textfinddialog.ui \
    toolbarwin.ui \
    usrgenworddialog.ui

QMAKE_CXXFLAGS +=

LIBS += -ldl -lX11 -lXext -lXtst  ../../../lib/libfreewbConversionTool.so





