# This voodoo comes from the Arora project

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# ls -1 *.ts | tr '\n' ' '
TRANSLATIONS += ar.ts bg.ts bg_BG.ts cs_CZ.ts de_DE.ts el.ts el_GR.ts en_US.ts es.ts es_AR.ts es_ES.ts fi_FI.ts fr_FR.ts gl.ts he_IL.ts hr.ts hr_HR.ts hu_HU.ts id.ts id_ID.ts it_IT.ts ja_JP.ts jv.ts lv.ts nb_NO.ts nl.ts nl_NL.ts pl_PL.ts pt_BR.ts pt_PT.ts ro.ts ro_RO.ts ru_RU.ts sr.ts tr_TR.ts uk.ts zh_CN.ts

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
