/*
  Written by evilution for the sole purpose of converting .ANS files
  to glftpd's color code format

  Can be extended to support almost any .ANS -> .COL format


  USAGE: ansi2gl ANSIFILE.ANS > outputfile

  Must save your ansi files in 74 colums and use the highlight 
  save in thedraw or aciddraw. Ansi do not limit the character width per 
  line, just use none when it asks. Or just figure this all out for yourself
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ESC 27
#define CTRLZ 0x1a

/* Define video modes */

#define ALLOFF 0
#define BOLD 1
#define UNDERSCORE 4
#define BLINK 5
#define REVERSE 7
#define CONCEAL 8

/* Define ansi codes */
#define COLORMODE 1
#define MOVEUP 2
#define MOVEDOWN 3
#define MOVERIGHT 4
#define MOVELEFT 5
#define UNKNOWN 6

/* Lalala, never used but i keep it here anyhow */
struct attrb { 
   int alloff;
   int bold;
   int underscore; /* shouldnt need this one */
   int blink;
   int reverse;
   int conceal;
};

/* global structure for video modes */

struct attrb attrib;

/* Global variables make it easy to store information about the current      */
/* video state. try to do it without them and get a headache and a big waste */
/* of time                                                                   */

int bold = 0;
int blink = 0;
int laststate[15];
int newline = 0;

/* Checks the last charater of the passed string for the code */

int parseansi(char *code, int x) {
   char ansicode[255];
   strcpy(ansicode, code);
   x--;
   switch(ansicode[x]) {
    case 'm' :
      return COLORMODE;
      break;
    case 'A' :
      return MOVEUP;
      break;
    case 'B' :
      return MOVEDOWN;
      break;
    case 'C' :
      return MOVERIGHT;
      break;
    case 'D':
      return MOVELEFT;
      break;
    default :
      return UNKNOWN;
      break;
   }
   return 0;
   
}

/* This is just one big switch statement to print out the color code */

int printcolor(int code) {
   
   if(code == 0) {
      printf("!0");
      bold = 0;
      blink = 0;
      return 0;
   }
   
   if(code == 1) {
      bold = 1;
      printf("!Z");
   }
   
   if(code == 5) {
      blink = 1;
      return 0;
   }
   
   switch(code) {
    case 30 :
      if ( blink == 0 ) {
	 if(bold == 0) {
	    printf("!a");
	    break;
	 } else {
	    printf("!A");
	    break;
	 }
      } else {
	 if(bold == 0) {
	    printf("!k");
	    break;
	 } else {
	    printf("!K");
	    break;
	 }
      }
    case 31 :
      if ( blink == 0 ) {
	 if(bold == 0) {
	    printf("!b");
	    break;
	 } else {
	    printf("!B");
	    break;
	 }
      } else {
	 if(bold == 0) {
	    printf("!l");
	    break;
	 } else {
	    printf("!L");
	    break;
	 }
      }
    case 32 :
      if ( blink == 0 ) {
	 if(bold == 0) {
	    printf("!c");
	    break;
	 } else {
	    printf("!C");
	    break;
	 }
      } else {
	 if(bold == 0) {
	    printf("!m");
	    break;
	 } else {
	    printf("!M");
	    break;
	 }
      }
	 
    case 33 :
      if ( blink == 0 ) {
	 if(bold == 0) {
	    printf("!d");
	    break;
	 } else {
	    printf("!D");
	    break;
	 }
      } else {
	 if(bold == 0) {
	    printf("!n");
	    break;
	 } else {
	    printf("!N");
	    break;
	 }
      }
    case 34 :
      if ( blink == 0 ) {
	 if(bold == 0) {
	    printf("!e");
	    break;
	 } else {
	    printf("!E");
	    break;
	 }
      } else {
	 if(bold == 0) {
	    printf("!o");
	    break;
	 } else {
	    printf("!O");
	    break;
	 }
      }
    case 35 :
      if ( blink == 0 ) {  
	 if(bold == 0) {
	    printf("!f");
	    break;
	 } else {
	    printf("!F");
	    break;
	 }
      } else {
	 if(bold == 0) {
	    printf("!p");
	    break;
	 } else {
	    printf("!P");  
	    break;
	 }
      }
    case 36 :
      if ( blink == 0 ) {
	 if(bold == 0) {
	    printf("!g");
	    break;
	 } else {
	    printf("!G");
	    break;
	 }
      } else {
	 if(bold == 0) {   
	    printf("!r");
	    break;
	 } else {
	    printf("!R");
	    break;
	 }
      }
    case 37 :
      if ( blink == 0 ) {
	 if(bold == 0) {
	    printf("!h");
	    break;
	 } else {
	    printf("!H");
	    break;
	 }
      } else {
	 if(bold == 0) { 
	    printf("!s");  
	    break;
	 } else { 
	    printf("!S");
	    break;
	 }
      }
    case 40 :
      printf("!1");
      break;
    case 41 :
      printf("!2");
      break;
    case 42 :
      printf("!3");
      break;
    case 43 :
      printf("!4");
      break;
    case 44 :
      printf("!5");
      break;
    case 45 :
      printf("!6");
      break;
    case 46 :
      printf("!7");
      break;
    case 47 :
      printf("!8");
      break;
   }
   return 0;
}


