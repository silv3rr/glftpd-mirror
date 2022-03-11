/* V2.1 by Usurper, March 2002
   - Added option -d
   - Fixed confusing/bad help screen
*/

/*****************************
	Default includes
 *****************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "glconf.h"

/**************************
	Localized variables
 **************************/
static 	char	SearchString[256];
static	char	LogfileName[256];
static	int	WhatToDo = 0;
static	int	MaxMatches;
static  int     Display = 0;
int maxdays = 9999;
time_t curtime;

enum {
        DO_FINDENTRY = 1,
        DO_NEWEST,
	DO_UNNUKES
};

char rootpath[MAXPATHLEN];
char datapath[MAXPATHLEN]; 
char config_file[MAXPATHLEN];
char dhmbuf[20];

void list_nukes(int howmany);
void list_unnukes(int howmany);
void load_nukelogs( void );
char *find_tag(char *user);
void find_dir(char *string);
char *strcasenewstr(register char *s, register char *find);

/****************************************************************************
	HMS_FORMAT
*****************************************************************************/
void hms_format(time_t dirtime)
{

 	time_t timenow = time(NULL);
	time_t difftime;
	int days = 0;
	int hours = 0;
	int minutes = 0;
	int seconds = 0;

	difftime = timenow - dirtime;

	while(difftime >= (time_t)86400)
	{
		difftime -= (time_t)86400;
		days++;
	}
	while (difftime >= (time_t)3600)
	{
		difftime -= (time_t)3600;
		hours++;
	}
	while(difftime >= (time_t)60)
	{
		difftime -= (time_t)60;
		minutes++;
	}
	seconds = (int)difftime;
	if (days != 0)
		sprintf(dhmbuf, "%2dd %2dh", days, hours);
	else if (hours != 0)
		sprintf(dhmbuf, "%2dh %2dm", hours, minutes);
	else
		sprintf(dhmbuf, "%2dm %2ds", minutes, seconds);
}

/***************************************************************************
        TRIM
***************************************************************************/

char *
trim( char *str )
{
        char *ibuf;
        char *obuf;

   if ( str )
   {
        for ( ibuf = obuf = str; *ibuf; )
      {
                while ( *ibuf && ( isspace ( *ibuf ) ) )
                ibuf++;
                if ( *ibuf && ( obuf != str ) )
            *( obuf++ ) = ' ';
                while ( *ibuf && ( !isspace ( *ibuf ) ) )
            *( obuf++ ) = *( ibuf++ );
      }
      *obuf = '\0';
   }
   return( str );
}
/*-- end of trim() -------------------------------------------------------*/

/***************************************************************************
        LOAD_SYSCONFIG
        Loads data from system configuration file.
***************************************************************************/

void
load_sysconfig( void )
{
   FILE *configfile;
   char lvalue[ 64 ];
   char rvalue[ MAXPATHLEN ];
   int  x, y;
   char work_buff[ MAXPATHLEN ];

 sprintf( work_buff, "%s", config_file);

   if ( ( configfile = fopen( work_buff, "r" ) ) == NULL )
        {
                fprintf( stderr, "bad or missing 'CONFIG FILE', using defaults\n");
		      exit(1);
        }

   while ( 1 )
   {
      if ( fgets( work_buff, sizeof( work_buff ), configfile ) == NULL )
      {
         fclose( configfile );

         return;
      }

      /* Clip out comments */
      for ( x = 0; x < (int)strlen( work_buff ); x++ )
         if ( work_buff[ x ] == '#' )
            work_buff[ x ] = '\0';

                /* Trim */
                (void)trim( work_buff );

      /* Clear out old values */
      memset( lvalue, '\0', sizeof( lvalue ) );
      memset( rvalue, '\0', sizeof( rvalue ) );

                /* The file allows spaces embedded, but clears leading & trailin spaces */

      /* Parse lvalue */
      y = 0;
      for ( x = 0; x < (int)strlen( work_buff ) && work_buff[ x ] != ' '; x++ )
         if ( isprint( work_buff[ x ] ) )
            lvalue[ y++ ] = work_buff[ x ];

      /* Parse rvalue */
      y = 0; x++;
      for ( ; x < (int)strlen( work_buff ); x++ )
         if ( isprint( work_buff[ x ] ) )
            rvalue[ y++ ] = work_buff[ x ];

      if ( strcasecmp( lvalue, "datapath" ) == 0 )
                strncpy( datapath, rvalue, sizeof( datapath ) );
      if ( strcasecmp( lvalue, "rootpath" ) == 0 )
                strncpy( rootpath, rvalue, sizeof( rootpath ) );
   }

   return;
}
/*-- end of load_sysconfig() ----------------------------------------------*/

