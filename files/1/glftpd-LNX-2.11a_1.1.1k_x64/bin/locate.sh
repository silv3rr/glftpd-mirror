#!/bin/bash -
# locate.sh by Usurper (November 1999)

#Be advised that it might be possible to "hack" into a site using scripts
#like these. This one should be safe, but it is still recommended that
#you only give access to "trusted" people.

#This small script will find files that the user gives on the command line.
#exmp: "site locate abcdefg.zip" will tell you where this file is.
#You can also use wildcards, like "site locate abcd*".

#NOTE: make sure you have /glftpd/dev/null and that it's globally writable
#(chmod 666 /glftpd/dev/null). Make sure you have the following binaries
#in /glftpd/bin: find, echo, cut, bash, and that you have libraries
#that these binaries need in /glftpd/lib (and of course you need to run
#chroot ldconfig, but this should be already done during installation)

#Change "sitepath" to where the script should search for the files.
#exmp: sitepath=/site
sitepath=/site

#Change "pathsize" to the number of characters that will be stripped
#from each file found.  For example, if you want "/site/incoming/file.zip"
#to appear as "/incoming/file.zip", set pathsize to 5.
pathsize=5

echo -e "Hold on, this might take a while...\r"
/bin/find "$sitepath" -name "$1" -exec /bin/echo -e {} \; 2>/dev/null | /bin/cut -c"$pathsize"-
