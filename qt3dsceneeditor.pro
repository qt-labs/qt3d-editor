TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += editorlib standalone

CREATOR_BUILD = $$(QTC_BUILD)
CREATOR_SOURCE = $$(QTC_SOURCE)
if(isEmpty(CREATOR_BUILD) | isEmpty(CREATOR_SOURCE)) {
    warning(Missing Qt Creator source or build tree. Skipping plugin. Set QTC_BUILD and QTC_SOURCE if you want to build qt creator plugin.)
} else {
    SUBDIRS += creatorplugin
}
