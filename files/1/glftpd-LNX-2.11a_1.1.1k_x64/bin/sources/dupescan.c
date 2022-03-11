/*
  Dupescan.C - Scans the dupefile for an entry
  02-24-98
  evilution @ efnet
*/

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

char * read_conf_datapath ( char *rootpath ) {
  FILE *fp;
  char datapath[255];
  char temp[255];
  if((fp = fopen(GLCONF, "r")) == NULL)
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

int main (int argc, char *argv[]) {
  FILE *fp;
  char dupename[255], dupefile[255], Temp[255];
  struct dupefile buffer;
  if (argc == 1){
    printf("USAGE: %s <filename>\n", argv[0]);
    return 0;
  }
  
  read_conf_datapath(Temp);
  sprintf(dupefile, "%s/logs/dupefile", Temp);
  
  strncpy(dupename, argv[1], sizeof(dupename)-1);
  dupename[sizeof(dupename)-1] = '\0';
  if((fp = fopen(dupefile, "r")) == NULL)
    return 0;

  while (!feof(fp)) {
    if (fread(&buffer, sizeof(struct dupefile), 1, fp) < 1)
      break;
    if (strcmp(buffer.filename, dupename) == 0) {
     printf("Found Dupe!");
     printf("\nName: %s\nUploader: %s\n", buffer.filename,buffer.uploader);
    }
  }
printf("Done\n");
fclose(fp);
return 0;
}
