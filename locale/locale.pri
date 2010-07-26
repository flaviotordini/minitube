# This voodoo comes from the Arora project

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

TRANSLATIONS += \
    it_IT.ts \
    pt_BR.ts \
    ru_RU.ts \
    pl_PL.ts \
    de_DE.ts \
    ja_JP.ts \
    cs_CZ.ts \
    uk.ts \
    he_IL.ts \
    lat.ts \
    hr_HR.ts \
    es.ts \
    gl.ts \
    fr_FR.ts \
    hu_HU.ts \
    tr_TR.ts \
    nb_NO.ts \
    ro_RO.ts \
    el_GR.ts \
    nl_NL.ts \
    ar.ts \
    pt_PT.ts \
    fi_FI.ts \
    bg_BG.ts \
    zh_CN.ts

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

updateqm.input = TRANSLATIONS
updateqm.output = build/target/locale/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm build/target/locale/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

#qmfiles.files = TRANSLATIONS
#qmfiles.path = Content/Resources
#QMAKE_BUNDLE_DATA += qmfiles
