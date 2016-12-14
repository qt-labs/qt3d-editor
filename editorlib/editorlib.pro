TEMPLATE = lib

TARGET = qt3dsceneeditor

DEFINES += QT3D_SCENE_EDITOR_LIBRARY

QT += qml quick widgets 3dcore 3drender 3dextras 3dinput 3dlogic 3dquick 3drender-private

SOURCES += src/qt3dsceneeditor.cpp \
    src/editorviewportitem.cpp \
    src/editorscene.cpp \
    src/editorutils.cpp \
    src/editorsceneitemmodel.cpp \
    src/editorsceneitem.cpp \
    src/editorsceneitemcomponentsmodel.cpp \
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
    src/undohandler/pasteentitycommand.cpp \
    src/undohandler/copycamerapropertiescommand.cpp \
    src/undohandler/genericpropertychangecommand.cpp \
    src/undohandler/reparententitycommand.cpp \
    src/undohandler/importentitycommand.cpp \
    src/undohandler/resetentitycommand.cpp \
    src/undohandler/resettransformcommand.cpp \
    src/inputcontrols/editorcameracontroller.cpp \
    src/materials/ontopeffect.cpp

TRANSLATIONS = editorlib_fi.ts \
               editorlib_en.ts

RESOURCES += qml.qrc \
    images.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

INCLUDEPATH += src src/components src/undohandler src/materials src/inputcontrols

HEADERS += \
    src/qt3dsceneeditor.h \
    src/editorviewportitem.h \
    src/editorscene.h \
    src/editorutils.h \
    src/editorsceneitemmodel.h \
    src/editorsceneitem.h \
    src/editorsceneitemcomponentsmodel.h \
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
    src/undohandler/pasteentitycommand.h \
    src/undohandler/copycamerapropertiescommand.h \
    src/undohandler/genericpropertychangecommand.h \
    src/undohandler/reparententitycommand.h \
    src/undohandler/importentitycommand.h \
    src/undohandler/resetentitycommand.h \
    src/undohandler/resettransformcommand.h \
    src/inputcontrols/editorcameracontroller.h \
    src/materials/ontopeffect.h

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

greaterThan(QT_MAJOR_VERSION, 4):greaterThan(QT_MINOR_VERSION, 8) {
    DEFINES += GLTF_EXPORTER_AVAILABLE
    SOURCES += src/editorscenesaver.cpp
    HEADERS += src/editorscenesaver.h
} else {
    SOURCES += src/editorsceneparser.cpp
    HEADERS += src/editorsceneparser.h
}
