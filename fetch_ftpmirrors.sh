#!/bin/sh

START=0

for line in `lynx -source http://www.netbsd.org/mirrors/`
do
	if [ "${START}" = "0" ]; then
		LABEL=`echo ${line} | grep -i "name=\"ftp\">FTP"`

		if [ "${LABEL}" = "" ]; then
			continue
		else
			START=1;
		fi
	fi

	LABEL=`echo ${line} | grep -i "<hr>"`
	if [ "${LABEL}" != "" ]; then
		exit 0
	fi

	TMP=`echo ${line} | grep -i "<h3>" | sed -E s,\<[h3/]+\>,,g`
	if [ "${TMP}" = "" ]; then
		ADDRESS=`echo ${line} | grep ftp:// | awk -F\" {'print $2'} | grep -E NetBSD/?`
	else
		COUNTRY=${TMP}
	fi
	if [ "${ADDRESS}" != "" ]; then
		echo ${COUNTRY}:${ADDRESS}
	fi
done
