/* Updated 1/9/2002 by Usurper - a bug with grppath was fixed */
/* New version updated to work with glftpd 1.16.5 */
/* minor fixes to get rid of compilation warnings by mCr */

/*****************************
    Default includes
 *****************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "glconf.h"

/**************************
    Localized variables
 **************************/
static  char    SearchString[256];
static  char    LogfileName[256];
static  int     WhatToDo = 0;
static  int     MaxMatches;
static  int     Display = 0;
static  char    pwd_name[BUFSIZ];
static  char    grp_name[BUFSIZ];
static  char    tagline[40];
static  char    datapath[255];
static  char    rootpath[255];
static  char    grppath[MAXPATHLEN];
static  char    pwdpath[MAXPATHLEN];

enum {
    DO_FINDENTRY = 1,
    DO_NEWEST,
    DO_LISTENTRY
};

char config_file[MAXPATHLEN];
char dhmbuf[20];

void show_newx(int howmany);
char *get_pwuid(uid_t userid);
char *get_grpid(gid_t grpid);
char *find_tag(char *user);
void find_dir(char *string);
void list_entry( void );
char *mystrcasestr(register char *s, register char *find);

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

    grppath[0] = '\0';
    pwdpath[0] = '\0';
      
    sprintf( work_buff, "%s", config_file);

    if ( ( configfile = fopen( work_buff, "r" ) ) == NULL )
        {
                fprintf( stderr, "bad or missing 'CONFIG FILE', using defaults\n");
              return;
        }

   while ( fgets( work_buff, sizeof( work_buff ), configfile ) != NULL )
   {

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
      if ( strcasecmp( lvalue, "pwd_path" ) == 0 ) 
                strncpy( pwdpath, rvalue, sizeof( pwdpath ) );
      if ( strcasecmp( lvalue, "grp_path" ) == 0 )
                strncpy( grppath, rvalue, sizeof( grppath ) );
   }

    fclose( configfile );            

    if (pwdpath[0] == '\0') 
        sprintf(work_buff,"%s/etc/passwd", rootpath);
    else
        sprintf(work_buff,"%s%s", rootpath, pwdpath);
    strncpy(pwdpath, work_buff, sizeof(pwdpath));
        
/*    printf("SET PWDPATH: %s\n",pwdpath); */
        
    if (grppath[0] == '\0') 
        sprintf(work_buff,"%s/etc/group", rootpath);
    else
        sprintf(work_buff,"%s%s", rootpath, grppath); 
    strncpy(grppath, work_buff, sizeof(grppath));
            
   return;
}
/*-- end of load_sysconfig() ----------------------------------------------*/

int main(int argc, char *argv[]) {
    int     ArgCounter,EntryToView=0;


    MaxMatches = 10;
    strcpy(config_file,GLCONF);

    if (argc < 2) {
        printf("GLFTPD DirLog Scanner v2.2b\n");
        printf("Syntax :\n");
        printf("DirLogScanner [-f{text}] [-n{nr}] [-N{nr}]\n\n");
        printf("-l or -L will list the LAST entry.\n");
        printf("-n or -N will list last {nr} directories.\n");
        printf("-f or -F{text} will search for the {text} parameter in the logfile\n");
        printf("-m{nr} will return {nr} maximum of matches (Default 10)\n");
        printf("-r{%s} Used to select alternate config file\n",GLCONF);
        return(0);
    }

    ArgCounter = 1;
    while (argc > ArgCounter) {
        if (argv[ArgCounter][0] != '-') {   
            printf("Unknown option, skipping [%s]\n",argv[ArgCounter]);
        } else {
            switch ((int)argv[ArgCounter][1]) {
                case 'l':
                    WhatToDo = DO_LISTENTRY;
                    break;
                case 'L':
                    WhatToDo = DO_LISTENTRY;
                    break;
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
                default:
                    printf("Unknown option, skipping [%s]",argv[ArgCounter]);
                    break;
            }
        }
        ArgCounter++;
    }

    load_sysconfig();
    sprintf(LogfileName,"%s/%s/logs/dirlog",rootpath, datapath);

    switch (WhatToDo) {
        case DO_NEWEST:
            (void)show_newx(EntryToView);
            break;
        case DO_FINDENTRY:
            (void)find_dir(SearchString);
            break;
        case DO_LISTENTRY:
            (void)list_entry();
            break;
    }
    return 0;
}
/***************************************************************************
   SHOW_NEWX
***************************************************************************/

