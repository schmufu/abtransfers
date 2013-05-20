#!/bin/bash

APP_RESOURCES=../abtransfers-build-Desktop_Version_4-Release/build/abtransfers.app/Contents/Resources/

if [ "$1" == "up" ]
then
	echo "Updating translations..."
	lupdate abtransfers.pro
else
	echo "Releasing translations into app package..."
	lrelease translation/abtransfers.en_GB.ts
	cp translation/abtransfers.en_GB.qm ${APP_RESOURCES}/abtransfers.en_US
	mv translation/abtransfers.en_GB.qm ${APP_RESOURCES}
fi
