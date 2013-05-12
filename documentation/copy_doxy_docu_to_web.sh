#!/bin/bash

# This script copy the createt doxygen documentation to the supplied
# folder. Which should be reachable by the webserver.

#default values, possible changed by parse_arguments()
SOURCEFILES="$(dirname $0)/doxygen/html/*"
DESTDIR=""
DESTUSER=""
DESTGROUP=""
DESTMODE=""
USESU=false

function echo_usage() {
	echo "usage: ${0} destination-directory [-s source] [-u unix-username] [-g unix-group] [-m mode] [-usesu true/false]"
	echo ""
	echo -e "\tdestination-directory"
	echo -e "\t\tthe directory where the doxygen documentation should be copied to"
	echo -e "\t-s source (optional)"
	echo -e "\t\tdefines the source files to copy. Default: '${SOURCEFILES}'"
	echo -e "\t-u unix-username (optional)"
	echo -e "\t\tdefines the new owner of the copied files. Default: '${DESTUSER}' (no change)"
	echo -e "\t-g unix-group (optional)"
	echo -e "\t\tdefines the new group of the copied files. Default: '${DESTGROUP}' (no change)"
	echo -e "\t-m mode (optional)"
	echo -e "\t\tdefines the new file mode bits. Default: '${DESTMODE}' (no change)"
	echo -e "\t-usesu true/false (optional)"
	echo -e "\t\tdefines if the modifications should be done using \"su\". Default: ${USESU}"
	echo ""
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
			SOURCEFILES="${!next}"
		fi

		if [[ "X${param}" == "X-u" ]]; then
			next=i+1
			if [[ ${!next:0:1} == "-" || -z "${!next:0:1}" ]]; then
				echo_usage
				exit 3
			fi
			DESTUSER="${!next}"
		fi

		if [[ "X${param}" == "X-g" ]]; then
			next=i+1
			if [[ ${!next:0:1} == "-" || -z "${!next:0:1}" ]]; then
				echo_usage
				exit 3
			fi
			DESTGROUP="${!next}"
		fi

		if [[ "X${param}" == "X-m" ]]; then
			next=i+1
			#poststring "-rc" should be possible!
			if [[ -z "${!next:0:1}" ]]; then
				echo_usage
				exit 3
			fi
			DESTMODE="${!next}"
		fi

		if [[ "X${param}" == "X-usesu" ]]; then
			next=i+1
			if [[ ${!next:0:1} == "-" || -z "${!next:0:1}" ]]; then
				echo_usage
				exit 3
			fi
			if [[ "x${!next}" == "xtrue" ]]; then
				USESU="true"
			fi
		fi

		i=i+1
	done

}


if [[ -z "${1}" ]]; then
	echo_usage
	exit 1
fi

DESTDIR="${1}"

parse_arguments $*



#check if su is wanted and use it
SUCMD=""
if [[ "X${USESU}" == "Xtrue" ]]; then
	SUCMD="su -c"
else
	SUCMD="bash -c"
fi

#check if the supplied DESTDIR exists, create it if not
if [[ ! -d ${DESTDIR} ]]; then
	echo "creating directory: ${DESTDIR}"
	${SUCMD} "mkdir -p ${DESTDIR}"
fi

#delete the old docu
#rm -r /path/to/the/new/location/*
echo "deleting old files in ${DESTDIR}"
${SUCMD} "rm -r ${DESTDIR}/*"

#then copy the created docu to the specified directory
#cp -R doxygen/html/* /path/to/the/new/location/
echo "copying ${SOURCEFILES} to ${DESTDIR}/"
${SUCMD} "cp -R ${SOURCEFILES} ${DESTDIR}/"


if [[ -n "${DESTUSER}" ]]; then
	echo "changing the owner to: ${DESTUSER}"
	${SUCMD} "chown -R ${DESTUSER} ${DESTDIR}/*"
fi

if [[ -n "${DESTGROUP}" ]]; then
	echo "changing the group to: ${DESTGROUP}"
	${SUCMD} "chgrp -R ${DESTGROUP} ${DESTDIR}/*"
fi

if [[ -n "${DESTMODE}" ]]; then
	echo "changing the file mode bits to: ${DESTMODE}"
	${SUCMD} "chmod -R ${DESTMODE} ${DESTDIR}/*"
fi

echo ""
echo "all done"
echo ""

exit 0
