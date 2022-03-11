/* Most of this is ripped from GreyLine's code from glftpd */
/*

Version 1.1 modified by iwdisb 2004-26-06

   - Will now accept the -r switch
  
*/

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <sys/param.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "glconf.h"

char rootpath[MAXPATHLEN], datapath[MAXPATHLEN];

void getarg(char *var, char *line, int size) {
  while(line && (*line == ' ' || *line == '\t'))
    line++;
  strncpy(var, line, size);
  while(var && *var != ' ' && *var != '\t' && *var != '\0')
    var++;
  *var = '\0';
}

int readconf(char *configpath) {
  FILE *configfile;
  char line[1024];
  int cnt;

  if ((configfile = fopen(configpath, "r")) == NULL) {
  /*    printf("Cannot open config file: %s", strerror(errno)); */
    return 0;
  }
  while (fgets(line, sizeof(line), configfile)) {
    for (cnt=0; line[cnt]; cnt++) {
      if (line[cnt] == '#' || line[cnt] == '\n' || line[cnt] == '\r')
        line[cnt] = '\0';
    }
    if (strncmp(line, "rootpath ", 9) == 0)
      getarg(rootpath, line+9, sizeof(rootpath));
    else if (strncmp(line, "datapath ", 9) == 0)
      getarg(datapath, line+9, sizeof(datapath));
  }
  fclose(configfile);
  return 1;
}

void
usage (void)
{
        printf ("Usage: dupeadd [-r /pathto/glftpd.conf] filename uploader\n");
        exit (0);
}


int main(int argc, char *argv[]) {
  FILE *dfile;
  uid_t oldid = getuid();
  char dupepath[MAXPATHLEN], *confpath = GLCONF;
  struct dupefile dupeentry;
  char filename[256];
  char uploader[25];
  int c;

  printf("Dupeadd for glftpd, v1.1 by Usurper, June 1999\n");
  printf("This util adds a filename to the dupe database.\n");

  /* Parse command line options */
  while((c = getopt(argc, argv, "hr:")) != EOF) {
    switch(c) {
      case 'h':
        usage();
      case 'r':
        confpath = strdup(optarg);
        break;
      default:
        usage();
    }
  }

  argc -= optind;
  argv += optind;

  if (argc != 2)
    usage();

  strcpy(rootpath, "/glftpd");
  strcpy(datapath, "/ftp-data");

  if (!readconf(confpath))
    printf("Couldn't read config file. Using defaults. Use option -r /path/config.file to override.\n");

  strcpy(dupepath, rootpath);
  strcat(dupepath, datapath);
  strcat(dupepath, "/logs/dupefile");

  strncpy(filename, argv[0], sizeof(filename));
  strncpy(uploader, argv[1], sizeof(uploader));

  seteuid(0);
  if((dfile = fopen(dupepath, "r+b")) == NULL) {
    printf("Unable to open %s\n", dupepath);
    seteuid(oldid);
    return 0;
  }
  while (!feof(dfile)) {
    if (fread(&dupeentry, sizeof(struct dupefile), 1, dfile) < 1)
      continue;
  }
  dupeentry.timeup = time(NULL);
  strncpy(dupeentry.uploader, uploader, 23);
  strncpy(dupeentry.filename, filename, 254);
  fwrite(&dupeentry, sizeof(struct dupefile), 1, dfile);
  fflush(dfile);
  fclose(dfile);
  seteuid(oldid);
  printf("File successfully added.\n");
  return 0;
}
