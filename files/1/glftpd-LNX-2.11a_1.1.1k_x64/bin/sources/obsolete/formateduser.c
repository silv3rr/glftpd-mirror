#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/file.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>


struct userfile {
  char username[20];
  int ratio, maxlogins;
  double weekalot, credits;
  char unfo[50];
  int allup_num, alldn_num, wkup_num, wkdn_num, dayup_num, daydn_num, monthup_num, monthdn_num;
  double allup_byte, alldn_byte, wkup_byte, wkdn_byte, dayup_byte, daydn_byte, monthup_byte, monthdn_byte;
  double allup_time, alldn_time, wkup_time, wkdn_time, dayup_time, daydn_time, monthup_time, monthdn_time;
  double nuke_time;
  int numnukes, mbnuked;
  int numlogins;
  double laston, timelim, ontoday;
  int slots;
  char groups[50];
  char ips[1024];
};

struct transtemp {
  int numup;
  double byte, time;
};

// GLOBAL!! I HATE THEM! but alas, for ease of code..

struct userfile buffer;
struct transtemp tempstruct;

char * read_conf_datapath ( char *rootpath, char *confpath ) {
  FILE *fp;
  char datapath[255];
  char temp[255];
  int size;
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

// GreyLines code mod'd a *bit* by me

char * hms_format(time_t dirtime, char *dhmbuf, int x)
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
  int ratio, maxlogins, lc = 0;
  double wkalot, credits;
  char temp[50];
  
  while (lc < 4) {
    switch(lc) {
    case 0: fscanf(fp, "%s", temp); ratio = atoi(temp); break;
    case 1: fscanf(fp, "%s", temp); wkalot = atoi(temp); break;
    case 2: fscanf(fp, "%s", temp); maxlogins = atoi(temp); break;
    case 3: fscanf(fp, "%s", temp); credits = atoi(temp); break;
    }
  lc++;
  }

  buffer.ratio = ratio;
  buffer.weekalot = wkalot;
  buffer.maxlogins = maxlogins;
  buffer.credits = credits;

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
  strcpy(buffer.unfo, usernfo);
  return(fp);
}

FILE *transinfo( FILE *fp, char *use) {
  int numfile, lc = 0;
  char temp[50];
  double bytes;
  double time;

  while (lc < 3) {
    switch(lc) {
    case 0: fscanf(fp, "%s", temp); numfile = atoi(temp); break;
    case 1: fscanf(fp, "%s", temp); bytes = atoi(temp); break;
    case 2: fscanf(fp, "%s", temp); time = atoi(temp); break;
    }
    lc++;
  }
 
  tempstruct.numup = numfile;
  tempstruct.byte = bytes;
  tempstruct.time = time;
 
  return (fp);
}

FILE *nuked( FILE *fp ) {
  double time;
 
  int numnuked, mbnuked, lc = 0;
  char temp[50];

  while (lc < 3) {
    switch(lc) {
    case 0: fscanf(fp, "%s", temp); time = atoi(temp); break;
    case 1: fscanf(fp, "%s", temp); numnuked = atoi(temp); break;
    case 2: fscanf(fp, "%s", temp); mbnuked = atoi(temp); break;
    }
    lc++;
  }
 
  buffer.nuke_time = time;
  buffer.numnukes = numnuked;
  buffer.mbnuked = mbnuked;
  return(fp);
}

FILE *timeinfo( FILE *fp ) {
  double ontoday, laston;
  int numlogins, timelimit, lc = 0;
  char temp[50];

  while (lc < 4) {
    switch(lc) {
    case 0: fscanf(fp, "%s", temp); numlogins = atoi(temp); break;
    case 1: fscanf(fp, "%s", temp); laston = atoi(temp); break;
    case 2: fscanf(fp, "%s", temp); timelimit = atoi(temp); break;
    case 3: fscanf(fp, "%s", temp); ontoday = atoi(temp); break;
    }
    lc++;
  }
  buffer.numlogins = numlogins;
  buffer.laston = laston;
  buffer.timelim = timelimit;
  buffer.ontoday = ontoday;
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
  strcpy(buffer.groups, groups);
  strcpy(buffer.ips, ips);
  return(fp);
}
  
