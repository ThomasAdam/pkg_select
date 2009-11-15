#!/bin/sh

echo "/* this file is generated */"
echo "typedef struct Cats {"
echo "		const char *category;"
echo "		const char *descr;"
echo "} Cats;"
echo
echo "static Cats cats[] = {"
for file in `find /usr/pkgsrc/*/Makefile -maxdepth 1`
do
	CAT=`echo $file|sed -E 's/.+pkgsrc\///'|sed 's/\/Makefile//'`

	COMMENT=`grep COMMENT $file|sed -E 's/COMMENT=[^[:alnum:]]//'`

	echo "	{\"${CAT}\",		\"${COMMENT}\"}, "
done
echo "	{\"system\",	\"Base system\"},"
echo "	{NULL,	NULL}"
echo "};"
