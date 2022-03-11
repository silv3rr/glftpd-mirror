/*

  ON - THE - FLY  SFV CHECKING for glFTPd, build 00.03, 30. November 1999

  Must be called with the following parameters:
  
  /bin/flysfv filename.rxx localcrc file1.sfv file2.sfv ...
  
  Usually done within /bin/zipscript using
  
  cd $2
  /bin/turbosfv $1 $3 *.[sS][fF][vV]
  
  Returns errorlevel 0 if file was OK, 1 if file was bad or not listed or
  no SFV file was found.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main (int numparams, char *params[])
{

  char filename[150];
  char localcrc[20], localcrc2[20];
  int  cnt;
  char sfvname[150];
  FILE *sfvfile;
  char sfvline[250];
  char outtext[250];


/*printf("[ SFV on-the-fly CRC checking for glFTPd build 00.03 ]\n");*/

  if (numparams < 4)
  {
  	printf("Usage: %s <filename.rxx> <localcrc> <sfv-file1> [sfv-file2...]\n", params[0]);
  	return 0;
  }
	
	
  strncpy (filename, params[1], 150);
  for (cnt=0; cnt < (int)strlen(filename); cnt++) filename[cnt] = tolower(filename[cnt]);

  strncpy (localcrc, params[2], 20);
  strncpy (localcrc2, params[2], 20);
  for (cnt=0; cnt < (int)strlen(localcrc); cnt++) localcrc[cnt] = tolower(localcrc[cnt]);  

  printf ("[ Filename: %-29s Local CRC: 0x%8s ]\n", filename, localcrc2);


  cnt = 3;	
  while (cnt < numparams)
  {
  	strncpy (sfvname, params[cnt], 150);
  	if (sfvname[0] != '*')  /* shell would pass *.[sS][fF][vV] if no sfv file available */
  	{
/*  	  printf("Processing: %s\n", sfvname);*/
  	
  	  sfvfile = fopen (sfvname, "rt");
  	  if (sfvfile == NULL)
  	  {
  	  	printf("Fatal error opening %s, aborting...\n", sfvname);
  	  	return 0;
  	  }
  		
  	  while (fgets (sfvline, 250, sfvfile) != NULL)
  	  {
  	  
		for (cnt=0; cnt < (int)strlen(sfvline); cnt++) sfvline[cnt] = tolower(sfvline[cnt]);  

                if (sfvline[0] != ';')    /* ignore comments */
		if (strstr(sfvline, filename) != NULL)
		{
		  if (strstr(sfvline, localcrc) != NULL)
		  {
		     sprintf (outtext, "(verified using %s...)", sfvname);
                     printf("[ FiLE iS OK! %49s ]\n", outtext);		  	
                     printf("[=-------------------------------------------------------------=]\n");                     
                     return 0;
		  }
		  else
		  {
		     sprintf (outtext, "(no match against CRC32 in %s...)", sfvname);
                     printf("[ FiLE iS BAD! %48s ]\n", outtext);		  	
                     printf("[=-------------------------------------------------------------=]\n");
                     return 1;
		  }
		}
  	  	
  	  }
  	  fclose (sfvfile);
  		
  	}
  	cnt++;
  }
	
	
printf("[  This file is NOT LiSTED in any SFV file, assuming it is BAD  ]\n");	
printf("[=-------------------------------------------------------------=]\n");	
	
return 1;
	
	
}

