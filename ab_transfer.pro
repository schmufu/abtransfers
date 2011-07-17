# -------------------------------------------------
# Project created by QtCreator 2011-07-03T18:32:54
# -------------------------------------------------
VERSION = 0.0.1.0 # Version of aqBanking-Transfers
TARGET = ab_transfers
DESTDIR = build
TEMPLATE = app
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/widgets/bankaccountswidget.cpp \
    src/aqb_accountinfo.cpp \
    src/aqb_accounts.cpp \
    src/aqb_banking.cpp \
    src/widgets/debugdialogwidget.cpp \
    src/widgets/ueberweisungswidget.cpp \
    src/pages/page_da_edit_delete.cpp \
    src/widgets/knownempfaengerwidget.cpp \
    src/abt_empfaengerinfo.cpp \
    src/abt_settings.cpp \
    src/abt_transaction_base.cpp \
    src/abt_transactions.cpp \
    src/abt_job_ctrl.cpp \
    src/pages/page_log.cpp \
    src/pages/page_ausgang.cpp \
    src/abt_conv.cpp
HEADERS += src/mainwindow.h \
    src/widgets/bankaccountswidget.h \
    src/aqb_accountinfo.h \
    src/aqb_accounts.h \
    src/aqb_banking.h \
    src/globalvars.h \
    src/widgets/debugdialogwidget.h \
    src/widgets/ueberweisungswidget.h \
    src/pages/page_da_edit_delete.h \
    src/widgets/knownempfaengerwidget.h \
    src/abt_empfaengerinfo.h \
    src/abt_settings.h \
    src/abt_transaction_base.h \
    src/abt_transactions.h \
    src/abt_job_ctrl.h \
    src/pages/page_log.h \
    src/pages/page_ausgang.h \
    src/abt_conv.h
FORMS += src/mainwindow.ui \
    src/widgets/bankaccountswidget.ui \
    src/widgets/debugdialogwidget.ui \
    src/widgets/ueberweisungswidget.ui \
    src/pages/page_da_edit_delete.ui \
    src/widgets/knownempfaengerwidget.ui \
    src/pages/page_log.ui \
    src/pages/page_ausgang.ui
OTHER_FILES += images/uerberweisungsformular.gif \
    documentation/Doxyfile
RESOURCES += src/resources.qrc
INCLUDEPATH += /usr/include/aqbanking5 \
    /usr/include/gwenhywfar4
LIBS += -laqbanking \
    -lgwenhywfar \
    -lgwengui-qt4

# This variable specifies the directory where all intermediate moc files should be placed.
MOC_DIR = tmp

# This variable specifies the directory where all intermediate objects should be placed.
OBJECTS_DIR = tmp
UI_DIR = tmp
SVN_REVISION = $$system(svnversion -n) # current repository revision (without newline)

# revision as define for the Preprocessor ( \\\" so that \" goes to the Preprocessor)
# DEFINES += MVW_SVN_REVISION=\\\"$${SVN_REVISION}\\\" \
# MVW_VERSION=\\\"$${VERSION}\\\" \ # MVW_VERSION_EXTRA=\"\\\"'development-version-test test-test'\\\"\" #damit auch space möglich ist
# MVW_VERSION_EXTRA=\\\"development-version\\\" # keine space möglich!
DEFINES += ABTRANSFER_SVN_REVISION=\\\"$${SVN_REVISION}\\\" \
    ABTRANSFER_VERSION=\\\"$${VERSION}\\\" \
    ABTRANSFER_VERSION_EXTRA=\\\"development-version\\\" # keine space möglich!

# NO_DEBUG
# QT_NO_DEBUG \
# QT_NO_DEBUG_STREAM
# we want to stop the build and output some information (only for debug)
# error(Subversion revision: $$SVN_REVISION)
message(Subversion revision: $$SVN_REVISION)
