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
APP_RESOURCES=../abtransfers-build-Qt_4_8_4_macports_test-Release/build/AB-Transfers.app/Contents/Resources/

if [ "$1" == "up" ]
then
	echo "Updating translations..."
	if [ "$2" == "noobs" ]
	then
		OPTIONS=-no-obsolete
	fi
	${BIN}/lupdate \
		-source-language de_DE \
		-target-language en_GB \
		-locations relative \
		${OPTIONS} \
			abtransfers.pro
else
	if [ `uname` = "Darwin" ]; then
		echo "Releasing translations into app package..."
		${BIN}/lrelease translation/abtransfers.en_GB.ts
		cp translation/abtransfers.en_GB.qm ${APP_RESOURCES}/AT-Transfers.en_GB.qm
	else
		echo "Releasing translations for Linux not yet implemented."
	fi
fi
