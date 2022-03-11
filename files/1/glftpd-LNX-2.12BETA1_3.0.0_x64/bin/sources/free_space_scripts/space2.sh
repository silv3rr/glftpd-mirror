#!/bin/bash
#  deleter routine written by barkeep
#  edited by Oliwa
#
#  
#  Full directories are deleted when free space
#  falls below a certain amount
#
#  ver 1.0
#
#
#  Run this manually at first to make sure its working fine.  Then if all
#  is ok, set it up in your crontab.
#
##########################  VARIABLES #############################

freespace=2000           # megs of space in uploads to keep free.

uploads=/glftpd/site/Incoming
			 # your upload directory
                         # This is the directory where files
                         # are removed from when not enough space

uploaddev=/dev/hdb1      # upload drive device. This is the same device
                         # where uploads is contained in.


########################## Do not modify below #####################

echo " "
echo " ********************************************** "
echo " * Space Creator Created by Barkeep and Oliwa * "
echo " ********************************************** "
echo " "

# Calculate free space on upload device
    freespace=`expr $freespace`
    if [ `df -m | grep $uploaddev | awk '{print $4}'` -gt "$freespace" ] ;
then
       echo "More than $freespace megs free"
       exit 0
    else
       echo "Not enough free space remaining.  Free'ing Space."
    fi
 
    echo " "

     until [ `df -m |grep $uploaddev | awk '{print $4}'` -gt "$freespace" ]
     do
        # Find oldest dir in uploads
        OLDUP=`ls -t --ignore=*.txt $uploads | tail -n1`
        echo "$OLDUP  -  is being removed."
        rm -rf $uploads/$OLDUP

     done
     