int main(int argc, char *argv[]) {
	int		ArgCounter,EntryToView=0;


	curtime = time(NULL);
	MaxMatches = 10;
	strcpy(config_file, GLCONF);

	if (argc < 2) {
		printf("GLFTPD NukeLog Scanner v2.1\n\n");
		printf("Syntax :\n");
		printf("NukeLogScanner [-f{text}] [-n{nr}] %s",
			"[-u{nr}] [-d{nr}] [-m{nr}] [-r{path}]\n\n");
		printf("* Options f, n and u, if capital, will use different display format *\n");
		printf("-n or -N will list the last {nr} Nukes.\n");
		printf("-u or -U will list the last {nr} UnNukes.\n");
		printf("-f or -F{text} will search for the {text} parameter in the nukefile\n");
		printf("-m{nr} will return {nr} maximum of matches (Default 10)\n");
		printf("-d{nr} will only return matches for up to {nr} days ago\n");
		printf("-r{%s} Used to select alternate config file\n", GLCONF);
		return(0);
	}

	ArgCounter = 1;
	while (argc > ArgCounter) {
		if (argv[ArgCounter][0] != '-') {	
			printf("Unknown option, skipping [%s]\n",argv[ArgCounter]);
		} else {
			switch ((int)argv[ArgCounter][1]) {
				case 'r':
					strcpy(config_file, &argv[ArgCounter][2]);
					break;
				case 'n':
					WhatToDo = DO_NEWEST;
					EntryToView = atoi(&argv[ArgCounter][2]);
					break;
				case 'N':
					WhatToDo = DO_NEWEST;
					EntryToView = atoi(&argv[ArgCounter][2]);
					Display = 1;
					break;
				case 'u':
					WhatToDo = DO_UNNUKES;
					EntryToView = atoi(&argv[ArgCounter][2]);
					break;
				case 'U':
					WhatToDo = DO_UNNUKES;
					EntryToView = atoi(&argv[ArgCounter][2]); 
					Display = 1;
					break;
				case 'f':
					strcpy(SearchString,&argv[ArgCounter][2]);
					printf("SearchString set to : '%s'\n",SearchString);
					WhatToDo = DO_FINDENTRY;
					break;
				case 'F':
					strcpy(SearchString,&argv[ArgCounter][2]);
					WhatToDo = DO_FINDENTRY;
					Display = 1;
					break;
				case 'm':
					MaxMatches = atoi(&argv[ArgCounter][2]);
					if (MaxMatches < 1) MaxMatches = 1;
					if (MaxMatches > 100) MaxMatches = 100;
					break;
				case 'd':
					maxdays = atoi(&argv[ArgCounter][2]);
					if (maxdays < 1) maxdays = 99999;
					break;
				default:
					printf("Unknown option, skipping [%s]",argv[ArgCounter]);
					break;
			}
		}
		ArgCounter++;
	}

        load_sysconfig();
        sprintf(LogfileName,"%s/%s/logs/nukelog",rootpath, datapath);

	switch (WhatToDo) {
		case DO_NEWEST:
			(void)list_nukes(EntryToView);
			break;
	  	case DO_FINDENTRY:
			(void)find_dir(SearchString);
			break;
		case DO_UNNUKES:
			(void)list_unnukes(EntryToView);
			break;
	}
	return 0;
}
/***************************************************************************
   SHOW_NUKES
***************************************************************************/

void
list_nukes(int howmany )
{
   struct nukelog npath;
   FILE  *file;
   int    shown = 0;
   char   actname[255];
   char   tempbuf2[MAXPATHLEN];
   char   nukelog[MAXPATHLEN];

        if ( howmany < 1 )
        	howmany = 10;


   sprintf(nukelog, "%s%s/logs/nukelog",rootpath, datapath);    
   if ((file = fopen(nukelog, "rb")) == NULL) {
   	printf("Unable to open %s\n", nukelog);
	exit(1);
   }
   fseek(file, 0L, SEEK_END);
   while ( shown < howmany )
   {
      if (fseek(file, -(sizeof(struct nukelog)), SEEK_CUR) != 0)
      	break;
      if (fread(&npath, sizeof(struct nukelog), 1, file) < 1)
      	break;
      else
      	fseek(file, -(sizeof(struct nukelog)), SEEK_CUR);
      if (npath.status != 0)
      	continue;

      if (npath.nuketime < (curtime - (maxdays * 60 * 60 * 24)))
        continue;

      shown++;

      hms_format( npath.nuketime );
   if (!Display) {  
      if (strrchr(npath.dirname, '/') == NULL)
                strncpy(actname, npath.dirname, sizeof(actname));
      else
                strncpy(actname, strrchr(npath.dirname, '/') + sizeof(char), sizeof(actname));

      sprintf( tempbuf2, "%-9.9s %-9.9s %2dx %5.1fM  %-7.7s  %-10.10s %-s",                      
	      	npath.nuker, npath.nukee, npath.mult, (float)npath.bytes, dhmbuf, npath.reason, actname);
      printf("%s\n", tempbuf2 );
   } else {
      printf("%-12.12s %-12.12s  %2dx %5.1f   %-60.60s\n", npath.nuker, npath.nukee, 
      	npath.mult, (float)npath.bytes, npath.reason);
      printf("%s\n", npath.dirname);
   }
   }
   fclose(file);
}