void
show_newx( int howmany )
{
    struct dirlog npath;
    FILE   *file;
    char   *mkr;
    char   *grp;
    int    shown = 0;
    char   tempbuf[ 20 ];
    char   actname[255];
    char   tempbuf2[BUFSIZ];
    char   tempbuf3[BUFSIZ];
    char   maker[14];
    char   group[14];
    char   *tag;
      

    if ( howmany < 1 )
        howmany = 10;

    if ((file = fopen (LogfileName, "rb") )== NULL) {
        printf("Failed to open %s\n", LogfileName);
        exit(1);
    }
    fseek(file, 0L, SEEK_END);
    while ( shown < howmany )
    { 
        if (fseek(file, -(sizeof(struct dirlog)), SEEK_CUR) != 0)
            break;
        if (fread(&npath, sizeof(struct dirlog), 1, file) < 1)
            break;
        else
            fseek(file, -(sizeof(struct dirlog)), SEEK_CUR);
        if (npath.status == 3) 
            continue;
                 
        shown++;
        sprintf( tempbuf, "%3.1fM", (double)( npath.bytes / 1024000.0L ) );
    
        hms_format( npath.uptime );
        
        if ((mkr = get_pwuid(npath.uploader)) == NULL)
            sprintf(maker,"%i",npath.uploader);
        else {
            strncpy(maker,mkr,sizeof(maker) );
            if ((tag = find_tag(maker)) == NULL)
                sprintf(tagline, "%s", "No TagLine Set");
            else
                strncpy(tagline, tag, sizeof(tagline));
        }
        
        if ((grp = get_grpid(npath.group)) == NULL)
            sprintf(group,"%i",npath.group);
        else   
            strncpy(group,grp,sizeof(group));
        
        if (npath.status == 1)
            sprintf(tempbuf3,"   *NUKED* ");
        else if (npath.files == 0)
            sprintf(tempbuf3,"   =Empty= ");
        else
            sprintf(tempbuf3,"%3dF%7.7s",npath.files, tempbuf);

        if (strrchr(npath.dirname, '/') == NULL)
            strncpy(actname, npath.dirname, sizeof(actname));
        else
            strncpy(actname, strrchr(npath.dirname, '/') + sizeof(char), sizeof(actname));

        if (!Display) {
            sprintf( tempbuf2, "[%2d] %s  %-10.10s %-s   %-s",
              shown, dhmbuf, maker, tempbuf3, actname );
            printf("%s\n", tempbuf2);
        }
        else {
            printf("%-12.12s %-12.12s %-30.30s\n",maker, group, tagline);
            printf("%s\n",npath.dirname);
        }
    }
    fclose(file);
}
/*-- bottom_list of show_newx() -------------------------------------------*/
  

/**************************************************************************
    GET_PWUID
**************************************************************************/
char *
get_pwuid(uid_t userid)
{
    FILE *pwd_file;
    char filebuff[BUFSIZ];
    char *tokptr;
    uid_t pwd_uid=0;
    char pwd_passwd[BUFSIZ];
    int x;

    pwd_file = fopen(pwdpath, "r");
    if (pwd_file == NULL) {
        printf("Unable to open pwdpath\n");
        return(NULL);
    }



    while (!feof(pwd_file))
    {
        if (fgets(filebuff, 127, pwd_file) == NULL && !feof(pwd_file)) {
            printf("Error reading pwd path from %s\n", pwdpath);
            break;
        }
        if (feof(pwd_file))
            break;
    
        for (x=0; x < (int)strlen(filebuff); x++)
            if (filebuff[x] == '#') 
                filebuff[x] = '\0';
       if (strlen(filebuff) < 1)
        continue;
        tokptr = strtok(filebuff, ":");
        if (tokptr)
            strncpy(pwd_name, tokptr, sizeof(pwd_name));
        tokptr = strtok('\0', ":");
        if (tokptr)
            strncpy(pwd_passwd, tokptr, sizeof(pwd_passwd));
        tokptr = strtok('\0', ":");
            if (tokptr) {
            pwd_uid = strtol(tokptr, (char **)NULL, 0);
            if (errno == ERANGE)
                continue;
        }
        if (pwd_uid == userid) {
            fclose(pwd_file);
            return (pwd_name);
        }       
    }        
    fclose(pwd_file);
    return(NULL);
}

