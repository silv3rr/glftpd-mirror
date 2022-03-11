/* Olddirclean v2.2 by Usurper
   - changed the 'bytes' variable in dirlog struct to be unsigned long

   Olddirclean v2.1 by Usurper
   - changed system calls to C functions (faster + no filename bugs)
   - made user output more friendly
   - changed errors to be printed to stderr
   - added support for glftpd config file
   - added command-line options
   - added automatic sorting of dirlog

This removes non-existent directories from your dirlog.
As of November 6, 2000, it also sorts the dirlog.

Use it so that SITE SEARCH will not find old directories that have been
moved or deleted. Also, if your dirlog file is too big, SITE NEW might be
slow, so use this util to prevent that.

I recommend using this in conjunction with the glupdate util (best run it from
cron every day). Run this *after* glupdate, so that the dirlog ends up sorted.

Here is an example of a cron entry for this:
00 01 * * * /glftpd/bin/olddirclean2 > /glftpd/ftp-data/logs/olddirclean.log
(This will run at 1am every day and put the output messages into
 olddirclean.log in case you want to examine them)

If you don't execute this as root, make sure you do chmod +s olddirclean2 or
that the user that executes it has rights to create/delete/rename files in the
ftp-data directory.

*/

int replace_dirlog = 0;      /* change this to 1 if you want to replace
			        the old dirlog with the new one by default   */
int delete_orig_dirlog = 0;  /* set this to 1 if you don't want a backup
                                of the original dirlog                       */

#define MAXDIRLOGSIZE 10000

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "glconf.h"

char datapath[255] = "/ftp-data";	/* assign default datapath, will use value from conf if defined there */
char rootpath[255] = "/glftpd";		/* assign default rootpath, will use value from conf if defined there */
char config_file[MAXPATHLEN];

void trim(char *str) {
    char *ibuf, *obuf;

    if (str) {
        for (ibuf = obuf = str; *ibuf;) {
            while (*ibuf && (isspace(*ibuf)))
                ibuf++;
            if (*ibuf && (obuf != str))
                *(obuf++) = ' ';
            while (*ibuf && (!isspace(*ibuf)))
                *(obuf++) = *(ibuf++);
        }
        *obuf = '\0';
    }
    return;
}

void load_sysconfig(void) {
    FILE *configfile;
    char lvalue[64], rvalue[MAXPATHLEN], work_buff[MAXPATHLEN];
    int  x, y;

    if ((configfile = fopen(config_file, "r")) == NULL) {
	fprintf(stderr, "Unable to open config file %s, using defaults.\n", config_file);
	return;
    }

    while (fgets(work_buff, sizeof(work_buff), configfile) != NULL) {
      for (x = 0; x < (int)strlen(work_buff); x++)  /* Clip out comments */
         if (work_buff[x] == '#')
            work_buff[x] = '\0';

      trim(work_buff);
      memset(lvalue, '\0', sizeof(lvalue));
      memset(rvalue, '\0', sizeof(rvalue));

      y = 0;
      for (x = 0; x < (int)strlen(work_buff) && work_buff[x] != ' '; x++)
         if (isprint(work_buff[x]))
            lvalue[y++] = work_buff[x];

      y = 0; x++;
      for (; x < (int)strlen(work_buff); x++)
         if (isprint(work_buff[x]))
            rvalue[y++] = work_buff[x];

      if (strcasecmp(lvalue, "datapath") == 0)
                strncpy( datapath, rvalue, sizeof(datapath));
      if (strcasecmp(lvalue, "rootpath") == 0)
                strncpy( rootpath, rvalue, sizeof(rootpath));
    }

    fclose(configfile);
    return;
}
void show_help(char *argv) {
	printf("\nThis util removes non-existent directories from your dirlog and\n");
	printf(" sorts it so that newest directories show up first in \"site new\".\n");
	printf("\nUsage: %s [options]\n",argv);
	printf("Options:\n");
	printf("         -r path   Specify alternate path to config file\n");
	printf("         -P        Force replacing of dirlog\n");
	printf("         -p        Force not replacing of dirlog [default]\n");
	printf("         -D        Force deleting of old dirlog\n");
	printf("         -d        Force keeping of old dirlog [default]\n");
	printf("\n");
	exit(0);
}

