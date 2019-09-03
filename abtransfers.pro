# -------------------------------------------------
# Project created by QtCreator 2011-07-03T18:32:54
# -------------------------------------------------
VERSION = 0.0.6.1
ABTRANSFER_FINTS_VER = 0.0.6  # max. 5 characters!
!macx:TARGET = abtransfers
macx:TARGET = "AB-Transfers"
DESTDIR = build
TEMPLATE = app

# set to "pkg_config" for the package installed at the system
# or to the path where you installed a local installation
# AQBANKING_INSTALLATION="/opt/aqb"
AQBANKING_INSTALLATION="pkg_config"

contains(AQBANKING_INSTALLATION, "pkg_config") {
    message("Using the AqBanking package from: The system installation")
} else {
    message("Using the AqBanking package from: $${AQBANKING_INSTALLATION}")
}

QT += gui
#check for Qt5 or greater
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets #needed for Qt5
}

USEWEBKIT=$$system(pkg-config QtWebKit && echo "available")
contains(USEWEBKIT, "available") {
    message("QtWebKit available")
    greaterThan(QT_MAJOR_VERSION, 4) {
	QT += webkitwidgets #needed for Qt5
    } else {
	QT += webkit #needed for Qt4
    }

    DEFINES += USE_QT_WEBKIT
} else {
    message("QtWebKit not available, using QLabel")
}

TRANSLATIONS = $$files(translation/abtransfers.*.ts)

SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/widgets/bankaccountswidget.cpp \
    src/aqb_accountinfo.cpp \
    src/aqb_accounts.cpp \
    src/aqb_banking.cpp \
    src/widgets/debugdialogwidget.cpp \
    src/widgets/knownempfaengerwidget.cpp \
    src/abt_empfaengerinfo.cpp \
    src/abt_settings.cpp \
    src/abt_transaction_base.cpp \
    src/abt_transactions.cpp \
    src/abt_job_ctrl.cpp \
    src/pages/page_log.cpp \
    src/pages/page_ausgang.cpp \
    src/abt_conv.cpp \
    src/abt_validators.cpp \
    src/abt_transactionlimits.cpp \
    src/widgets/widgetlineeditwithlabel.cpp \
    src/pages/pagewidgettests.cpp \
    src/widgets/widgetaccountdata.cpp \
    src/widgets/widgettextkey.cpp \
    src/widgets/widgetpurpose.cpp \
    src/widgets/widgetvalue.cpp \
    src/widgets/widgetdate.cpp \
    src/widgets/widgetrecurrence.cpp \
    src/widgets/widgettransfer.cpp \
    src/widgets/widgetknownstandingorders.cpp \
    src/widgets/widgetaccountcombobox.cpp \
    src/widgets/widgetknowndatedtransfers.cpp \
    src/abt_jobinfo.cpp \
    src/abt_parser.cpp \
    src/abt_standingorderinfo.cpp \
    src/abt_datedtransferinfo.cpp \
    src/abt_history.cpp \
    src/dialogs/dialogsettings.cpp \
    src/dialogs/abt_dialog.cpp \
    src/pages/page_history.cpp \
    src/aqb_imexporters.cpp \
    src/translationchooser.cpp \
    src/widgets/widgetcalculator.cpp
HEADERS += src/mainwindow.h \
    src/widgets/bankaccountswidget.h \
    src/aqb_accountinfo.h \
    src/aqb_accounts.h \
    src/aqb_banking.h \
    src/globalvars.h \
    src/widgets/debugdialogwidget.h \
    src/widgets/knownempfaengerwidget.h \
    src/abt_empfaengerinfo.h \
    src/abt_settings.h \
    src/abt_transaction_base.h \
    src/abt_transactions.h \
    src/abt_job_ctrl.h \
    src/pages/page_log.h \
    src/pages/page_ausgang.h \
    src/abt_conv.h \
    src/abt_validators.h \
    src/abt_transactionlimits.h \
    src/widgets/widgetlineeditwithlabel.h \
    src/pages/pagewidgettests.h \
    src/widgets/widgetaccountdata.h \
    src/widgets/widgettextkey.h \
    src/widgets/widgetpurpose.h \
    src/widgets/widgetvalue.h \
    src/widgets/widgetdate.h \
    src/widgets/widgetrecurrence.h \
    src/widgets/widgettransfer.h \
    src/widgets/widgetknownstandingorders.h \
    src/widgets/widgetaccountcombobox.h \
    src/widgets/widgetknowndatedtransfers.h \
    src/abt_jobinfo.h \
    src/abt_parser.h \
    src/abt_standingorderinfo.h \
    src/abt_datedtransferinfo.h \
    src/abt_history.h \
    src/dialogs/dialogsettings.h \
    src/dialogs/abt_dialog.h \
    src/pages/page_history.h \
    src/aqb_imexporters.h \
    src/translationchooser.h \
    src/widgets/widgetcalculator.h