/* ANSI.SYS has cursor placement, glftpd doesnt. So all we can do is */
/* substitude spaces for the ESC[#B statement.                       */
/* Works quite well :)                                               */

int moveright( char *code, int x ) {
   char movenum[3];
   int spaces = 0, count = 0;
   x--;
   strncpy(movenum, code, x);
   spaces = atoi(movenum);
   for( x = 0; x < spaces ; x++ ) {
      printf(" ");
   }
   for(count = 0 ; count < 15; count++ ){
      if(laststate[count] == -1) {
	 return 0;
      } 
      printcolor(laststate[count]);
   }
   return 0;
}


/* Parse the ansi color mode code ESC[##;...;##m                      */
/* where there can be (x > 0) number of arguments to the control code */

int parsecolor( char *code, int x ) {
   /* Assume no more than 15 color modes to the set. I have not had any */
   /* problems with this yet                                            */
   int codes[15];
   char temp[3];
   /* A buffer for the ansi code to be stored in */
   char ansicode[255];
   int a = 0, b = 0, len = 0, c;
   newline = 0;
   x--;
   /* Copy the code into a buffer, cutting the last charater that we */
   /* do not need                                                    */
   strncpy(ansicode, code, x);
   /* set the ansi code buffer to -1 so that the return condition can */
   /* catch the end of the color codes                                */
   for( c = 0; c < 15; c++ ) { 
      codes[c] = -1;
   }
   /* Make it a proper string by setting the last byte to \0 */
   ansicode[x] = '\0';
   len = strlen(ansicode);
   /* A nice little loop to parse the color codes out of the */
   /* format ##;...;## to an array of integers               */
   for (x = 0; x <= len; x++ ) {
      /* this nabs the code out of the string */
      if ( ansicode[x] == ';' || ansicode[x] == '\0') {
	 temp[b] = '\0';
	 codes[a] = atoi(temp);
	 a++;
	 b = 0;
      } else {
      	 temp[b] = ansicode[x];
      	 b++;
      }
   }
   /* store the color state to a buffer for use after a newline */
   for(c = 0; c < 15 ; c++ ) {
      laststate[c] = codes[c];
   }
   /* Step through the array of codes and print each out */
   for(c = 0; c < 15 ; c++ ) {
      if( codes[c] == -1) {
	 break;
      }
      printcolor(codes[c]);

   }
   return 0;
}

int main ( int argc, char *argv[] ) {
   FILE *fp;
   char c;
   char ansicode[255];
   int x = 0, count = 0;
   
   if ((fp = fopen(argv[1], "r")) == NULL) 
     return -1;
   
   while (!feof(fp)) {
      c = fgetc(fp);
      /* CTRLZ signals the end of the file */
      if( c == CTRLZ) {
	 return 0;
      }
      /* If there is a \r\n we want to strip the \r and keep the \n */
      /* standard for ansi files to contain the \r\n combo          */
      /* so this asumption is safe                                  */
      if (c == 13) {
	 c = fgetc(fp);
	 printf("%c", c);
	 /* After a new line restore the color state to that of the        */
	 /* last state on the line before. ANSI.SYS assumes color rollover */
	 /* from one line to the next, glftpd doesnt                       */
	 for(count = 0; count < 15 ; count++ ) {          
	    if( laststate[count] == -1) {         
	       break;
	    }  
	    printcolor(laststate[count]);      
	 }
      /* We have an ansi escape sequence, grab it and send it to the */
      /* parser                                                      */
      } else if( c == ESC ) {
	 c = fgetc(fp);
	 for(x = 0; c != 'm' && c != 'A' && c != 'B' && c != 'C' && c != 'D' ; ++x) {
	    c = fgetc(fp);
	    ansicode[x] = c;
	 }
      	 ansicode[x] = '\0';
	 switch(parseansi(ansicode, x)) {
	  case COLORMODE :
	    parsecolor(ansicode, x);
	    break;
	  case MOVEUP :
	    break;
	  case MOVEDOWN :
	    break;
	  case MOVERIGHT :
	    printf("!0");
	    moveright(ansicode, x);
	    break;
	  case MOVELEFT :
	    break;
	  case UNKNOWN :
	    break;
	 }
      } else {
	 /* if the character is not a escape sequence, or a CR/LF or CTRLZ */
	 /* print it out                                                   */
	 printf("%c", c);
      }
   }
   
   fclose(fp);
   return 0;
}
