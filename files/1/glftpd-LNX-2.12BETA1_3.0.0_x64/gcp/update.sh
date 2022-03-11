#!/bin/sh

BINDIR="/glftpd/bin"
GLUPDATE="$BINDIR/glupdate"

# if you need to pass an argument to glupdate, like config file in a different
# path, do it here (only one argument will work)
#GLUPDATEARG="-r/etc/glftpd.conf"

# NOTE: glupdate is NOT recursive, meaning it won't update all subdirectories
# automatically, you need to specify all paths you want updated.

# Paths which will be scanned for dirs to update:
# Note: if you have your apps/games in directories like a-c, d-k,
# or dated directories, put a "/*" at the end of directory name,
# so that all directories under that path will be processed. You can also
# specify multiple directories, i.e.
# APPDIR="/glftpd/site/arch1/* /glftpd/site/requests /glftpd/site/blah"
APPDIR="/glftpd/site/archives/apps"  # processes all dirs in apps
GAMDIR="/glftpd/site/archives/games" # processes all dirs in games
INCDIR="/glftpd/site/incoming/*"     # processes all dirs in dirs under incoming

# archive games
ls -d $GAMDIR | while read dirname; do
 if [ -d $dirname ]; then   # only process directories
  echo ""
  echo "Processing $dirname..."
  echo ""
  "$GLUPDATE" "$GLUPDATEARG" "$dirname"
 fi
done

#archive apps
ls -d $APPDIR | while read dirname; do
 if [ -d $dirname ]; then   # only process directories
  echo ""
  echo "Processing $dirname..."
  echo ""
  "$GLUPDATE" "$GLUPDATEARG" "$dirname"
 fi
done

#incoming
ls -d $INCDIR | while read dirname; do
 if [ -d $dirname ]; then   # only process directories
  echo ""
  echo "Processing $dirname..."
  echo ""
  "$GLUPDATE" "$GLUPDATEARG" "$dirname"
 fi
done
