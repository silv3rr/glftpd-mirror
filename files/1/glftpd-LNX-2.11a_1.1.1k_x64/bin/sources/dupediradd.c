/* Most of this is ripped from GreyLine's code from glftpd

Version 1.1 modified by iwdisb 2004-26-06

   - Will now accept the -r switch
   - Command line parsing with getopts()
     (ie: this version requires GNU getopts or a compatible implementation)
   - Compiles with "-pedantic"

*/

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/file.h>
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
        printf ("Usage: dupediradd [-r /pathto/glftpd.conf] dirname\n");
        exit (0);
}


int main(int argc, char *argv[]) {
  int fdes = -1, c;
  uid_t oldid = getuid();
  char dupelogpath[MAXPATHLEN], direntry[MAXPATHLEN], *confpath = GLCONF;
  char frame_buf[64];
  struct tm *ptr;
  time_t timenow = time(NULL);
  mode_t oldmask=0;

  printf("Dupediradd for glftpd, v1.1 by Usurper, June 1999\n");
  printf("This util adds a dirname to the dupe database.\n");

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

  if (argc != 1)
    usage();

  strcpy(rootpath, "/glftpd");
  strcpy(datapath, "/ftp-data");
  if (!readconf(confpath))
    printf("Couldn't read config file. Using defaults. Use option -r /path/config.file to override.\n");

  strcpy(dupelogpath, rootpath);
  strcat(dupelogpath, datapath);
  strcat(dupelogpath, "/logs/dupelog");

  ptr = (struct tm *)localtime(&timenow);
  strftime(frame_buf, 64, "%m%d%y",ptr);
  snprintf(direntry, sizeof(direntry), "%s %s\n",frame_buf, argv[0]);
  seteuid(0);
  fdes = open(dupelogpath, O_CREAT | O_WRONLY | O_APPEND, 0666);
  umask(oldmask);
  if (fdes < 0) {
    seteuid(oldid);
    printf("Error opening %s\n", dupelogpath);
    return 0;
  }
  write (fdes, direntry , strlen( direntry ));
  close (fdes);
  seteuid(oldid);
  printf("Directory successfully added.\n");
  return 0;
}
