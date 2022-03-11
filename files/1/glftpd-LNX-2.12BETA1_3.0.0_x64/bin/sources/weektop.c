/***********************************
  Program:   Weekly Top Upload/Downloads
  Version:   .99 beta
  Date:      26th October 1997
  Author:    raveman   iNCDoA 
  Notes:     This is partially based on weekTop by pSi
             Written for use by GreyLine
***********************************/

/**********************************
  About:     What this basically does is parse the
             file xferlog to track uploads and downloads
             In this file, the "o" indicates downloads (outgoing)
             and the "i" indicates uploads (incoming)

             From this file, the size of the uploads/download
             is taken, along with the username.  This is then
             stored in a sorted link list.

             The output of this link list is then written to the
             output file WeekTop
***********************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define	XFERLOG		"/var/log/xferlog"
#define WEEKTOP_OUT      "/tmp/weektop"
#define	LINE_DELIMITER	' '
#define	USERS_LISTED	10



/******* Structures for holding upload and download details *******/
    
struct useruploads {
	char name[32];
	unsigned long uploads;
	struct useruploads *next;
};

struct userdownloads {
	char name[32];
	unsigned long downloads;
	struct userdownloads *next;
};

FILE *file, *file2;
char space[] = "                                                          ";

struct useruploads *headuploads = NULL;
struct userdownloads *headdownloads = NULL;



/******* Check if it is an upload *******/

int UseLineuploads(char *mode)
{
	if(*mode == 'i')
		return(1);

	/* for future use */
	return(0);
}



/******* Check if it is a download *******/

int UseLinedownloads(char *mode)
{
	if(*mode == 'o')
		return(1);

	/* for future use */
	return(0);
}

struct useruploads *FindUseruploads(char *useruploads)
{
	struct useruploads *tempuploads = headuploads, *temp1uploads=0;
	int found = 0;

	if(!tempuploads) {
		/* create the head */
		headuploads = tempuploads = (struct useruploads *)malloc(sizeof(struct useruploads));
		strcpy(&(tempuploads->name[0]), useruploads);
		tempuploads->uploads = 0;
		tempuploads->next = NULL;
	}
	else {
		/* first try to find the name */
		while(tempuploads && !found) {
			if(!strcmp(&(tempuploads->name[0]), useruploads))
				found = 1;
			else {
				temp1uploads = tempuploads;
				tempuploads = tempuploads->next;
			}
		}

		if(!found) {
			/* we have to create a new entry */
			tempuploads = (struct useruploads *)malloc(sizeof(struct useruploads));
			temp1uploads->next = tempuploads;
			strcpy(&(tempuploads->name[0]), useruploads);
			tempuploads->uploads = 0;
			tempuploads->next = NULL;
		}
	}

	return(tempuploads);
}


struct userdownloads *FindUserdownloads(char *userdownloads)
{
	struct userdownloads *tempdownloads = headdownloads, *temp1downloads=0;
	int found = 0;

	if(!tempdownloads) {
		/* create the head */
		headdownloads = tempdownloads = (struct userdownloads *)malloc(sizeof(struct userdownloads));
		strcpy(&(tempdownloads->name[0]), userdownloads);
		tempdownloads->downloads = 0;
		tempdownloads->next = NULL;
	}
	else {
		/* first try to find the name */
		while(tempdownloads && !found) {
			if(!strcmp(&(tempdownloads->name[0]), userdownloads))
				found = 1;
			else {
				temp1downloads = tempdownloads;
				tempdownloads = tempdownloads->next;
			}
		}

		if(!found) {
			/* we have to create a new entry */
			tempdownloads = (struct userdownloads *)malloc(sizeof(struct userdownloads));
			temp1downloads->next = tempdownloads;
			strcpy(&(tempdownloads->name[0]), userdownloads);
			tempdownloads->downloads = 0;
			tempdownloads->next = NULL;
		}
	}

	return(tempdownloads);
}





void WriteLine(char *useruploads, unsigned long uploads, char *userdownloads, unsigned long downloads)
{

	float temp_uploads, temp_downloads;
	char tempuploads[80], tempdownloads[80], line[160];



       /******* formats the user-name for uploads *******/

	if(strlen(useruploads) > 15)
		*(useruploads + 15) = '\0';
	else
	{
		strcpy(&tempuploads[0], &space[0]);
		strncpy(&tempuploads[0], useruploads, strlen(useruploads));
		tempuploads[15] = '\0';
		strcpy(useruploads, &tempuploads[0]);
	}
	strcpy(&line[0], useruploads);



       /******* format the upload size *******/

	temp_uploads = (float)uploads / 1024.0;

	if(temp_uploads >= 1024.0) {
		sprintf(&tempuploads[0], "%4.3f", (float)uploads / (1024.0 * 1024.0));
		strcpy(&tempuploads[8], "      .000 GB   ");
	}
	else {
		sprintf(&tempuploads[0], "%4.3f", (float)uploads / 1024.0);
		strcpy(&tempuploads[8], "      .000 MB   ");
	}

	strncpy(&tempuploads[18 - strlen(&tempuploads[0])], &tempuploads[0], strlen(&tempuploads[0]));
	strcat(&line[0], &tempuploads[10]);



        /******* format user-name for downloads *******/

	if(strlen(userdownloads) > 15)
		*(userdownloads + 15) = '\0';
	else
	{
		strcpy(&tempdownloads[0], &space[0]);
		strncpy(&tempdownloads[0], userdownloads, strlen(userdownloads));
		tempdownloads[15] = '\0';
		strcpy(userdownloads, &tempdownloads[0]);
	}
	strcat(&line[0], userdownloads);



	/******* format the download size *******/

	temp_downloads = (float)downloads / 1024.0;

	if(temp_downloads >= 1024.0) {
		sprintf(&tempdownloads[0], "%4.3f", (float)downloads / (1024.0 * 1024.0));
		strcpy(&tempdownloads[8], "      .000 GB   ");
	}
	else {
		sprintf(&tempdownloads[0], "%4.3f", (float)downloads / 1024.0);
		strcpy(&tempdownloads[8], "      .000 MB   ");
	}

	strncpy(&tempdownloads[18 - strlen(&tempdownloads[0])], &tempdownloads[0], strlen(&tempdownloads[0]));
	strcat(&line[0], &tempdownloads[10]);


	fprintf(file, "%c %s %c\n", LINE_DELIMITER, &line[0], LINE_DELIMITER);
}







