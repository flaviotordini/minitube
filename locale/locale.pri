# This voodoo comes from the Arora project

INCLUDEPATH += $$PWD
VPATH += $$PWD

# ls -1 *.ts | tr '\n' ' '
TRANSLATIONS += ar.ts ast.ts be.ts bg_BG.ts ca.ts ca_ES.ts cs_CZ.ts da.ts de_DE.ts el.ts en.ts en_GB.ts es.ts es_AR.ts es_ES.ts es_MX.ts fi.ts fi_FI.ts fr.ts gl.ts he_IL.ts hr.ts hu.ts id.ts it.ts ja_JP.ts ko_KR.ts ky.ts ms_MY.ts nb.ts nl.ts nn.ts pl.ts pl_PL.ts pt.ts pt_BR.ts pt_PT.ts ro.ts ru.ts sk.ts sl.ts sq.ts sr.ts sv_SE.ts th.ts tr.ts uk.ts uk_UA.ts vi.ts zh_CN.ts zh_TW.ts 
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
