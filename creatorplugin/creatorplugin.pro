DEFINES += QT3DSCENEEDITOR_LIBRARY

QT += qml quick quickwidgets

DEPENDPATH += ../editorlib
INCLUDEPATH += ../editorlib

# Qt3DSceneEditor files

SOURCES += qt3dsceneeditorplugin.cpp \
        qt3dsceneeditorwidget.cpp \
        qt3dsceneeditorcontext.cpp

HEADERS += qt3dsceneeditorplugin.h \
        qt3dsceneeditor_global.h \
        qt3dsceneeditorconstants.h \
        qt3dsceneeditorwidget.h \
        qt3dsceneeditorcontext.h

RESOURCES += creatorplugin.qrc

# Qt Creator linking

## Either set the IDE_SOURCE_TREE when running qmake,
## or set the QTC_SOURCE environment variable, to override the default setting
isEmpty(IDE_SOURCE_TREE): IDE_SOURCE_TREE = $$(QTC_SOURCE)
isEmpty(IDE_SOURCE_TREE): IDE_SOURCE_TREE = "/dev/qt/qt-creator"

## Either set the IDE_BUILD_TREE when running qmake,
## or set the QTC_BUILD environment variable, to override the default setting
isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE = "/dev/qt/build-qtcreator"

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on OS X
# USE_USER_DESTDIR = yes

###### If the plugin can be depended upon by other plugins, this code needs to be outsourced to
###### <dirname>_dependencies.pri, where <dirname> is the name of the directory containing the
###### plugin's sources.

QTC_PLUGIN_NAME = Qt3DSceneEditor
QTC_LIB_DEPENDS += \
    # nothing here at this time

QTC_PLUGIN_DEPENDS += \
    coreplugin \
    projectexplorer \
    qmakeprojectmanager

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######

include($$IDE_SOURCE_TREE/src/qtcreatorplugin.pri)

# Figure out the scene editor library name and path
win32 {
    EDITORLIBNAME = qt3dsceneeditor.dll
    CONFIG (release, debug|release): EDITORLIBDIR = $$OUT_PWD/../editorlib/release
    else:CONFIG (debug, debug|release): EDITORLIBDIR = $$OUT_PWD/../editorlib/debug
} else {
    EDITORLIBNAME = qt3dsceneeditor.so
    EDITORLIBDIR = $$OUT_PWD/../editorlib
}

LIBS += -L$$EDITORLIBDIR -lqt3dsceneeditor

# Install necessary files under creator
editordll.path = $$IDE_BIN_PATH
editordll.files = $$EDITORLIBDIR/$$EDITORLIBNAME
editordll.CONFIG = no_check_exist

wizardfiles.path = $$IDE_DATA_PATH/templates/wizards/files/qt3dsceneeditor
wizardfiles.files = \
    tocreator/qt3dsceneeditor/wizard.json \
    tocreator/qt3dsceneeditor/GeneratedScene.qt3d.qrc

wizardresfiles.path = $$IDE_DATA_PATH/templates/wizards/files/qt3dsceneeditor/GeneratedScene_scene_res
wizardresfiles.files = \
    tocreator/qt3dsceneeditor/GeneratedScene_scene_res/GeneratedScene.qml \
    tocreator/qt3dsceneeditor/GeneratedScene_scene_res/r0_qtlogo.png

INSTALLS += wizardfiles wizardresfiles editordll
