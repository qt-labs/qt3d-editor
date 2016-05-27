TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += editorlib standalone
CHECK_CREATOR_ENV = $$(QTC_BUILD)$$(QTC_SOURCE)
!isEmpty(CHECK_CREATOR_ENV) {
    SUBDIRS += creatorplugin
}