int main ( int argc, char *argv[]) {
   FILE *fp;
   char config_file[255] = "/etc/glftpd.conf";
   char datapath[255], userfile[255], temp[255], temp2[255], temp3[255], username[20];
   int x = 1;

   if (argc < 2) {
     printf("USEAGE: %s [options] [user]\n", argv[0]);
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
   

   if((fp = fopen(userfile, "r")) == NULL)
      return 0;
   
   while (fscanf(fp, "%s", temp) == 1 ) {
     if (strcmp(temp, "USER") == 0)
       fscanf(fp, "%s", buffer.username);
     if (strcmp(temp, "GENERAL") == 0)
       general(fp);
     if (strcmp(temp, "UNFO") == 0)
       unfo(fp);
     if (strcmp(temp, "ALLUP") == 0) {
       transinfo(fp, "ALLUP");
       buffer.allup_num = tempstruct.numup;
       buffer.allup_byte = tempstruct.byte; 
       buffer.allup_time = tempstruct.time; 
     }
     if (strcmp(temp, "ALLDN") == 0) {
       transinfo(fp, "ALLDN");
       buffer.alldn_num = tempstruct.numup;
       buffer.alldn_byte = tempstruct.byte; 
       buffer.alldn_time = tempstruct.time;
     }
     if (strcmp(temp, "WKUP") == 0) {
       transinfo(fp, "WKUP");
       buffer.wkup_num = tempstruct.numup;
       buffer.wkup_byte = tempstruct.byte; 
       buffer.wkup_time = tempstruct.time;
     }
     if (strcmp(temp, "WKDN") == 0) {
       transinfo(fp, "WKDN");
       buffer.wkdn_num = tempstruct.numup;
       buffer.wkdn_byte = tempstruct.byte; 
       buffer.wkdn_time = tempstruct.time;
     }
     if (strcmp(temp, "DAYUP") == 0) {
       transinfo(fp, "DAYUP");
       buffer.dayup_num = tempstruct.numup;
       buffer.dayup_byte = tempstruct.byte; 
       buffer.dayup_time = tempstruct.time;
     }
     if (strcmp(temp, "DAYDN") == 0) {
       transinfo(fp, "DAYDN");
       buffer.daydn_num = tempstruct.numup;
       buffer.daydn_byte = tempstruct.byte; 
       buffer.daydn_time = tempstruct.time;
     }
     if (strcmp(temp, "MONTHUP") == 0) {
       transinfo(fp, "MONTHUP");
       buffer.monthup_num = tempstruct.numup;
       buffer.monthup_byte = tempstruct.byte; 
       buffer.monthup_time = tempstruct.time;
     }
     if (strcmp(temp, "MONTHDN") == 0) {
       transinfo(fp, "MONTHDN");
       buffer.monthdn_num = tempstruct.numup;
       buffer.monthdn_byte = tempstruct.byte; 
       buffer.monthdn_time = tempstruct.time;
     }
     if (strcmp(temp, "NUKE") == 0)
       nuked(fp);
     if (strcmp(temp, "TIME") == 0)
       timeinfo(fp);
     if (strcmp(temp, "GROUP") == 0)
       showGrp(fp);
   }
hms_format(buffer.nuke_time, temp, 0);
hms_format(buffer.laston, temp2, 0);
hms_format(buffer.ontoday, temp3, 1);

printf(".`,.-----. ---=--- -------.--- [USER INFO] =- .---= ----- -=--- --------';`.\n");
printf(": UserName: %-15s Ratio: 1:%d MaxLogins: %d Credits: %10.1fMB  :\n", buffer.username, buffer.ratio, buffer.maxlogins, (buffer.credits/1024) );
printf(": UNFO: %-50s                 :\n", buffer.unfo);
printf(": Idents: %-60s     :\n", buffer.ips);
printf(": Groups: %-50s               :\n", buffer.groups);
printf(": Times Nuked: %-3d        Nuked: %-3d MB       Last Nuked: %-10s's ago :\n", buffer.numnukes, buffer.mbnuked, temp);
printf(": Total Logins: %-3d       Laston: %-10s     Time On Today: %-10s :\n", buffer.numlogins, temp2, temp3);
printf(": -------==--- - --- --=--. [ All Time Stats ]==-- - ==- -=--.--- -- =-==--:\n");
printf(": UP-Number Files: %-5d for %8.2f MB at an average rate of %6.2f K/s  :\n", buffer.allup_num, (buffer.allup_byte/1024), (buffer.allup_byte/buffer.allup_time));
printf(": DN-Number Files: %-5d for %8.2f MB at an average rate of %6.2f K/s  :\n", buffer.alldn_num, (buffer.alldn_byte/1024), (buffer.alldn_byte/buffer.alldn_time));
printf(": ----=-----.=.-  ---==---=. [ Month Stats ]=- - ==---== -=--=----=---  --.:\n");
printf(": UP-Number Files: %-5d for %8.2f MB at an average rate of %6.2f K/s  :\n", buffer.monthup_num, (buffer.monthup_byte/1024), (buffer.monthup_byte/buffer.monthup_time));
printf(": DN-Number Files: %-5d for %8.2f MB at an average rate of %6.2f K/s  :\n", buffer.monthdn_num, (buffer.monthdn_byte/1024), (buffer.monthdn_byte/buffer.monthdn_time)); 
printf(": --- ---==--. - --- --=--`-.[ Weeks Stats ]-==.-- - ==- -=-- ------ =- =--:\n");
printf(": UP-Number Files: %-5d for %8.2f MB at an average rate of %6.2f K/s  :\n", buffer.wkup_num, (buffer.wkup_byte/1024), (buffer.wkup_byte/buffer.wkup_time));
printf(": DN-Number Files: %-5d for %8.2f MB at an average rate of %6.2f K/s  :\n", buffer.wkdn_num, (buffer.wkdn_byte/1024), (buffer.wkdn_byte/buffer.wkdn_time)); 
printf(": --- ---==--- -.--- --=--=--[ Today Stats ]==-- .--=- -=--, --- -- =-== --:\n");
printf(": UP-Number Files: %-5d for %8.2f MB at an average rate of %6.2f K/s  :\n", buffer.dayup_num, (buffer.dayup_byte/1024), (buffer.dayup_byte/buffer.dayup_time));
printf(": DN-Number Files: %-5d for %8.2f MB at an average rate of %6.2f K/s  :\n", buffer.daydn_num, (buffer.daydn_byte/1024), (buffer.daydn_byte/buffer.daydn_time));
printf(":---- -=-. =-  -----==- -=.,`' -=---. -- - .;`',.---=- - =-=  -=====- --== :\n");

fclose (fp);
return 0;
}


