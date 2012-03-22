#!/bin/bash

# This script creates a new source archive of the supplied version in the
# supplied directory.
# 
# Therefore it does a checkout of the supplied version from the tag-directory
# of the subversion repository and exports all files.
# During development the compiled subversion revision is supplied to the
# compiler per define. This is no longer possible when the sources from the
# archive are compiled. The corresponding expression in the project file is
# replaced with a static value of the current subversion revision.
# [All files that are only relevant for development are removed] (not yet).
# After that the new source archive is created as an .tar.bz2 in the
# supplied folder.

function echo_usage() {
	echo "usage: $0 version folder_to_store_archive"
	echo ""
	echo -e "\tversion:"
	echo -e "\t\tthe version that should be checked out for archive creation"
	echo ""
	echo -e "\tfolder_to_store_archive:"
	echo -e "\t\tthe folder where the archive should be stored"
	echo -e "\n"

}

function modify_dynamic_project_rules() {
	if [[ -z "${1}" ]]; then
		echo "SCRIPTING FAILURE: $FUNCNAME called without parameter"
		cleanup
		exit 9
	fi

	echo "modifying dynamic values in project qmake rules to static values"
	
	#svn revision replacement with static value
	sed -i "s/SVN_REVISION = .*/SVN_REVISION = ${1}/" ab_transfer.pro
	#remove backslash from ABTRANSFER_VERSION line
	sed -i "s/\([ ]*ABTRANSFER_VERSION=.*\) \\\/\1/" ab_transfer.pro
	#remove the while ABTRANSFER_VERSION_EXTRA line
	sed -i "s/[ ]*ABTRANSFER_VERSION_EXTRA=.*//" ab_transfer.pro	    
}

function remove_files_only_for_development() {
	#the whole scripts folder contains only files for development
	rm -rf scripts
	
	
}
	

function cleanup() {
	cd /	#to be in a defined directory
	rm -rf ${TMPCHKOUTDIR}
}
	

if [[ -z "$1" || -z "$2" ]]; then
	echo_usage
	exit 1
fi


VERSION=$1
DESTDIR=$2

SVNURL="http://schmufu.dyndns.org/svn/ab_transfers/tags/"
PROGRAMNAME="abtransfers"
TMPCHKOUTDIR="/tmp/${PROGRAMNAME}"


#check out the supplied version and change in this directory
svn co ${SVNURL}${VERSION} ${TMPCHKOUTDIR}

if [[ $? -ne 0 ]]; then
	echo "ERROR: checkout of version $VERSION failed"
	cleanup
	exit 2
fi

cd ${TMPCHKOUTDIR}

#get the subersion revision and set it static in project file
SVNREV=$(svnversion -n)
modify_dynamic_project_rules "$SVNREV"

#delete all files that are only for development (not needed in archive)
#remove_files_only_for_development


#export all sources from our checked out version
svn export . "${PROGRAMNAME}-${VERSION}"

if [[ $? -ne 0 ]]; then
	echo "ERROR: export of version $VERSION failed"
	cleanup
	exit 3
fi

#create the new archive in the supplied directory
tar -cjf "${DESTDIR}/${PROGRAMNAME}-${VERSION}.tar.bz2" "${PROGRAMNAME}-${VERSION}"

if [[ $? -ne 0 ]]; then
	echo "ERROR: creating the archive ${DESTDIR}/${PROGRAMNAME}-${VERSION} failed"
	cleanup
	exit 4
fi

#if we are here, everything went fine -> cleanup
cleanup


echo ""
echo "##############################"
echo ""
echo "archive created: ${DESTDIR}/${PROGRAMNAME}-${VERSION}.tar.bz2"
echo ""

exit 0
