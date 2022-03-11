#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>

/* 

	Site dupe for glftpd 1.16.5+
	Author: 187
	Email: 187@drinkordie.com
	Date: 990623

*/

void show_help() {
	printf("\n.--------------------------------------------------.\n");
	printf("| Dupechecker for glftpd - version 990623 - by 187 |\n");
	printf("|                                                  |\n");
	printf("| Usage: site dupe arg1 arg2 arg3 ...              |\n");
	printf("`--------------------------------------------------'");
	printf("\n");
	exit(0);
	
}


void print_text (char *row) {
	
	char string[255];
	const char delimiters[] = " ";
	char *running;
	char *token;

	bzero(string, sizeof(string));	
	running = NULL;
	token = NULL;
	
	running = string;
	strcpy(string, row);
	token = strsep (&running, delimiters);    
	printf("  %s",token);
	token = strsep (&running, delimiters);    
	printf("   %s",token);

}

char *strupr( char *string ){
	char *s;

	if ( string ){
		for ( s = string; *s; ++s )
			*s = toupper( *s );
	}
	return string;
}                        

int main (int argc, char **argv) {

	FILE *fp;
	char dupebuff[255];
	char *buf;
	int i;
	int counter = 0;
	int found = 0;
	int total = 0;
	
	if(argc == 1)
		show_help();
		
	
	for(i=1; i<argc; i++){
		strupr(argv[i]);
	}	
		
	if ((fp = fopen("/ftp-data/logs/dupelog", "r")) == NULL) {
		printf("couldn't open file\n");
		return 0;
        }

	printf("\n          Matches found in the online dupe database:\n");
	printf(".-------------------------------------------------------------.\n");
	printf("| DATE:  | RELEASE NAME:                                      |\n");
	printf("`--------+----------------------------------------------------'\n");

        while (!feof(fp)) 
        {

		if (fgets(dupebuff, 127, fp) == NULL && !feof(fp)) 
			break;

		if (feof(fp)) 
	       		break;

		buf = strdup(dupebuff);
		strupr(buf);

		counter = 0;

		for(i=1; i<argc; i++){
			if(strstr(buf,argv[i]) != NULL)
				counter++;
		}	

		free(buf);

		if(counter == argc-1){
			print_text(dupebuff);
			found++;
			
		}
		total++;
  

        }
	fclose(fp);
	if(found == 0)
		printf("\nNo matches found in dupelist!!\n\n");
	else if(found == 1)
		printf("\n%d dupe listed (%d total)\n\n",found,total);
	else
		printf("\n%d dupes listed (%d total)\n\n",found,total);
	return 0;
}
