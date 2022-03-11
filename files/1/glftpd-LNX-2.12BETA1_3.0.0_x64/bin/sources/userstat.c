#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/file.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "glconf.h"

char * read_conf_datapath ( char *rootpath, char *confpath ) {
  FILE *fp;
  char datapath[255];
  char temp[255];
  if((fp = fopen(confpath, "r")) == NULL)
    return 0;

  while (fscanf(fp, "%s", temp) == 1) {
    if (strcmp(temp, "rootpath") == 0) {
      fscanf(fp, "%s", rootpath);
    }
    if (strcmp(temp, "datapath") == 0) {
      fscanf(fp, "%s", datapath);
      strcat(rootpath, datapath);
      return rootpath;
    }
  }
fclose(fp);
return 0;
}

void hms_format(time_t dirtime, char *dhmbuf, int x)
{
        time_t timenow = time(NULL);
        time_t difftime;
        int days = 0;
        int hours = 0;
        int minutes = 0;
        int seconds = 0;

	if (x == 0) {
	  difftime = timenow - dirtime;
	} else { difftime = dirtime; }

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

FILE * general ( FILE *fp ) {
  int ratio=0, maxlogins=0, lc = 0;
  double wkalot=0, credits=0;
  char temp[50];
  
  while (lc < 4) {
    switch(lc) {
    case 0: fscanf(fp, "%s", temp); ratio = atoi(temp); break;
    case 1: fscanf(fp, "%s", temp); wkalot = atoi(temp); wkalot = wkalot/1024; break;
    case 2: fscanf(fp, "%s", temp); maxlogins = atoi(temp); break;
    case 3: fscanf(fp, "%s", temp); credits = atoi(temp); credits = credits/1024; break;
    }
  lc++;
  }
  printf("RATIO: %d\nWEEKLY ALOTMENT: %f\nMAX LOGINS: %d\nCREDITS: %f\n", ratio, wkalot, maxlogins, credits);
  fflush(fp);
  return(fp);
}
  
FILE *unfo ( FILE *fp ) {
  char usernfo[50] = "", temp[50] = "";
  fflush(fp);
  
  while (!feof(fp)) {
    fscanf(fp, "%s", temp);
    if(strcmp(temp, "CREDITS") == 0)
      break;
    strcat(usernfo, temp);
    sprintf(usernfo, "%s ", usernfo);
  }
  printf("UNFO: %s\n", usernfo);
  return(fp);
}

FILE *transinfo( FILE *fp, char *use) {
  int numfile=0, lc = 0;
  char temp[50];
  double bytes=0;
  double time=0;
  printf("------------------[%10s]---------------------\n", use);
  while (lc < 3) {
    switch(lc) {
    case 0: fscanf(fp, "%s", temp); numfile = atoi(temp); break;
    case 1: fscanf(fp, "%s", temp); bytes = atoi(temp); break;
    case 2: fscanf(fp, "%s", temp); time = atoi(temp); break;
    }
    lc++;
  }
  printf("%s # Files: %d\n%s Kb: %f\n%s K/s: %f\n",use, numfile,use, (bytes/1024),use, (bytes/time));
  return (fp);
}

FILE *nuked( FILE *fp ) {
  double time=0;
 
  int numnuked=0, mbnuked=0, lc = 0;
  char dhm[20], temp[50];

  while (lc < 3) {
    switch(lc) {
    case 0: fscanf(fp, "%s", temp); time = atoi(temp); break;
    case 1: fscanf(fp, "%s", temp); numnuked = atoi(temp); break;
    case 2: fscanf(fp, "%s", temp); mbnuked = atoi(temp); break;
    }
    lc++;
  }
  hms_format(time, dhm, 0);

  printf("LAST NUKED: %s\nNUM NUKED: %d\nMB NUKED: %d\n", dhm, numnuked, mbnuked);
  return(fp);
}

FILE *timeinfo( FILE *fp ) {
  double ontoday=0, laston=0;
  int numlogins=0, timelimit=0, lc = 0;
  char dhm[20], dhm2[20], temp[50];

  while (lc < 4) {
    switch(lc) {
    case 0: fscanf(fp, "%s", temp); numlogins = atoi(temp); break;
    case 1: fscanf(fp, "%s", temp); laston = atoi(temp); break;
    case 2: fscanf(fp, "%s", temp); timelimit = atoi(temp); break;
    case 3: fscanf(fp, "%s", temp); ontoday = atoi(temp); break;
    }
    lc++;
  }

  hms_format(laston, dhm, 0);
  hms_format(ontoday, dhm2, 1);

  printf("NUM LOGINS: %d\nLASTON: %s\nTIME-LIMIT: %d\nONTODAY: %s\n", numlogins, dhm, timelimit, dhm2 );
  return(fp);
}

FILE *showIP( FILE *fp) {
  char usernfo[50] = "", temp[50] = "";
  fflush(fp);
  
  while (!feof(fp)) {
    fscanf(fp, "%s", temp);
    strcat(usernfo, temp);
    sprintf(usernfo, "%s ", usernfo);
  }
  printf("IP: %s\n", usernfo);
  return(fp);
}

FILE *showGrp( FILE *fp ) {
  char groups[50] = "", temp[50] = "", ips[1024] = "";
  fflush(fp);
  
  while (!feof(fp)) {
    fscanf(fp, "%s", temp);
    if(strcmp(temp, "IP") == 0)
      break;
    strcat(groups, temp);
    sprintf(groups, "%s ", groups);
  }
  while (!feof(fp)) {
    fscanf(fp, "%s", temp);
    strcat(ips, temp);
    sprintf(ips, "%s ", ips);
  }
  printf("IP: %s\n", ips);
  printf("GROUPS: %s\n", groups);
  return(fp);
}
  
int main ( int argc, char *argv[]) {
   FILE *fp;
   char config_file[255] = GLCONF;
   char datapath[255], userfile[255], temp[255], username[20];
   int x = 1;

   if (argc < 2) {
     printf("USAGE: %s [options] [user]\n", argv[0]);
     printf("OPTION: -r [alt-config]\n");
     return 1;
   }
  
   while ( argc > x ) {
    if (argv[x][0] != '-') { } else {
      switch ( argv[x][1] ) {
        case 'r':
	        strcpy(config_file, argv[x+1]);
	        break;
      }
    }
    x++;
   }
   
   read_conf_datapath(datapath, config_file);
   sprintf(userfile, "%s/users/%s", datapath, argv[argc-1]);
   
   printf("USERFILE: %s\n", userfile);
   if((fp = fopen(userfile, "r")) == NULL)
      return 0;
   
   while (fscanf(fp, "%s", temp) == 1 ) {
     if (strcmp(temp, "USER") == 0)
       fscanf(fp, "%s", username);
     if (strcmp(temp, "GENERAL") == 0)
       general(fp);
     if (strcmp(temp, "UNFO") == 0)
       unfo(fp);
     if (strcmp(temp, "ALLUP") == 0)
       transinfo(fp, "ALLUP");
     if (strcmp(temp, "ALLDN") == 0)
       transinfo(fp, "ALLDN");
     if (strcmp(temp, "WKUP") == 0)
       transinfo(fp, "WKUP");
     if (strcmp(temp, "WKDN") == 0)
       transinfo(fp, "WKDN");
     if (strcmp(temp, "DAYUP") == 0)
       transinfo(fp, "DAYUP");
     if (strcmp(temp, "DAYDN") == 0)
       transinfo(fp, "DAYDN");
     if (strcmp(temp, "MONTHUP") == 0)
       transinfo(fp, "MONTHUP");
     if (strcmp(temp, "MONTHDN") == 0)
       transinfo(fp, "MONTHDN");
     if (strcmp(temp, "NUKE") == 0)
       nuked(fp);
     if (strcmp(temp, "TIME") == 0)
       timeinfo(fp);
     if (strcmp(temp, "GROUP") == 0)
       showGrp(fp);
   }

fclose (fp);
return 0;
}
