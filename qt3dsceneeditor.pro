TEMPLATE = app

QT += qml quick widgets 3dcore 3drender 3dextras 3dinput 3dlogic 3dquick 3drender-private

SOURCES += src/main.cpp \
    src/editorviewportitem.cpp \
    src/editorscene.cpp \
    src/editorutils.cpp \
    src/editorsceneitemmodel.cpp \
    src/editorsceneitem.cpp \
    src/editorsceneitemcomponentsmodel.cpp \
    src/editorsceneparser.cpp \
    src/components/transformcomponentproxyitem.cpp \
    src/components/editorsceneitemtransformcomponentsmodel.cpp \
    src/components/materialcomponentproxyitem.cpp \
    src/components/editorsceneitemmaterialcomponentsmodel.cpp \
    src/components/meshcomponentproxyitem.cpp \
    src/components/editorsceneitemmeshcomponentsmodel.cpp \
    src/components/lightcomponentproxyitem.cpp \
    src/components/editorsceneitemlightcomponentsmodel.cpp \
    src/components/qdummyobjectpicker.cpp \
    src/undohandler/undohandler.cpp \
    src/undohandler/insertentitycommand.cpp \
    src/undohandler/removeentitycommand.cpp \
    src/undohandler/renameentitycommand.cpp \
    src/undohandler/propertychangecommand.cpp \
    src/undohandler/modelrolechangecommand.cpp \
    src/undohandler/replacecomponentcommand.cpp \
    src/undohandler/duplicateentitycommand.cpp \
    src/undohandler/copycamerapropertiescommand.cpp \
    src/undohandler/genericpropertychangecommand.cpp \
    src/undohandler/reparententitycommand.cpp \
    src/undohandler/importentitycommand.cpp \
    src/undohandler/resetentitycommand.cpp \
    src/materials/draghandleeffect.cpp \
    src/inputcontrols/editorcameracontroller.cpp

TRANSLATIONS = qt3dsceneeditor_fi.ts \
               qt3dsceneeditor_en.ts

RESOURCES += qml.qrc \
    images.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

INCLUDEPATH += src src/components src/undohandler src/materials src/inputcontrols

HEADERS += \
    src/editorviewportitem.h \
    src/editorscene.h \
    src/editorutils.h \
    src/editorsceneitemmodel.h \
    src/editorsceneitem.h \
    src/editorsceneitemcomponentsmodel.h \
    src/editorsceneparser.h \
    src/components/transformcomponentproxyitem.h \
    src/components/editorsceneitemtransformcomponentsmodel.h \
    src/components/materialcomponentproxyitem.h \
    src/components/editorsceneitemmaterialcomponentsmodel.h \
    src/components/meshcomponentproxyitem.h \
    src/components/editorsceneitemmeshcomponentsmodel.h \
    src/components/lightcomponentproxyitem.h \
    src/components/editorsceneitemlightcomponentsmodel.h \
    src/components/qdummyobjectpicker.h \
    src/undohandler/undohandler.h \
    src/undohandler/insertentitycommand.h \
    src/undohandler/removeentitycommand.h \
    src/undohandler/renameentitycommand.h \
    src/undohandler/propertychangecommand.h \
    src/undohandler/modelrolechangecommand.h \
    src/undohandler/replacecomponentcommand.h \
    src/undohandler/duplicateentitycommand.h \
    src/undohandler/copycamerapropertiescommand.h \
    src/undohandler/genericpropertychangecommand.h \
    src/undohandler/reparententitycommand.h \
    src/undohandler/importentitycommand.h \
    src/undohandler/resetentitycommand.h \
    src/materials/draghandleeffect.h \
    src/inputcontrols/editorcameracontroller.h

lupdate_only {
SOURCES = qml/*.qml \
    qml/inputfields/*.qml \
    qml/lights/*.qml \
    qml/materials/*.qml \
    qml/meshes/*.qml \
    qml/transforms/*.qml \
    src/editorscene.cpp \
    src/editorsceneitemmodel.cpp \
    src/editorutils.cpp
}
