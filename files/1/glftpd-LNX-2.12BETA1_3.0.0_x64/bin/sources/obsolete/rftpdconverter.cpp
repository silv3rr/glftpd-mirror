#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/fcntl.h>

struct userident {
	char ident[12];
	char ip[32];
};

struct userflags {
	bool disabled;
	bool nuke;
	bool kill;
	bool leech;
	bool siteop;
	bool color;
	bool beta;
	bool quiet;
	bool useredit;
	bool groupop;
};

struct userrecord {
	char username[12];
	char groupname[12];
	char tagline[40];
	long int ratio;
	unsigned short int uid, gid;
	long int todayupfile, todaydownfile, weekupfile, weekdownfile, allupfile, alldownfile;
	long int todayup, todaydown, weekup, weekdown, allup, alldown;
	long int todayupsec, todaydownsec, weekupsec, weekdownsec, allupsec, alldownsec;
	time_t lastlogin;
	long int logins, maxlogins, credits;
	struct userflags flags;  
	char ext[40];
	struct userident ident[10];
	char end[1];
};


int main ( int argc, char *argv[] ) 
{
	FILE *xf;
	int fdes = -1;
	int ct;
	char work_buff[1024];
	char work_buff2[1024];
        struct userrecord *xfr;

      xfr = (struct userrecord *)malloc(sizeof(struct userrecord));
	xf = fopen("xferlog.bin", "r");
      while (!feof(xf)) {
      fread(xfr, sizeof(struct userrecord), 1, xf);
      fdes = open(xfr->username, O_CREAT | O_WRONLY | O_TRUNC, 0666);
	
	lseek(fdes, 0, L_SET);

	if (xfr->flags.leech)
	    sprintf(work_buff, "GENERAL 0 0 %lu %lu\n", xfr->maxlogins, xfr->credits);
	else
	    sprintf(work_buff, "GENERAL %lu 0 %lu %lu\n", xfr->ratio, xfr->maxlogins, xfr->credits);
	write(fdes, work_buff, strlen(work_buff));

	sprintf(work_buff, "FLAGS ");
	if (xfr->flags.nuke)
	    strcat(work_buff, "A");
	if (xfr->flags.groupop)
	    strcat(work_buff, "2");
	if (xfr->flags.useredit)
	    strcat(work_buff, "7");
	if (xfr->flags.siteop)
	    strcat(work_buff, "1");
        if (xfr->flags.disabled)
	    strcat(work_buff, "6");
	if (xfr->flags.color)
	    strcat(work_buff, "5");
	strcat(work_buff, "\n");
	write(fdes, work_buff, strlen(work_buff));  

        sprintf(work_buff, "UNFO %s\n", xfr->tagline);
        write(fdes, work_buff, strlen(work_buff));

  
        /*
         * Xfer information fields:
         */
	sprintf(work_buff, "DAYUP %lu %lu %lu\n", xfr->todayupfile, xfr->todayup, xfr->todayupsec);
	write(fdes, work_buff, strlen(work_buff));
	sprintf(work_buff, "DAYDN %lu %lu %lu\n", xfr->todaydownfile, xfr->todaydown, xfr->todaydownsec);
	write(fdes, work_buff, strlen(work_buff)); 

	sprintf(work_buff, "WKUP %lu %lu %lu\n", xfr->weekupfile, xfr->weekup, xfr->weekupsec);
	write(fdes, work_buff, strlen(work_buff));
	sprintf(work_buff, "MONTHUP %lu %lu %lu\n", xfr->weekupfile, xfr->weekup, xfr->weekupsec);
	write(fdes, work_buff, strlen(work_buff));

	sprintf(work_buff, "WKDN %lu %lu %lu\n", xfr->weekdownfile, xfr->weekdown, xfr->weekdownsec);
	write(fdes, work_buff, strlen(work_buff));
	sprintf(work_buff, "MONTHDN %lu %lu %lu\n", xfr->weekdownfile, xfr->weekdown, xfr->weekdownsec);
	write(fdes, work_buff, strlen(work_buff));

        sprintf(work_buff, "ALLUP %lu %lu %lu\n", xfr->allupfile, xfr->allup, xfr->allupsec);
        write(fdes, work_buff, strlen(work_buff));
        sprintf(work_buff, "ALLDN %lu %lu %lu\n", xfr->alldownfile, xfr->alldown, xfr->alldownsec);
        write(fdes, work_buff, strlen(work_buff));

        sprintf(work_buff, "TIME %ld %ld 0 0\n", xfr->logins, xfr->lastlogin);
        write(fdes, work_buff, strlen(work_buff));
	sprintf(work_buff, "NUKE 0 0 0\n");
	write(fdes, work_buff, strlen(work_buff));
	sprintf(work_buff, "SLOTS -1\n");
	write(fdes, work_buff, strlen(work_buff));

	sprintf(work_buff, "GROUP %s\n", xfr->groupname);
	write(fdes, work_buff, strlen(work_buff));
	
	for (ct=0; ct < 10 && strcmp("Username",xfr->ident[ct].ident); ct++) {
      		if (!strcasecmp(xfr->ident[ct].ident, "rFtpd"))
			strcpy(work_buff2, "*");
		else
			strcpy(work_buff2, xfr->ident[ct].ident);
		sprintf(work_buff, "IP %s@%s\n", work_buff2, xfr->ident[ct].ip);
		write(fdes, work_buff, strlen(work_buff));
	}
	close(fdes);
	}    
  	free(xfr);
	fclose(xf);
  exit(0);
}
