#include <stdio.h>
#include <dirent.h>
#include <string.h>

int main() {

  FILE *ufile;
  FILE *newfile;
  DIR *dirg;
  struct dirent *dn;
  char oldname[30];
  char readline[255];
  char buff[255];
  char *ptr;
  char genline[255];
  char ratline[30];
  int namelen = 0;

  dirg = opendir(".");
  if (!dirg) {
    printf("Error opening current directory\n");
    return 1;
  }
  while (dn = readdir(dirg)) {
    if (strcmp("uconv", dn->d_name) == 0) continue;
    namelen = strlen(dn->d_name);
    if (strcmp((dn->d_name)+namelen-4, ".old") == 0) continue;
    if (strcmp(dn->d_name, ".") == 0 || strcmp(dn->d_name, "..") == 0 || strcmp(dn->d_name, "backup") == 0) continue;
    snprintf(oldname, sizeof(oldname), "%s.old", dn->d_name);
    if (rename(dn->d_name, oldname)) {
      printf("Unable to rename %s to %d\n", dn->d_name, oldname);
      continue;
    }
    ufile = fopen(oldname, "r");
    if (ufile == NULL) {
      printf("Error opening %s\n", oldname);
      continue;
    }
    newfile = fopen(dn->d_name, "w");
    if (newfile == NULL) {
      printf("Error creating %s\n", dn->d_name);
      continue;
    }
    memset(readline, '\0', sizeof(readline));
    while (fgets(readline, sizeof(readline), ufile)) {
      strcpy(buff, readline);
      ptr = strtok(buff, " ");
      if (ptr == NULL)
        continue;
      if (!strcmp(ptr, "GENERAL")) {
	strcpy(genline, "GENERAL ");
        ptr = strtok(NULL, " "); /* ratio */
	strcpy(ratline, "RATIO ");
	strcat(ratline, ptr);
	strcat(ratline, "\n");
        ptr = strtok(NULL, " ");  /* wkly_allotment */
        strcat(genline, ptr);
	strcat(genline, " ");
        ptr = strtok(NULL, " ");  /* num_logins */
        strcat(genline, ptr);
	strcat(genline, " ");
        ptr = strtok(NULL, " "); /* credits */
        ptr = strtok(NULL, " "); /* max dlspeed */
	if (!ptr) goto goodgen; /* if this is null, this is already 1.17.0 format */
        strcat(genline, ptr);
	strcat(genline, " ");
        ptr = strtok(NULL, " "); /* max ulspeed */
	if (!ptr) {              /* if no ulspeed, this is not 1.16.9 or 1.17.0 format */
	  genline[strlen(genline)-2] = ' ';
	  genline[strlen(genline)-1] = '\0';
	  strcat(genline, "0\n");  /* set ulspeed to 0 */
	} else
          strcat(genline, ptr);
        fputs(genline, newfile);
	fputs(ratline, newfile);
      } else
goodgen:
        fputs(readline, newfile);
      memset(readline, '\0', sizeof(readline));
    }
    fclose(newfile);
    fclose(ufile);
  }
  printf("done\n");
}
  