int main(int argc, char *argv[]) {
  FILE *file, *file2;
  uid_t oldid = geteuid();
  char dirlogpath[MAXPATHLEN], newdirlog[MAXPATHLEN], testdir[MAXPATHLEN+8];
  struct dirlog direntry, *dirptr, *dirlist[MAXDIRLOGSIZE];
  struct stat st;
  int curr=0, curr2, ArgCounter;

  strcpy(config_file, GLCONF);

  printf("Olddirclean for glftpd, v2.2 by Usurper, November 2000\n");
  printf("Use option -h (%s -h) for syntax help.\n", argv[0]);

  ArgCounter = 1;
  while (argc > ArgCounter) {
	if (argv[ArgCounter][0] != '-') {
	  printf("Unknown option, skipping [%s]\n",argv[ArgCounter]);
	} else {
	  switch (argv[ArgCounter][1]) {
		case 'r':
		    if (argv[ArgCounter][2])
                      strcpy(config_file, argv[ArgCounter]+2);
		    else if (argv[ArgCounter+1])
                      strcpy(config_file, argv[++ArgCounter]);
		    else
		      printf("No argument to the r option\n");
                    break;
		case 'P':
		    replace_dirlog = 1;
                    break;
		case 'p':
		    replace_dirlog = 0;
                    break;
		case 'D':
		    delete_orig_dirlog = 1;
                    break;
		case 'd':
		    delete_orig_dirlog = 0;
                    break;
		case 'h':
		case '?':
		    show_help(argv[0]);
		    break;
		default:
                    printf("Unknown option, skipping [%s]",argv[ArgCounter]);
            } /* switch */
        } /* else */
        ArgCounter++;
  } /* while */

  load_sysconfig();
  sprintf(dirlogpath,"%s%s/logs/dirlog", rootpath, datapath);
  printf("\nReading %s\n", dirlogpath);

  seteuid(0);
  if ((file = fopen(dirlogpath, "r+b")) == NULL) {
    fprintf(stderr, "Unable to open dirlog\n");
    return 0;
  }
  snprintf(newdirlog, sizeof(newdirlog), "%s2", dirlogpath);
  if((file2 = fopen(newdirlog, "w+b")) == NULL) {
    fprintf(stderr, "Unable to create dirlog2\n");
    return 0;
  }
  while(!feof(file)) {
     if (fread(&direntry, sizeof(struct dirlog), 1, file) < 1)
       break;
     snprintf(testdir, sizeof(testdir),"%s%s", rootpath, direntry.dirname);
     if (stat(testdir, &st) != 0)
       printf("Removing %s\n",direntry.dirname);
     else {
    /*  fwrite(&direntry, sizeof(struct dirlog), 1, file2);  */
	dirlist[curr] = (struct dirlog *)malloc(sizeof (struct dirlog));
	*dirlist[curr++] = direntry;
	if (curr >= MAXDIRLOGSIZE) {
	  printf("Too many records in dirlog - recompile %s with MAXDIRLOGSIZE set to a greater value!\n", argv[0]);
	  exit(1);
	}
     }
  }
  fclose(file);
  printf("\nSorting dirlog");
  for (curr=0; dirlist[curr] != NULL; curr++) {
    for (curr2=curr+1; dirlist[curr2] != NULL; curr2++) {
      if (dirlist[curr]->uptime > dirlist[curr2]->uptime) {
        dirptr = dirlist[curr];
	dirlist[curr] = dirlist[curr2];
	dirlist[curr2] = dirptr;
      }
    }
    printf(".");
  }
  printf("\n\nWriting new dirlog...\n");
  for (curr=0; dirlist[curr] != NULL; curr++)
    fwrite(dirlist[curr], sizeof(struct dirlog), 1, file2);
  fclose(file2);
  if (replace_dirlog) {
    /* using testdir as a temporary buffer */
    snprintf(testdir, sizeof(testdir), "%stmp", dirlogpath);
    if (rename(dirlogpath, testdir) != 0) {
      fprintf(stderr, "Unable to rename %s!\n", dirlogpath);
      return 0;
    }
    if (rename(newdirlog, dirlogpath) != 0) {
      fprintf(stderr, "Unable to rename %s!\n", newdirlog);
      return 0;
    }
    if (rename(testdir, newdirlog) != 0) {
      fprintf(stderr, "Unable to rename %s!\n", testdir);
      return 0;
    }
    printf("Dirlog replaced.\n");
    if (delete_orig_dirlog) {
      if (unlink(newdirlog) != 0)
        fprintf(stderr, "Unable to delete %s\n", newdirlog);
      else
        printf("%s deleted.\n",newdirlog);
    }
  } else
    printf("Done. Rename %s to %s for changes to take effect.\n",newdirlog,dirlogpath);
  setuid(oldid);
  return 0;
}
