/*
  Dupelist.C - list text output of dupefile
  02-24-99
  evilution @ EFnet

  psxc August 2006
  - Copied some better code from glupdate.c to read the variables in the
    glftpd.conf - spaces and comments should now be parsed correctly.
  - fix some formatting and a warning
  - print some debug info
*/

#include <sys/param.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "glconf.h"

char *
trim (char *str)
{
	char *ibuf;
	char *obuf;

	if (str)
	  {
		  for (ibuf = obuf = str; *ibuf;)
		    {
			    while (*ibuf && (isspace (*ibuf)))
				    ibuf++;
			    if (*ibuf && (obuf != str))
				    *(obuf++) = ' ';
			    while (*ibuf && (!isspace (*ibuf)))
				    *(obuf++) = *(ibuf++);
		    }
		  *obuf = '\0';
	  }
	return (str);
}

/*
   Loads data from system configuration file.
*/
char * read_conf_datapath ( char *fullpath, char *config_file ) {
	FILE *configfile;
	char lvalue[64];
	char rvalue[MAXPATHLEN];
	char rootpath[MAXPATHLEN];
	char datapath[MAXPATHLEN];
	int x, y;
	char work_buff[MAXPATHLEN];

	strncpy (work_buff, config_file, MAXPATHLEN);

	if ((configfile = fopen (work_buff, "r")) == NULL) {
		fprintf(stderr, "Bad or missing config file (%s), using defaults\n", config_file);
		strcpy(fullpath, "/glftpd/ftp-data");
		return fullpath;
	}

	for (;;) {
		if (fgets(work_buff, sizeof(work_buff), configfile) == NULL) {
			fclose(configfile);
			strcat(fullpath, rootpath);
			strcat(fullpath, datapath);
			return fullpath;
		}

		/* Clip out comments */
		for (x = 0; x < (int)strlen (work_buff); x++)
			if (work_buff[x] == '#')
				work_buff[x] = '\0';

		/* Trim */
		trim(work_buff);

		/* Clear out old values */
		memset (lvalue, '\0', sizeof (lvalue));
		memset (rvalue, '\0', sizeof (rvalue));

		/* Parse lvalue */
		y = 0;
		for (x = 0; x < (int)strlen (work_buff) && work_buff[x] != ' '; x++)
			if (isprint (work_buff[x]))
				lvalue[y++] = work_buff[x];

		/* Parse rvalue */
		y = 0;
		x++;
		for (; x < (int)strlen (work_buff); x++)
			if (isprint (work_buff[x]))
				rvalue[y++] = work_buff[x];

		if (strcasecmp (lvalue, "datapath") == 0)
			strncpy (datapath, rvalue, sizeof (datapath));
		if (strcasecmp (lvalue, "rootpath") == 0)
			strncpy (rootpath, rvalue, sizeof (rootpath));
	}
	strcat(fullpath, rootpath);
	strcat(fullpath, datapath);
	return fullpath;
}


int main (void) {
  FILE *fp;
  char dupefile[MAXPATHLEN], Temp[MAXPATHLEN];
  struct dupefile buffer;
    
  read_conf_datapath(Temp, GLCONF);
  sprintf(dupefile, "%s/logs/dupefile", Temp);
  
  if((fp = fopen(dupefile, "r")) == NULL) {
    printf("Failed to open dupefile (%s)\n", dupefile);
    return 0;
  }

  while (!feof(fp)) {
    if (fread(&buffer, sizeof(struct dupefile), 1, fp) < 1)
      break;
//    printf("%-30s\t%-20ld\t%-25s\n", buffer.filename,(long int)buffer.timeup,buffer.uploader);
    printf("%s\t%ld\t%s\n", buffer.filename,(long int)buffer.timeup,buffer.uploader);
  }
  fclose(fp);
  return 0;
}
