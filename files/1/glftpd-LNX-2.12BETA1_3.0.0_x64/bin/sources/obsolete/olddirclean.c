// Olddirclean v1.2 by Usurper
// Removes non-existent directories from your dirlog
// Use it so that SITE SEARCH blah will not find old directories
// Also, if your dirlog file is too big, SITE NEW might be slow,
// so use this util to prevent that.
// I recommend using this in conjunction with the update util
// (best run it from cron every day, remember to move the new
// dirlog file from dirlog2 to dirlog a few minutes later if you
// set replace_dirlog to 0)
// Here is an example of a cron entry for this:
// 00 01 * * * /glftpd/bin/olddirclean /glftpd /glftpd/ftp-data/logs/dirlog > /glftpd/ftp-data/logs/olddirclean.log
// (This will run at 1am every day and put the output messages into olddirclean.log in case you want to examine them)

// If you don't execute this as root, make sure you do
// chmod +s olddirclean or that the user that executes it has
// rights to create/delete/rename files in the ftp-data directory

int replace_dirlog = 1;  /* change this to 0 if you don't want this util
                          to replace the old dirlog with the new one  */
int delete_orig_dirlog = 0;  /* set this to 1 if you don't want the backup
                                    of the original dirlog */

#include <sys/param.h>
#include <string.h>
#include <stdio.h>

struct dirlog {
        ushort status;     /* 0 = NEWDIR, 1 = NUKE, 2 = UNNUKE, 3 = DELETED */
        time_t uptime;
        ushort uploader;
        ushort group;
        ushort files;
        unsigned long long bytes;
        char dirname[255];
        struct dirlog *nxt;
        struct dirlog *prv;
};

main(int argc, char *argv[]) {
  FILE *file, *file2;
  uid_t oldid = geteuid();
  char dirlogpath[MAXPATHLEN];
  char newdirlog[MAXPATHLEN];
  char testdir[MAXPATHLEN+8];
  char rootpath[MAXPATHLEN];
  struct dirlog direntry;
  int tmp;

  printf("Olddirclean for glftpd, v1.2 by Usurper, May 1999\n");
  printf("This util removes non-existent directories from your dirlog.\n");
  if (argc > 1) {
    if (!strcmp(argv[1],"--help")) {
      printf("Usage: %s /rootpath /glftpd/ftp-data/logs/dirlog\n",argv[0]);
      return 0;
    }
    strcpy(rootpath,argv[1]);
  } else {
    printf("For syntax type %s --help\n",argv[0]);
    printf("No rootpath specified, using /glftpd\n");
    strcpy(rootpath,"/glftpd");
  }

  if (argc == 3)
    strcpy(dirlogpath,argv[2]);
  else {
    printf("Second argument not specified, using /glftpd/ftp-data/logs/dirlog\n");
    strcpy(dirlogpath,"/glftpd/ftp-data/logs/dirlog");
  }

  snprintf(newdirlog, sizeof(newdirlog), "%s2", dirlogpath);

  seteuid(0);
  if ((file = fopen(dirlogpath, "r+b")) == NULL) {
    printf("Unable to open dirlog\n");
    return 0;
  }
  if((file2 = fopen(newdirlog, "w+b")) == NULL) {
    printf("Unable to create dirlog2\n");
    return 0;
  }
  while(!feof(file)) {
     if (fread(&direntry, sizeof(struct dirlog), 1, file) < 1)
       break;
     snprintf(testdir, sizeof(testdir),"test -d %s%s",rootpath,direntry.dirname);
     if (tmp = system(testdir))
       printf("Removing %s\n",direntry.dirname);
     else
       fwrite(&direntry, sizeof(struct dirlog), 1, file2);
  }

  fclose(file);
  fclose(file2);
  if (replace_dirlog) {
    snprintf(testdir, sizeof(testdir), "mv %s %stmp", dirlogpath, dirlogpath);
    if (system(testdir)) {
      printf("Unable to rename dirlog!\n");
      return 0;
    }
    snprintf(testdir,sizeof(testdir), "mv %s %s", newdirlog, dirlogpath);
    if (system(testdir)) {
      printf("Unable to rename dirlog2!\n");
      return 0;
    }
    snprintf(testdir,sizeof(testdir), "mv %stmp %s", dirlogpath, newdirlog);
    if (system(testdir)) {
      printf("Unable to rename dirlogtmp!\n");
      return 0;
    }
    printf("Dirlog replaced.\n");
    if (delete_orig_dirlog) {
      snprintf(testdir, sizeof(testdir), "rm -f %s", newdirlog);
      if (system(testdir))
        printf("Unable to delete %s\n", newdirlog);
      else
        printf("%s deleted.\n",newdirlog);
    }
  } else
    printf("Done. Rename %s to %s for changes to take effect.\n",newdirlog,dirlogpath);
  setuid(oldid);
}

