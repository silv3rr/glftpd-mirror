#!/bin/sh
#dn's NFO Lister for ZIP Files
VERSION="1.00"

#INSTRUCTIONS
#Add the following lines to your glftpd.conf:
#  site_cmd NFO            EXEC    /bin/sitenfo.sh
#  custom-nfo	!8 *
#Make sure unzip and echo are in your binary path that is
#specified below and they are chmod 755

#VARIABLES
binpath="/bin" #relative to /glftpd

#DO NOT TOUCH BELOW THIS
#---------------------------------------------------------------
echo "dn's NFO Lister v"$VERSION""
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
    echo "Usage: site nfo zipfile.zip [something.nfo]"
    exit 0
}

file1="`basename $1`"

[ ! -e "$file1" ] && {
    echo "That zipfile does not exist!"
    exit 1
}

if [ -z "$file2" ]; then
    echo "nfo(s) from "$file1":"
    "$binpath"/unzip -p -C "$file1" "*.[Nn][Ff][Oo]" 2>/dev/null
    echo ""
    exit 0
else
    file2="`basename $2`"
    echo ""$file2" from "$file1":"
    "$binpath"/unzip -p -C "$file1" "$file2" 2>/dev/null
    exit 0
fi

exit 0
