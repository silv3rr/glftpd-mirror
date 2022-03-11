#!/bin/bash
#  Free space routine written by barkeep
#  Modified by Oliwa (this version WORKS more efficiently)
#
#  This script will move a whole directory to an archive drive
#  to help keep space available on your incoming drive.  When
#  the archive drive gets filled, it deletes the directories off
#  that drive to make space for the newer dir's.
#
#  ver 1.0
#
#
#  Run this manually at first to make sure its working fine.  Then if all
#  is ok, set it up in your crontab.
#
##########################  VARIABLES #############################

freespace=1500           # megs of space in uploads to keep free.
			 # Judge according to your site traffic

uploads=/glftpd/site/Incoming     
			 # your upload directory
                         # This is the directory where you
                         # would like your directories moved
                         # to the archives.

archives=/glftpd/site/Archive   
			 # your archive dir.  The place where
                         # your old uploads are moved to as well
                         # as where old archives are removed to
                         # clear up space.

movedir=/glftpd/site/Archive    
			 # dir where you want stuff moved to.
                         # This should generally be the same as
                         # your archive dir unless you have a
                         # special case.

uploaddev=/dev/hdb1      # upload drive device. This is the same device
                         # where uploads is contained in.

archivedev=/dev/hdc1     # archive drive device.  This is the same device
                         # where your archives is contained in.


########################## Do not modify below #####################
echo " "
echo " ************************************** "
echo " * Space Creator by Barkeep and Oliwa   *
echo " ************************************** "
echo " "

# This deletes all nukes from your drive first.  What's
# the sense of archiving nukes?

#Nuking all nukes
#cd /glftpd/site/Incoming
#rm -fr */*NUKED*


# Calculate free space on upload device
    freespace=`expr $freespace`
    if [ `df -m | grep $uploaddev | awk '{print $4}'` -gt "$freespace" ]
then
       echo "More than $freespace megs free"
       exit 0
    else
       echo "Not enough free space remaining.  Free'ing Space."
    fi
 
    echo " "

     until [ `df -m | grep $uploaddev | awk '{print $4}'` -gt "$freespace" ]
     do
        # Find oldest dir in uploads
        OLDUP=`ls -r $uploads | tail -n1`
        echo "$OLDUP will be moved to $movedir"

        # chdir into that dir and calculate SPACENEEDED
        SPACENEEDED=`du -ms /$uploads/$OLDUP | awk '{print $1}'`
       
        SPACENEEDED=`expr $SPACENEEDED`
        echo " " 
        until [ `df -m |grep $archivedev |awk '{print $4}'` -gt "$SPACENEEDED" ]
        do
             # Find oldest dir in archives
             OLDARCH=`ls -r $archives | tail -n1`
             echo "Removing the $OLDARCH   directory "
             rm -fr $archives/$OLDARCH
        done  

        # Move the Files to the Archives dir.
        echo "Moving $OLDUP to $movedir... "
        mv $uploads/$OLDUP $movedir
        echo " "
    done
    