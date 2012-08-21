#!/bin/bash

# this scripts creates a new branch of the trunk folder and replaces the
# relevant parts in the project file (if wanted)


SVNROOTURL=$(svn info | grep "Repository Root:" | awk '{ print $3 }')

#default values, possible changed by parse_arguments()
SOURCEDIR="trunk"
DESTDIR="branch"
PRESTRING=""
POSTSTRING=""
MODIFYPRJFILE="true"


function echo_usage() {
	echo "usage: ${0} version [-s source] [-d dest] [-pre string] [-post string] [-modify true/false]"
	echo ""
	echo -e "\tversion"
	echo -e "\t\tthe version for the created branch"
	echo -e "\t-s source (optional)"
	echo -e "\t\tdefines the directory to copy, default: trunk"
	echo -e "\t-d dest (optional)"
	echo -e "\t\tdefines the destination directory, default: branch"
	echo -e "\t-pre string (optional)"
	echo -e "\t\tString to prepend to the 'version', default: ''"
	echo -e "\t-post string (optional)"
	echo -e "\t\tString to append to the 'version', default: ''"
	echo -e "\t-modify true/false (optional)"
	echo -e "\t\tdefines if the project file should be modified or not, default: true"
}

function parse_arguments() {

	local i
	local next
	local param
	typeset -i i=1;
	typeset -i next=0;
	
	for param in $*; do
#		echo "for $i: $param"
		if [[ "X${param}" == "X-s" ]]; then
			next=i+1
			if [[ ${!next:0:1} == "-" || -z "${!next:0:1}" ]]; then
				echo_usage
				exit 3
			fi
			SOURCEDIR="${!next}"
		fi
		
		if [[ "X${param}" == "X-d" ]]; then
			next=i+1
			if [[ ${!next:0:1} == "-" || -z "${!next:0:1}" ]]; then
				echo_usage
				exit 3
			fi
			DESTDIR="${!next}"
		fi
		
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
			#poststring "-rc" should be possible!
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
		

if [[ -z "${1}" ]]; then
	echo_usage
	exit 1
fi

VERSION="${1}"

parse_arguments $*


#create a new branch
svn copy --quiet --message "Branch for release preperation of version $VERSION created" \
    ${SVNROOTURL}/${SOURCEDIR} ${SVNROOTURL}/${DESTDIR}/${PRESTRING}${VERSION}${POSTSTRING}



#we checkout the newly branched tree and modify the version information 
#in the project file (if wanted)

if [[ "${MODIFYPRJFILE}" == "true" ]]; then
	echo ""
	echo "the script should modifiy the project file 'ab_transfer.pro', lets do it"
	echo " - checkout the newly created branch"
	
	svn co --quiet ${SVNROOTURL}/${DESTDIR}/${PRESTRING}${VERSION}${POSTSTRING} /tmp/abtransfers_branch_temp
	
	echo -n " - modify the project file - replaced: "
	
	#replace the version with the supplied one
	sed -i "s/VERSION =.*/VERSION = ${VERSION}/" /tmp/abtransfers_branch_temp/abtransfers.pro
	echo -n "version " 
	#replace development-version with release-candidate
	sed -i "s/\([ ]*ABTRANSFER_VERSION_EXTRA=\\\*\"\)development-version\(.*\)/\1release-candidate\2/" /tmp/abtransfers_branch_temp/abtransfers.pro
	echo "Version-Extra"
	
	echo " - commit the modified project file to the repository"
	
	svn ci --quiet --message "Changed version number in project file to $VERSION" /tmp/abtransfers_branch_temp/
	
	echo " - remove the temporary checkout"
	rm -rf /tmp/abtransfers_branch_temp
	
	echo "done"
	echo ""
fi

echo " --------------------------------- "
echo ""
echo "new branch created, please checkout the new branch (or update the current"
echo "working copy)."
echo ""

exit 0

