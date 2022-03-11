#!/bin/sh
#dn's ZIP Checker
VERSION="1.00"

#INSTRUCTIONS
#Add the following lines to your glftpd.conf:
#  site_cmd ZIPCHK           EXEC    /bin/sitezipchk.sh
#  custom-zipchk   !8 *
#Make sure unzip and echo are in your binary path that is
#specified below and they are chmod 755

#VARIABLES
binpath="/bin" #relative to /glftpd

#DO NOT TOUCH BELOW THIS
#---------------------------------------------------------------
echo "dn's ZIP Checker v"$VERSION""
echo ""

#Do some tests for needed bins
[ ! -e "$binpath"/unzip ] && {
    echo "ERROR: the binary \"unzip\" can't be found in"
    echo "       $binpath.  Please fix this problem and "
    echo "       try again."
    exit 1
}

[ ! -e "$binpath"/echo ] && {       
    echo "ERROR: the binary \"echo\" can't be found in"
    echo "       $binpath.  Please fix this problem and "
    echo "       try again."
    exit 1
}

[ -z "$1" ] && {
    echo "Usage: site zipchk zipfile.zip"
    exit 0
}

file1="`basename $1`"

[ ! -e "$file1" ] && {
    echo "That zipfile does not exist!"
    exit 1
}
echo -n "Testing "$file1"... "
if "$binpath"/unzip -t "$file1" >/dev/null 2>&1; then
    echo "OK"
    echo ""
    exit 0
else
    echo "FAILED"
    echo ""
    exit 0
fi
