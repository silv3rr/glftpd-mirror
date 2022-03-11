#!/bin/bash
# simple script to create dated dirs
# run this script as root. change paths if you're not using standard.

date=`date +%m%d`
#date2=`date --date '1 days ago' +%m%d`  # this is just an example

# the following will make sure people can't upload to old dated dirs
chmod o-w /glftpd/site/incoming/*

# this will create a new dated dir
mkdir /glftpd/site/incoming/$date
chmod 777 /glftpd/site/incoming/$date

# This will create a 'today' link to today's dated directory
cd /glftpd/site
rm today
ln -s ./incoming/$date today