FORMS += src/mainwindow.ui \
    src/widgets/bankaccountswidget.ui \
    src/widgets/debugdialogwidget.ui \
    src/widgets/knownempfaengerwidget.ui \
    src/pages/page_log.ui \
    src/pages/page_ausgang.ui \
    src/dialogs/dialogsettings.ui \
    src/dialogs/abt_dialog.ui \
    src/pages/page_history.ui
OTHER_FILES += \
    documentation/Doxyfile \
    documentation/HBCI_Geschaeftsvorfaelle.txt \
    translation/abtransfers-helptext_de.html \
    translation/abtransfers-helptext_en.html \
    translation/abtransfers.en_GB.ts \
    translation/README.translations
RESOURCES += src/resources.qrc

unix|macx {
    contains(AQBANKING_INSTALLATION, "pkg_config") {
        CONFIG += link_pkgconfig
        #check for Qt5 or greater
        greaterThan(QT_MAJOR_VERSION, 4) {
            PKGCONFIG += gwenhywfar gwengui-qt5 aqbanking
        } else {
            DEFINES += GWEN_QT4
            PKGCONFIG += gwenhywfar gwengui-qt4 aqbanking
        }
    } else {
        # For tests with the lastest or locally installed aqbanking version! Requires Qt5!
        LIBS += -L$${AQBANKING_INSTALLATION}/lib -laqbanking -lgwenhywfar -lgwengui-qt5 -lgwengui-cpp
        INCLUDEPATH += $${AQBANKING_INSTALLATION}/include/aqbanking5 \
                       $${AQBANKING_INSTALLATION}/include/gwenhywfar4
    }
} else {
    warning(configuring project for a not tested platform. The LIBS are probably wrong.)
    #The above (CONFIG/PKGCONFIG) could be used if pkg-config is available.
    #Otherwise adjust the following
    LIBS += -laqbanking -lgwenhywfar -lgwengui-qt4
    INCLUDEPATH += /usr/include/aqbanking5 \
		   /usr/include/gwenhywfar4
}

# This is needed for the dock icon on Max OS X.
macx:ICON = images/abtransfers.icns
# According to http://osdir.com/ml/kde-commits/2011-09/msg06346.html the following seems to be necessary.
macx:DEFINES += MSG_NOSIGNAL=0

# This variable specifies the directory where all intermediate moc files should be placed.
MOC_DIR = tmp

# This variable specifies the directory where all intermediate objects should be placed.
OBJECTS_DIR = tmp
UI_DIR = tmp

# Compile translations, so that they are useable for the resources file
#  But keep in mind that
#   1) "lupdate's CODECFORTR is deprecated and will be removed soon."   (Seems to be only needed for Windows!)
#   2) "QTextCodec::codecForTr() and QTextCodec::setCodecForTr() have been removed"  -->> main.cpp affected!!!
#  (See http://qt.gitorious.org/qt/qtbase/blobs/stable/dist/changes-5.0.0 for reference.)
!greaterThan(QT_MAJOR_VERSION, 4):CODECFORTR = UTF-8
system(lrelease $${TRANSLATIONS})

# Only one of the two commands will return a valid SVN revision, depending on whether the working is SVN or Hg(+hgsubversion):
SVN_REVISION = $$system(svnversion -n | sed 's/[^0-9MSP:]//g') # current repository revision (without newline) and consider case of non-svn-directory
HG_REVISION =  $$system(hg sum 2>/dev/null | grep \"parent:\" | sed \"s/parent: \\([0-9^:]*\\):.*$/\\1/\") # repo checked out with Mercurial and hgsubversion?

# revision as define for the Preprocessor ( \\\" so that \" goes to the Preprocessor)
# DEFINES += MVW_SVN_REVISION=\\\"$${SVN_REVISION}\\\" \
# MVW_VERSION=\\\"$${VERSION}\\\" \ # MVW_VERSION_EXTRA=\"\\\"'development-version-test test-test'\\\"\" #damit auch space möglich ist
# MVW_VERSION_EXTRA=\\\"development-version\\\" # keine space möglich!
DEFINES += ABTRANSFER_SVN_REVISION=\\\"$${SVN_REVISION}$${HG_REVISION}\\\" \
    ABTRANSFER_VERSION=\\\"$${VERSION}\\\" \
    ABTRANSFER_FINTS_VER=\\\"$${ABTRANSFER_FINTS_VER}\\\"


# NO_DEBUG
# QT_NO_DEBUG \
# QT_NO_DEBUG_STREAM
# we want to stop the build and output some information (only for debug)
# error(Subversion revision: $$SVN_REVISION)
message(Subversion revision: $${SVN_REVISION}$${HG_REVISION})
#message(Mercurial revision: $${HG_REVISION})
