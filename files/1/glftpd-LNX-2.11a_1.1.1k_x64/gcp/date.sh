#!/bin/sh

BINDIR="/glftpd/bin"
SITEDIR="/glftpd/site"
INCDIR="/glftpd/site/incoming"

# This is the relative path from the SITEDIR to INCDIR (needed to create a
# 'today' link to newest dated directory. If incoming is in /glftpd/site,
# then the relative path from site is ./incoming
RELATIVEINC="./incoming"

# No need to modify anything below

date=`date +%m%d`

# the following will make sure people can't upload to old dated dirs
chmod o-w "$INCDIR/*"

# this will create a new dated dir
mkdir "$INCDIR/$date"
chmod 777 "$INCDIR/$date"
cd "$SITEDIR"
rm -f ./today
ln -s "$RELATIVEINC/$date" today
