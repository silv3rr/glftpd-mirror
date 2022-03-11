/** UnDUPE.C - Clean the dupefile of unwanted data
 ** Works as far as i know
 ** evilution @ efnet
 ** 02-25-99
 **
 ** Fixed USAGE spelling error - Turranius 2004-01-04
 **
 ** psxc August 2006
 ** Copied some better code from glupdate.c to read the variables in the
 ** glftpd.conf - spaces and comments should now be parsed correctly.
 ** Added '-f' as default - ie, 'undupe file.rar' should now work.
 ** Added some debug output.
 ** Added better (internal) handling when renaming the dupefile.
 **/

#include <sys/param.h>

#include <ctype.h>
#include <strings.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "glconf.h"

#define DELETE_USER 1
#define PERGE_FILE 2
#define OLDER_THAN 3

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
 
int main (int argc, char *argv[]) {
  /* Variables */
  FILE *fp, *fp2;
  char dupename[MAXPATHLEN], datapath[MAXPATHLEN], 
       data2[MAXPATHLEN], data3[MAXPATHLEN], dupefile[MAXPATHLEN];
  
  int days = 0;
  time_t timeinfo = time( NULL );
  /* setting default config file */
  char config_file[255] = GLCONF;
  
  struct dupefile buffer;
  int x, toDo=0, doneSomething=0;
  
  /* User better supply a few options */
  
  if ( argc < 2 ) {
    printf("USAGE: %s [switch]\n", argv[0]);
    printf("Switches: -f [filename]\n");
    printf("\t  -u [username]\n");
    printf("\t  -r [alt-config]\n");
    printf("\t  -d [days old]\n");
    return 0;
  }
  x = 1;

  /* Command line parsing */
  while ( argc > x ) {

    if (argv[x][0] != '-') { /* nothing here */
	toDo = PERGE_FILE;
	strcpy(dupename, argv[x]);
	printf("Trying to clear %s \n", argv[x]);
    } else {
      if (x+1>=argc) { printf("incorrect syntax\n");return 1; }

      switch ( argv[x][1] ) {
        case 'r':
	        strcpy(config_file, argv[x+1]);
	        x++;
	        break;
        case 'u':
                if (toDo != 0) { printf("incorrect syntax\n");return 1; }
	        toDo = DELETE_USER;
	        strcpy(dupename, argv[x+1]);
		printf("Clearing DUPEFILE of %s \n", argv[x+1]);
		x++;
		break;
        case 'f':
                if (toDo != 0) { printf("incorrect syntax\n");return 1; }
                toDo = PERGE_FILE;
		strcpy(dupename, argv[x+1]);
		printf("Trying to clear %s \n", argv[x+1]);
		x++;
	        break;
        case 'd':
                if (toDo != 0) { printf("incorrect syntax\n");return 1; }
	        toDo = OLDER_THAN;
		days = atoi(argv[x+1]);
		x++;
		break;

        default:
	        printf("Error, unknown switch\n");
	        return 1;
	        break;
      }
    }
    x++;
  }
  
  if (toDo == 0) {
   printf("nothing to do\n");
   return 0;
  }
  
  /* Get data from the conf file                         */

  read_conf_datapath(datapath, config_file);
  sprintf(dupefile, "%s/logs/dupefile", datapath);
  printf("Using dupefile: %s\n", dupefile);
  
  /* This is my kludge to update the file. Just write a new one. Copy it over the old */
  
  sprintf(data2, "%s.bak", dupefile);
  sprintf(data3, "%s.bak2", dupefile);
  
  
  /* Opening the data files */
  if((fp = fopen(dupefile, "r+b")) == NULL) {
    printf("Unable to open dupefile");
    return 0;
  }
  if((fp2 = fopen(data2, "w+b")) == NULL) {
    printf("Unable to open dupefile2");
    return 0;
  }

  /* If you wanna purge a file, do it here */

  if (toDo == PERGE_FILE) {  
    /* While the file is still open */
    while (!feof(fp)) {
      
      if (fread(&buffer, sizeof(struct dupefile), 1, fp) < 1)
	break;
      /* If we found the file, delete it */
      if (strcmp(buffer.filename, dupename) == 0) {
	fflush(fp);
	printf("Dupe! Clearing!!\n");
	doneSomething=1;
      }
      /* if not, write it to the new file */
      if (strcmp(buffer.filename, dupename) != 0)
	if (fwrite(&buffer, sizeof(struct dupefile), 1, fp2) < 1)
	  break;
    }
  }
  
  /* this code is the same, cept looking for user, not file. */
  if (toDo == DELETE_USER) {
    while(!feof(fp)) {
      if (fread(&buffer, sizeof( struct dupefile), 1, fp) < 1)
	break;
      if (strcmp(buffer.uploader, dupename) == 0) {
	fflush(fp);
	printf(".");
	doneSomething=1;
      }
      if (strcmp(buffer.uploader, dupename) != 0) {
	if (fwrite(&buffer, sizeof(struct dupefile), 1, fp2) < 1)
	  break;
      }
    }
  }
 
  if (toDo == OLDER_THAN ) {
    while(!feof(fp)) {
      if (fread(&buffer, sizeof( struct dupefile), 1, fp) < 1)
	break;
      if ( buffer.timeup <  (timeinfo - 86400*days) ) {
	fflush(fp);
	printf(".");
	doneSomething=1;
      } else {
	if (fwrite(&buffer, sizeof(struct dupefile), 1, fp2) < 1)
	  break;
      }
    }
  }

  fclose(fp);
  fclose(fp2);
  /* My HUGE cludge that works quite well.                                       */
  /* Read in the old data file, write to a new data file.                        */
  /* After everything is done, and sucessful. Move the new one over the old one. */
  /* Hey, it works.                                                              */
 if (doneSomething) {
  if (rename(dupefile, data3) != 0) {
   printf("Error backuping old dupefile! Returning FAILED\n");
   return 1;
  }
  if (rename(data2, dupefile) != 0) {
   printf("Error renaming new dupefile! Returning FAILED\n");
   return 1;
  }
  chmod(dupefile,0666);
  if (rename(data3, data2) != 0) {
   printf("Error renaming old dupefile! Returning FAILED\n");
   return 1;
  }
  chmod(data2,0666);
 } else {
  printf("Sorry, nothing found. Try, try again.\n");
 }
 printf("\n");
 return 0;
}

