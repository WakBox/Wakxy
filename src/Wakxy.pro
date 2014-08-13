#-------------------------------------------------
#
# Project created by QtCreator 2013-09-30T11:37:36
#
#-------------------------------------------------

QT       += core gui network script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Wakxy
TEMPLATE = app
RC_FILE = Ressources/icon.rc

INCLUDEPATH += . PacketReader/ ScriptEditor/ Utils/
DEPENDPATH += . PacketReader/ ScriptEditor/ Utils/

SOURCES += main.cpp\
        mainwindow.cpp \
    PacketReader/packetreader.cpp \
    Utils/utils.cpp \
    ScriptEditor/dialogpacket.cpp \
    ScriptEditor/highlighter.cpp \
    ScriptEditor/completer.cpp

HEADERS  += mainwindow.h \
    PacketReader/packetreader.h \
    Utils/utils.h \
    ScriptEditor/dialogpacket.h \
    ScriptEditor/highlighter.h \
    ScriptEditor/completer.h

FORMS    += mainwindow.ui \
    ScriptEditor/dialogpacket.ui

RESOURCES += \
    Ressources/ressources.qrc

TRANSLATIONS = Ressources/Wakxy_fr.ts
