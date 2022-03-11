/*
  Nukelogclean v1.0 by Usurper
  Removes old nuke entries from your nukelog
  Use it to cut down the size of your nukelog, it will save you
  some space and perhaps speed up the site nukes command

  If you don't execute this as root, make sure you do
  chmod +s nukelogclean or that the user that executes it has
  rights to create/delete/rename files in the ftp-data directory
*/

int replace_nukelog = 1;  /* change this to 0 if you don't want this util
                             to replace the old nukelog with the new one  */
int delete_orig_nukelog = 0;  /* change this to 1 if you don't want to keep
                                 a backup of the original nukelog */

#include <time.h>
#include <sys/param.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "glconf.h"

int get_dirage(time_t dirtime) {

        time_t timenow = time(NULL);
        time_t difftime;
        int days = 0;

        difftime = timenow - dirtime;

        while(difftime >= (time_t)86400)
        {
                difftime -= (time_t)86400;
                days++;
        }
  return days;
}

int main(int argc, char *argv[]) {
  FILE *file, *file2;
  uid_t oldid = geteuid();
  char nukelogpath[MAXPATHLEN];
  char newnukelog[MAXPATHLEN];
  char tempcommand[MAXPATHLEN];
  struct nukelog nukeentry;
  int howold, dirage;

  printf("Nukelogclean for glftpd, v1.0 by Usurper, May 1999\n");
  printf("This util removes old nuke entries from your nukelog.\n");
  if (argc > 1) {
    if (!strcmp(argv[1],"--help")) {
      printf("Usage: %s dirage /glftpd/ftp-data/logs/nukelog\n",argv[0]);
      printf("       Dirage means anything older than this number of days will be deleted\n");
      return 0;
    } 
    howold = atoi(argv[1]);
  } else {
    printf("For syntax type %s --help\n",argv[0]);
    printf("No dirage specified, using 14\n");
    howold = 14;
  }
  
  if (argc > 2)
    strcpy(nukelogpath,argv[2]);
  else {
    printf("Second argument not specified, using /glftpd/ftp-data/logs/nukelog\n");
    strcpy(nukelogpath,"/glftpd/ftp-data/logs/nukelog");
  }

  snprintf(newnukelog, sizeof(newnukelog), "%s2", nukelogpath);

  seteuid(0);
  if ((file = fopen(nukelogpath, "r+b")) == NULL) {
    printf("Unable to open %s\n",nukelogpath);
    return 0;
  }
  if((file2 = fopen(newnukelog, "w+b")) == NULL) {
    printf("Unable to create %s\n",newnukelog);
    return 0;
  }
  while(!feof(file)) {
     if (fread(&nukeentry, sizeof(struct nukelog), 1, file) < 1)
       break;
     dirage = get_dirage(nukeentry.nuketime);
     if (dirage > howold)
       printf("Removing %s (%i days old)\n",nukeentry.dirname, dirage);
     else 
       fwrite(&nukeentry, sizeof(struct nukelog), 1, file2);
  }

  fclose(file);
  fclose(file2);
  if (replace_nukelog) {
    snprintf(tempcommand, sizeof(tempcommand), "mv %s %stmp", nukelogpath, nukelogpath);
    if (system(tempcommand)) {
      printf("Unable to rename nukelog!\n");
      return 0;
    }
    snprintf(tempcommand,sizeof(tempcommand), "mv %s %s", newnukelog, nukelogpath);
    if (system(tempcommand)) {
      printf("Unable to rename nukelog2!\n");
      return 0;
    }
    snprintf(tempcommand,sizeof(tempcommand), "mv %stmp %s", nukelogpath, newnukelog);
    if (system(tempcommand)) {
      printf("Unable to rename nukelogtmp!\n");
      return 0;
    }
    printf("Nukelog replaced.\n");
    if (delete_orig_nukelog) {
      snprintf(tempcommand, sizeof(tempcommand), "rm -f %s", newnukelog);
      if (system(tempcommand))
        printf("Unable to delete %s\n", newnukelog);
      else
        printf("%s deleted.\n", newnukelog);
    }
  } else
    printf("Done. Rename %s to %s for changes to take effect.\n",newnukelog,nukelogpath);
  setuid(oldid);
  return 0;
}

  
