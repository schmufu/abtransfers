#!/bin/bash

# create a new release in the tag directory
#
# checkout the supplied branch
# modifiy the project file
# copy the modified working copy to the supplied tag folder


SVNROOTURL=$(svn info | grep "Repository Root:" | awk '{ print $3 }')
DESTDIR="tags"

#default values, possible changed by parse_arguments()
PRESTRING=""
POSTSTRING=""
MODIFYPRJFILE="true"


function echo_usage() {
	echo "usage: ${0} branch-name version [-modify true/false] [-pre string] [-post string]"
	echo ""
	echo -e "\tbranch-name"
	echo -e "\t\tthe directory name of the branch that should be released"
	echo -e "\tversion"
	echo -e "\t\tthe version for the created release"
	echo -e "\t-modify true/false (optional)"
	echo -e "\t\tdefines if the project file should be modified or not, default: true"
	echo -e "\t-pre string (optional)"
	echo -e "\t\tString to prepend to the 'version', default: ''"
	echo -e "\t-post string (optional)"
	echo -e "\t\tString to append to the 'version', default: ''"
}

function parse_arguments() {

	local i
	local next
	local param
	typeset -i i=1;
	typeset -i next=0;
	
	for param in $*; do
#		echo "for $i: $param"
		if [[ "X${param}" == "X-pre" ]]; then
			next=i+1
			if [[ ${!next:0:1} == "-" || -z "${!next:0:1}" ]]; then
				echo_usage
				exit 3
			fi
			PRESTRING="${!next}"
		fi
		
		if [[ "X${param}" == "X-post" ]]; then
			next=i+1
			#as poststring "-rc" should be possible!
			if [[ -z "${!next:0:1}" ]]; then
				echo_usage
				exit 3
			fi
			POSTSTRING="${!next}"
		fi
		
		if [[ "X${param}" == "X-modify" ]]; then
			next=i+1
			if [[ ${!next:0:1} == "-" || -z "${!next:0:1}" ]]; then
				echo_usage
				exit 3
			fi
			if [[ "x${!next}" == "xfalse" ]]; then
				MODIFYPRJFILE="false"
			fi
		fi

		i=i+1
	done
	
}
		

if [[ -z "${1}" || -z "${2}" ]]; then
	echo_usage
	exit 1
fi

VERSION="${2}"
BRANCHNAME="${1}"

parse_arguments $*


#checkout the supplied branch to a temporary directory
svn co --quiet ${SVNROOTURL}/branch/${BRANCHNAME} /tmp/abtransfers_branch_temp


#we checkout a branch that should be released.
#modify the project file (only if wanted)

if [[ "${MODIFYPRJFILE}" == "true" ]]; then
	echo ""
	echo "the script should modifiy the project file 'ab_transfer.pro', lets do it"
	echo -n " - modify the project file - replaced: "
	
	#replace the version with the supplied one
	sed -i "s/VERSION =.*/VERSION = ${VERSION}/" /tmp/abtransfers_branch_temp/abtransfers.pro
	echo -n "version " 
	
	#remove backslash from ABTRANSFER_VERSION line
	sed -i "s/\([ ]*ABTRANSFER_VERSION=.*\) \\\/\1/" /tmp/abtransfers_branch_temp/abtransfers.pro
	#remove the whole ABTRANSFER_VERSION_EXTRA line (now it is an official release)
	sed -i "s/[ ]*ABTRANSFER_VERSION_EXTRA=.*//" /tmp/abtransfers_branch_temp/abtransfers.pro
	echo "Version-Extra"
	
	echo "done"
	echo ""
	
fi

#copy the current WC to a new tagged release
svn cp --quiet --message "released version ${PRESTRING}${VERSION}${POSTSTRING}" \
    /tmp/abtransfers_branch_temp ${SVNROOTURL}/${DESTDIR}/${PRESTRING}${VERSION}${POSTSTRING}


echo "remove the temporary checkout"
rm -rf /tmp/abtransfers_branch_temp



echo " --------------------------------- "
echo ""
echo "new release created, please checkout the new branch (or update the current"
echo "working copy)."
echo "Remember to merge back the changes, made by this script in the tags folder,"
echo "back to trunk for the current development."
echo ""

exit 0