void ProcessXferLog(void)
{
	struct useruploads *tempuploads, *temp1uploads, *temp2uploads;
	struct userdownloads *tempdownloads=0, *temp1downloads, *temp2downloads;
	char line[256], day[10], username[32], dummy[256], mode[5];
	int user_listeduploads = 0, dummy_int;
	int user_listeddownloads = 0;
	unsigned long size, max;



	/******* read the xferlog and build a user-database ********/
	do {
		fgets(&line[0], 256, file);
		if(!feof(file)) {
			/* parse line */
			sscanf(&line[0], "%s %s %d %s %d %s %s %ld %s %s %s %s %s %s %s %s %s", &day[0], &dummy[0], &dummy_int, &dummy[0], &dummy_int, &dummy[0], &dummy[0], &size, &dummy[0], &dummy[0], &dummy[0], &mode[0], &dummy[0], &username[0], &dummy[0], &dummy[0], &dummy

[0]);
			if(UseLineuploads(&mode[0])) {
				/* this line has a correct date, use it */
				tempuploads = FindUseruploads(&username[0]);
				tempuploads->uploads += size / 1024;
			}
			if(UseLinedownloads(&mode[0])) {
				/* this line has a correct date, use it */
				tempdownloads = FindUserdownloads(&username[0]);
				tempdownloads->downloads += size / 1024;
			}
		}
	} while(!feof(file));
	fclose(file);



        /******* Open the WeekTop file, write out the the header *******/

	file = fopen(WEEKTOP_OUT, "w");

	fprintf(file, "%s", "                    !GW!geekly !GT!gop !GT!gotals!0 \n");
	fprintf(file, "%s", "                 !g(c) raveman iNCDoA 1997!0\n");
	fprintf(file, "%s", "!b-----------------------------------------------------------!0\n");
	fprintf(file, "%s", "          !BUploads                       Downloads!0\n");
	fprintf(file, "%s", "!b-----------------------------------------------------------!0\n");



        /******* Sort the upload and download link lists *******/

	tempuploads = headuploads;
	while(tempuploads && tempdownloads) {
		max = tempuploads->uploads;
		temp2uploads = tempuploads;
		temp1uploads = headuploads;
		while(temp1uploads) {
			if(temp1uploads->uploads > max) {
				max = temp1uploads->uploads;
				temp2uploads = temp1uploads;
			}
			temp1uploads = temp1uploads->next;
		}

		tempdownloads = headdownloads;
		max = tempdownloads->downloads;
		temp2downloads = tempdownloads;
		temp1downloads = headdownloads;
		while(temp1downloads) {
			if(temp1downloads->downloads > max) {
				max = temp1downloads->downloads;
				temp2downloads = temp1downloads;
			}
			temp1downloads = temp1downloads->next;

		}


  	        WriteLine(&(temp2uploads->name[0]), temp2uploads->uploads, &(temp2downloads->name[0]), temp2downloads->downloads);
		temp2uploads->uploads = 0;
		tempuploads = tempuploads->next;
		temp2downloads->downloads = 0;
		tempdownloads = tempdownloads->next;

		user_listeduploads++;
		if(user_listeduploads == USERS_LISTED)
			tempuploads = NULL;

		user_listeddownloads++;
		if(user_listeddownloads == USERS_LISTED)
			tempdownloads = NULL;
	}

	fclose(file);



	/******* Free memory used by the upload link list *******/
	tempuploads = headuploads;
	while(tempuploads) {
		temp1uploads = tempuploads;
		tempuploads = tempuploads->next;
		free(temp1uploads);
	}



	/******* Free memory used by the download link list *******/
	tempdownloads = headdownloads;
	while(tempdownloads) {
		temp1downloads = tempdownloads;
		tempdownloads = tempdownloads->next;
		free(temp1downloads);
	}


}

int main(void)
{
	if((file = fopen(XFERLOG, "r"))) {
		ProcessXferLog();
		fclose(file);
	}
	return 0;
}