/***************************************************************************
   SHOW_UNNUKES
***************************************************************************/

void list_unnukes( int howmany )
{
   struct nukelog npath;
   FILE   *file;
   int    shown = 0;
   char   actname[MAXPATHLEN];
   char   tempbuf2[MAXPATHLEN];
   char   nukelog[MAXPATHLEN];

   if ( howmany < 1 )
       	howmany = 10;
	
   
   sprintf(nukelog, "%s%s/logs/nukelog",rootpath, datapath);    
   if ((file = fopen(nukelog, "rb")) == NULL) {
   	printf("Unable to open %s\n", nukelog);
	exit(1);
   }
   fseek(file, 0L, SEEK_END);
   while ( shown < howmany )
   {
    	if (fseek(file, -(sizeof(struct nukelog)), SEEK_CUR) != 0)
		break;
	if (fread(&npath, sizeof(struct nukelog), 1, file) < 1)
		break;
	else
		fseek(file, -(sizeof(struct nukelog)), SEEK_CUR);
	if (npath.status != 1)
		continue;

      if (npath.nuketime < (curtime - (maxdays * 60 * 60 * 24)))
        continue;

      shown++;

      hms_format( npath.nuketime );
   if (!Display) {
      if (strrchr(npath.dirname, '/') == NULL)
                strncpy(actname, npath.dirname, sizeof(actname));
      else
                strncpy(actname, strrchr(npath.dirname, '/') + sizeof(char), sizeof(actname));

      sprintf( tempbuf2, "%-9.9s %-9.9s %2dx %5.1fM  %-7.7s  %-10.10s %-s",
             npath.nuker, npath.nukee, npath.mult, (float)npath.bytes, dhmbuf, npath.reason, actname);     
      printf("%s\n", tempbuf2 );
   } else {
        strncpy(actname, npath.dirname, sizeof(actname));
   	printf("%s   %s   %2dx %5.1f    %s\n", npath.nuker, npath.nukee,
		npath.mult, (float)npath.bytes, npath.reason); 
	printf("%s\n", actname);
   }
   }
   fclose(file);
}

/***************************************************************************
	FIND_DIR
***************************************************************************/
void
find_dir(char *string)
{
	FILE *file;
	struct nukelog log;
	char tempbuf2[MAXPATHLEN];
	int num = 0;
	int x;	


	for (x=0; x<(int)strlen(string); x++)
		string[x] = toupper(string[x]);

	file = fopen(LogfileName, "rb");
	if (file == NULL)
	{
		fprintf(stderr, "Unable to open %s\n",LogfileName);
		exit(1);
	}
	fseek(file, -(sizeof(struct nukelog)), SEEK_END);
	while (1)
	{
		fread(&log, sizeof(struct nukelog), 1, file);
		if (strcasenewstr(log.dirname, string) &&
		   log.nuketime > (curtime - (maxdays * 60 * 60 * 24))) {
		   num++;
		   hms_format(log.nuketime);	
		   if (!Display) {
                   sprintf( tempbuf2, "%-9.9s %-9.9s %2dx %5.1fM  %-7.7s  %-10.10s %-s",
		   	log.nuker, log.nukee, log.mult, (float)log.bytes, dhmbuf, log.reason, log.dirname);
		   printf("%s\n",tempbuf2);
		   } else {
		   printf("%-12.12s %-12.12s  %2dx %5.1f   %s\n", log.nuker, log.nukee, 
			log.mult, (float)log.bytes, log.reason);
		   printf("%-s\n", log.dirname);
		   } 		   	           
      		   if (num == MaxMatches)
			break;
		} 
		x = fseek(file, -(sizeof(struct nukelog)*2), SEEK_CUR);
		if (x == -1)
		   break;
	}
	fclose(file);
	exit(0);
}
		
/**************************************************************************
	strcasenewstr
***************************************************************************/
char *
strcasenewstr(register char *s, register char *find)
{
        register char c,
          sc;
        register size_t len;

        if ((c = *find++) != 0) {
                len = strlen(find);
                do {
                        do {
                                if ((sc = *s++) == 0)
                                        return (NULL);
                        } while (sc != c);
                } while (strncasecmp(s, find, len) != 0);
                s--;
        }
        return ((char *) s);
}