/**************************************************************************
        GET_GRPID
**************************************************************************/
char *
get_grpid(gid_t grpid)
{
        FILE *grp_file;
        char filebuff[BUFSIZ];
        char *tokptr;
        uid_t grp_gid=0;
        char grp_desc[BUFSIZ];
    int x;

        grp_file = fopen(grppath, "r");
        if (grp_file == NULL) {
                printf("Unable to open grppath: %s\n", grppath);
                return(NULL);
        }
        while (!feof(grp_file))
        {
                if (fgets(filebuff, 127, grp_file) == NULL && !feof(grp_file)) {                       
            printf("Error reading group path from %s\n", grppath);
                        break;
                }
                if (feof(grp_file))
                        break;
                
        for (x=0; x < (int)strlen(filebuff); x++)
            if (filebuff[x] == '#')
                filebuff[x] = '\0';
    if (strlen(filebuff) < 1)
        continue;
        tokptr = strtok(filebuff, ":");
                if (tokptr)
                        strncpy(grp_name, tokptr, sizeof(grp_name));
                if (tokptr[strlen(tokptr)+1] != ':') {
            tokptr = strtok('\0', ":");
                    if (tokptr)
                             strncpy(grp_desc, tokptr, sizeof(grp_desc));
                }
        tokptr = strtok('\0', ":");
                if (tokptr) {
                        grp_gid = strtol(tokptr, (char **)NULL, 0);
            if (errno == ERANGE)
                continue;
        }
                if (grp_gid == grpid) {
                        fclose(grp_file);
                        return (grp_name);
                }
        }
        fclose(grp_file);
        return(NULL);
}
/**************************************************************************
    FIND_TAG
***************************************************************************/
char *
find_tag(char *user)
{
    FILE *userfile;
    char usrpath[MAXPATHLEN];
    char filebuff[BUFSIZ];
    int x,y;
    char lvalue[64];
    char rvalue[MAXPATHLEN];

    sprintf(usrpath, "%s%s/users/%s", rootpath, datapath, user);
    userfile = fopen(usrpath, "r");
    if (userfile == NULL) 
        return(NULL);
     while (!feof(userfile))
     {
    if (fgets(filebuff, 127, userfile) == NULL && !feof(userfile)) 
        return(NULL);
    if (feof(userfile))
        break;
    
    
    for (x=0; x < (int)strlen(filebuff); x++)
        if (filebuff[x] == '#')
            filebuff[x] = '\0';

     
    (void)trim(filebuff);
    memset(lvalue, '\0', sizeof(lvalue));
    memset(rvalue, '\0', sizeof(rvalue));

        y = 0;
        for ( x = 0; x < (int)strlen( filebuff ) && filebuff[ x ] != ' '; x++ )
         if ( isprint( filebuff[ x ] ) )
            lvalue[ y++ ] = filebuff[ x ];

     
        y = 0; x++;
        for ( ; x < (int)strlen( filebuff ); x++ )
         if ( isprint( filebuff[ x ] ) )
            rvalue[ y++ ] = filebuff[ x ];

        if ( strcasecmp( lvalue, "TAGLINE" ) == 0 )  {
                strncpy( tagline, rvalue, sizeof( tagline ) );
        return(tagline);
        }
     }
return(NULL);
}
/***************************************************************************
        LIST_ENTRY
***************************************************************************/
void
list_entry(void)
{
        FILE *file;
        struct dirlog npath;
    struct dirlog npath_test;
        time_t uptime = 0L;
    char tempbuf[BUFSIZ];
        char tempbuf2[MAXPATHLEN];
        char tempbuf3[BUFSIZ];
 

    if ((file = fopen(LogfileName, "rb")) == NULL) {
                printf("Unable to open logfile: %s\n",LogfileName);
                exit(1);
        } 
    while (!feof(file))
        {   
            fread(&npath_test, sizeof(struct dirlog), 1, file);
        if (npath_test.uptime > uptime) {
            npath = npath_test;
            uptime = npath_test.uptime;
        }
    }   
        sprintf( tempbuf, "%3.1fM",
        (double)( npath.bytes / 1024000.0L ) );
        hms_format( npath.uptime );
        if (npath.status == 1)
              sprintf(tempbuf3,"  NUKE     ");
        else if (npath.status == 2)
              sprintf(tempbuf3,"  UNNUKE   ");
        else if (npath.status == 3)
              sprintf(tempbuf3,"  DELETE   ");
        else
              sprintf(tempbuf3,"  NEWDIR   ");
        sprintf( tempbuf2, "%s  %-s   %-s", dhmbuf, tempbuf3, npath.dirname );
        printf("%s\n", tempbuf2);
        fclose(file);
        exit(0);
}


