#!/bin/bash
#
# Usage:
#
# * extract new translations from the sources
#     make-translations.bash up
# 
# * extract translations and dump all obsolete ones
#     make-translations.bash up noobs
#
# * create qm file and copy it into MacOSX app package
#     make-translations.bash


if [ `uname` = "Darwin" ]; then
	PREFIX=/opt/macports-test
else
	PREFIX=/usr
fi

BIN=${PREFIX}/bin
TC_APPNAME=abtransfers
APP_RESOURCES=../abtransfers-build-Qt_4_8_4_macports_test-Release/build/AB-Transfers.app/Contents/Resources/
LANGS="en_GB"

if [ "$1" == "up" ]
then
	echo "Updating translations..."
	if [ "$2" == "noobs" ]
	then
		OPTIONS=-no-obsolete
	fi

	for LANG in $LANGS ; do
		echo "lupdate for language '$LANG'..."
		${BIN}/lupdate \
			-source-language de_DE \
			-target-language $LANG \
			-locations relative \
			${OPTIONS} \
				abtransfers.pro \
			-ts translation/abtransfers.$LANG.ts
	done
else
	if [ `uname` = "Darwin" ]; then
		echo "Releasing translations into app package..."
		for LANG in $LANGS ; do
			echo "lrelease for language '$LANG'..."
			${BIN}/lrelease translation/abtransfers.$LANG.ts
			cp translation/abtransfers.$LANG.qm ${APP_RESOURCES}/$TC_APPNAME.$LANG.qm
		done
	else
		echo "Releasing translations for Linux not yet implemented."
	fi
fi
