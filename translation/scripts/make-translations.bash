#!/bin/bash

if [ `uname` = "Darwin" ]; then
	PREFIX=/opt/macports-test
else
	PREFIX=/usr
fi

BIN=${PREFIX}/bin
APP_RESOURCES=../abtransfers-build-Desktop_Version_4-Release/build/abtransfers.app/Contents/Resources/

if [ "$1" == "up" ]
then
	echo "Updating translations..."
	${BIN}/lupdate \
		-source-language de_DE \
		-target-language en_GB \
		-locations relative \
			abtransfers.pro

	mv abtransfers.en_GB.ts translation
else
	echo "Releasing translations into app package..."
	${BIN}/lrelease translation/abtransfers.en_GB.ts
	cp translation/abtransfers.en_GB.qm ${APP_RESOURCES}/abtransfers.en_US
	mv translation/abtransfers.en_GB.qm ${APP_RESOURCES}
fi