/***************************************************************************
    FIND_DIR
***************************************************************************/
void
find_dir(char *string)
{
    FILE *file;
    struct dirlog log;
    char tempbuf[BUFSIZ];
    char tempbuf2[MAXPATHLEN];
    char tempbuf3[BUFSIZ];
    char *mkr;
    char *grp;
    char *tag;
    char group[24];
    char maker[24];
    int num = 0;
    int x;  

    for (x=0; x<(int)strlen(string); x++)
        string[x] = toupper(string[x]);

    file = fopen(LogfileName, "rb");
    if (file == NULL)
    {
        printf("Unable to open logfile: %s\n",LogfileName);
        exit(1);
    }
    if (fseek(file, -(sizeof(struct dirlog)), SEEK_END) == -1) {
        fclose(file);
        exit(0);
    }
    while (1)
    {
        if (fread(&log, sizeof(struct dirlog), 1, file) == 0)
            break;        
        if (mystrcasestr(log.dirname, string) != NULL) {
            num++;
            sprintf( tempbuf, "%3.1fM", 
              (double)( log.bytes / 1024000.0L ) );
            hms_format( log.uptime );
 
            if ((mkr = get_pwuid(log.uploader)) == NULL)
                sprintf(maker,"%i",log.uploader);
            else {
                strncpy(maker,mkr,sizeof(maker) );
                if ((tag = find_tag(maker)) == NULL) 
                    sprintf(tagline, "%s", "No Tagline Set"); 
                else
                    strncpy(tagline, tag, sizeof(tagline));
            }
            if ((grp = get_grpid(log.group)) == NULL)
                sprintf(group,"%i",log.group); 
            else
                strncpy(group,grp,sizeof(group));  
            if (log.status != 3) {  
            if (!Display) {     
                sprintf(tempbuf3,"%3dF%7.7s",log.files, tempbuf);
                if (log.status == 1)
                    sprintf(tempbuf3,"   *NUKED* ");
                sprintf( tempbuf2, "%s  %-10.10s %-s   %-s",
                  dhmbuf, maker, tempbuf3, log.dirname );
                printf("%s\n", tempbuf2);
            } else {
                printf("%-12.12s %-12.12s %-30.30s\n",
                  maker, group, tagline);  
                printf("%-s\n", log.dirname);
           }
           if (num == MaxMatches)
            break;
         }
        } 
        if ((x = fseek(file, -(sizeof(struct dirlog)*2), SEEK_CUR)) == -1)
           break;
    }
    fclose(file);
    exit(0);
}
        
/**************************************************************************
    STRCASESTR
***************************************************************************/
char *
mystrcasestr(register char *s, register char *find)
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


