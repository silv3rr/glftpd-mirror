#!/bin/sh
#dn's ZIP Lister
VERSION="1.00"

#INSTRUCTIONS
#Add the following lines to your glftpd.conf:
#  site_cmd ZIPLIST           EXEC    /bin/siteziplist.sh
#  custom-ziplist   !8 *
#Make sure unzip and echo are in your binary path that is
#specified below and they are chmod 755

#VARIABLES
binpath="/bin" #relative to /glftpd

#DO NOT TOUCH BELOW THIS
#---------------------------------------------------------------
echo "dn's ZIP Lister v"$VERSION""
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
    echo "Usage: site ziplist zipfile.zip [somefile]"
    exit 0
}

file1="`basename $1`"

[ ! -e "$file1" ] && {
    echo "That zipfile does not exist!"
    exit 1
}

if [ -z "$2" ]; then
    echo "Listing from:"
    "$binpath"/unzip -l -v "$file1" 2>/dev/null
    echo ""
    exit 0
else
    file2="`basename $2`"
    echo "Listing "$file2" from:"
    "$binpath"/unzip -l -v "$file1" "$file2" 2>/dev/null
    exit 0
fi

exit 0
