# This voodoo comes from the Arora project

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# ls -1 *.ts | tr '\n' ' '
TRANSLATIONS += ar.ts ca.ts ca_ES.ts cs_CZ.ts da.ts de_DE.ts el.ts el_GR.ts es.ts es_AR.ts es_ES.ts fi_FI.ts fr.ts gl.ts he_IL.ts hr.ts hu.ts hu_HU.ts ia.ts id.ts id_ID.ts it.ts jv.ts lv.ts nb.ts nl.ts pl.ts pl_PL.ts pt.ts pt_BR.ts ro.ts ru.ts sl.ts sq.ts sr.ts sv_SE.ts te.ts tr.ts uk.ts uk_UA.ts zh_CN.ts 
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
