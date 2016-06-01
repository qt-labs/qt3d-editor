TEMPLATE = app

TARGET = qt3dsceneeditor

QT += qml quick widgets

DEPENDPATH += ../editorlib
INCLUDEPATH += ../editorlib

win32:CONFIG (release, debug|release): LIBS += -L$$OUT_PWD/../editorlib/release -lqt3dsceneeditor
else:win32:CONFIG (debug, debug|release): LIBS += -L$$OUT_PWD/../editorlib/debug -lqt3dsceneeditor
else: LIBS += -L$$OUT_PWD/../editorlib -lqt3dsceneeditor

CONFIG += c++11

SOURCES += main.cpp

# Default rules for deployment.
include(deployment.pri)
