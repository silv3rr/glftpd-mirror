#!/bin/bash -
# locate.sh by Usurper (November 1999)
# 4/10/2000;	updated by djsmoke (djsmoke@optonline.net)

#### IMPORTANT: make sure you have the following binaries in /bin:
####            wc, find, echo, bash, cut, cat and rm
####
####		make sure the following files have the proper permissions
####	chmod 666 /glftpd/dev/null
####    chmod a+rwxt /glftpd/tmp

# change S_PATH to where the script should search for the files.
# example: S_PATH=/site
S_PATH=/site

# change PATH_SIZE to the number of characters that will be stripped
# from each file found.  For example, if you want "/site/incoming/file.zip"
# to appear as "/incoming/file.zip", set pathsize to 6.
PATH_SIZE=6

# change LOCATE_FILE to the *full* path where you want this script to store its
# temp files. If you want to create a tmp dir in your site root, issue the
# following command: mkdir /glftpd/tmp && chmod a+rwxt /glftpd/tmp
# If you want to store the file somewhere else, make sure you have all the
# correct permissions set.
LOCATE_FILE="/tmp/update.db"

### NO MORE EDITING IS NEEDED PAST THIS POINT.

if [ "$1" = "-d" ] || [ "$1" = "-f" ]; then
	echo "[locate]: searching for $2"
	echo " "
        # get the search type from $1
        S_TYPE="`echo $1 | cut -c2- -`"
	find $S_PATH -type $S_TYPE -iname $2 -exec echo \{\} >$LOCATE_FILE \; 2>/dev/null
else
	echo "Usage: site locate <switch> <search_pattern>"
	echo " "
	echo "locate switches are.."
	echo "-d	:search for directories that match <search_pattern>"
	echo "-f	:search for files that match <search_pattern>"
	echo " "
	exit 0
fi

# output what we found here and strip the crap we dont want
for i in `cat $LOCATE_FILE`; do
	echo $i | cut -c ${PATH_SIZE}-
done
echo " "
echo "`cat $LOCATE_FILE | wc -l` match(es) found"

# looks like we're done, lets clean up
rm -f $LOCATE_FILE
exit 0
