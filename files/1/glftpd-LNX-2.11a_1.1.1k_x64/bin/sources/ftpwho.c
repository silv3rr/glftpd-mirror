#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/param.h>
#include "glconf.h"

static struct ONLINE *online;
static int  dl = 0;
static int  ul = 0;
static int  num_users = 0;
static int  shmid;
static char ipckey[20];
static struct shmid_ds ipcbuf;

static void quit();
static double calc_time();


void getarg(char *var, char *line, int size) {
  while(line && (*line == ' ' || *line == '\t'))
    line++;
  strncpy(var, line, size);
  while(var && *var != ' ' && *var != '\t' && *var != '\0')
    var++;
  *var = '\0';
}

int readconf(char *configpath) {
  FILE *configfile;
  char line[1024];
  int cnt;

  if ((configfile = fopen(configpath, "r")) == NULL) {
  /*    printf("Cannot open config file: %s", strerror(errno)); */
    return 0;
  }
  while (fgets(line, sizeof(line), configfile)) {
    for (cnt=0; line[cnt]; cnt++) {
      if (line[cnt] == '#' || line[cnt] == '\n' || line[cnt] == '\r')
        line[cnt] = '\0';
    }
    if (strncmp(line, "ipc_key", 7) == 0) {
      getarg(ipckey, line+7, sizeof(ipckey));
    }
  }
  fclose(configfile);
  return 1;
}


static char *
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
				 
static char hmsbuf[50];			
void
pid_hms_format( time_t testtime )
{
  time_t timenow = time( NULL );
  time_t difftime;
  int    hours = 0;
  int    minutes = 0;

  difftime = timenow - testtime;
   
  while ( difftime >= (time_t)3600 )
    {
      difftime -= (time_t)3600;
      hours++;
    }          
   
  while ( difftime >= (time_t)60 )
    {
      difftime -= (time_t)60;
      minutes++;
    }
   
  if ( hours != 0 )
    sprintf( hmsbuf, "%02d:%02d:%02ld", hours, minutes, difftime );
  else
    sprintf( hmsbuf, "   %02d:%02ld", minutes, difftime );
}

static int
checkusers(void)
{
  register int i, j, found = 0;
  char statbuf[ 500 ];
  char pidbuf[ 10 ];
  char filename[20];
			
  for (i=0; i<num_users; i++) 
    {
      if (online[i].procid == 0)
        continue;
      sprintf( pidbuf, "%u", online[i].procid );
      pid_hms_format( online[i].tstart.tv_sec );  
		
      if ((!strncasecmp (online[i].status, "STOR", 4) ||
	  !strncasecmp (online[i].status, "APPE", 4)) &&
          online[i].bytes_xfer != 0) {
          strncpy(filename, online[i].status+5, sizeof(filename));
          j = strlen(filename);
          if (!isprint(filename[j-2]))
            filename[j-2] = '\0';
          else if (!isprint(filename[j-1]))
            filename[j-1] = '\0';
          sprintf( statbuf, "Up: %-17.17s at %.1fK/sec", filename,
                  calc_time(i) );
          ul++;
        }
      /* Downloading */
      else if (!strncasecmp (online[i].status, "RETR", 4) && online[i].bytes_xfer != 0)
        {
          strncpy(filename, online[i].status+5, sizeof(filename));
          j = strlen(filename);
          if (!isprint(filename[j-2]))
            filename[j-2] = '\0';
          else if (!isprint(filename[j-1]))
            filename[j-1] = '\0';

          sprintf( statbuf, "Dn: %-17.17s at %.1fK/sec", filename, 
                  calc_time(i) );
          dl++;
        }
      /* Idling */
      else if (time(NULL) - online[i].tstart.tv_sec > 5)
        {
          sprintf( statbuf, "Idle: %-8.8s", hmsbuf );
        }
      /* Doing something else... */
      else
        {
          sprintf( statbuf, "\"%s\"", online[i].status );
          trim(statbuf);
        }
      pid_hms_format( online[i].login_time); 
      fprintf( stdout, "| %-8.8s | %-5.5s | %-8.8s | %-44.44s |\n", 
              online[i].username, pidbuf, hmsbuf, statbuf);
      found++;
    }
  return (found);
}

void
usage (void)
{
        printf ("Usage: ftpwho [-r /pathto/glftpd.conf]\n");
        exit (0);
}

int
main( int argc, char **argv )
{
  char *confpath = GLCONF;
  int numusers, c;

  /* Parse command line options */
  while((c = getopt(argc, argv, "hr:")) != EOF) {
    switch(c) {
      case 'h':
        usage();
      case 'r':
        confpath = strdup(optarg);
        break;
      default:
        usage();
    }
  }

  argc -= optind;
  argv += optind;

  if (argc > 0)
    usage();

  strcpy(ipckey, "0x0000DEAD");
  if (!readconf(confpath))
    printf("Couldn't read config file. Using defaults. Use option -r/path/config.file to override.\n");

  if ((shmid = shmget( (key_t)strtoul(ipckey, NULL, 16), 0, 0)) == -1) {
      fprintf(stdout, "No Users Currently On Site!\n");
      exit(1);
    }

  if ((online = (struct ONLINE *)shmat( shmid, NULL, SHM_RDONLY))
      == (struct ONLINE *)-1)
    {
      fprintf(stdout, "Error: (SHMAT) Failed!\n");
      exit(1);
    }
							     
  shmctl( shmid, IPC_STAT, &ipcbuf);
  num_users = ipcbuf.shm_segsz / sizeof( struct ONLINE );

  fprintf(stdout,".----------------------------------------------------------------------------.\n");
  fprintf(stdout,"| Username | PID   | OnLine   | Action                                       |\n" );
  fprintf(stdout,"|----------+-------+----------+----------------------------------------------|\n" );
  numusers = checkusers();
  fprintf(stdout,"`----------------------------------------------------------------------------'\n" );   					  
   
  quit( 0 );
  //never reached
  return 0;
}

static double
calc_time(int pid)
{
  struct timeval tstop;
  double delta, rate;

  if (online[pid].bytes_xfer < 1)
    return 0;
  gettimeofday(&tstop, (struct timezone *)0 );
  delta = ((tstop.tv_sec*10.)+(tstop.tv_usec/100000.)) -
         ((online[pid].tstart.tv_sec*10.)+(online[pid].tstart.tv_usec/100000.));
  delta = delta/10.;
  rate = ((online[pid].bytes_xfer / 1024.0) / (delta));
  if (!rate) rate++;
  return (double)(rate);
}

static void
quit(int exit_status)
{
  shmctl( shmid, IPC_STAT, &ipcbuf);
  if (ipcbuf.shm_nattch <= 1) {
    shmctl( shmid, IPC_RMID, 0);
  }
  shmdt(0);
  exit(exit_status);
}

