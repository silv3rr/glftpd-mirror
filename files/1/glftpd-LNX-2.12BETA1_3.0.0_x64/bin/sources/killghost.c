/* Killghost v1.2 by Usurper (based on HoE's killuser)

   Try ./killghost -h for a list of options.
   This program has to be run as root.

   * v1.3 October 2001 * - Modified and enhanced the change below to work with all users, not just those who are idle too long and don't have the idle flag

   * v1.2 October 2001 * - Added code to kill another type of ghosts that appear on some systems - where the process is dead, but the shared memory segment isn't cleared

   * v1.1 October 2001 * - fixed a bug that was killing all uploads or downloads in progress if the transfer took more than idle_limit seconds

   Purpose: to kill idle users who don't issue any commands but are not automatically disconnected by glftpd. This is a rare probllem of unknown origin, most people don't have to worry about it. If it happens to you, execute this program from root's crontab every 5 minutes or so, here is an example syntax:

   0-59/5 * * * * /glftpd/bin/killghost >> /glftpd/ftp-data/logs/killghost.log

*/

int idle_limit=7200;
char logging=0;

#include <stdio.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>
#include "glconf.h"

struct ONLINE *online;
int  num_users = 0;
int  shmid;
char ipckey[20];
char rootpath[50];
char datapath[50];

struct shmid_ds ipcbuf;

void clearshmem();

int protected(char *name) {
  char userfilepath[256];
  char line[128];
  FILE *userfile;

  snprintf(userfilepath, sizeof(userfilepath), "%s%s/users/%s",
   rootpath, datapath, name);
  if ((userfile = fopen(userfilepath, "r")) == NULL)
    return 0;

  while (fgets(line, sizeof(line), userfile)) {
    if (strncmp(line, "FLAGS ", 6) == 0) {
      if (strchr(line, 'I') || strchr(line, '4')) {
        fclose(userfile);
        return 1; /* don't kill users with exempt or idler flags */
      }
    }
  }
  fclose(userfile);
  return 0;
}

void checkusers(void) {
  int cnt, timeout;
  time_t timenow = time(NULL);

  for (cnt=0; cnt<num_users; cnt++) {
      if (online[cnt].procid == 0)
        continue;
      if (kill(online[cnt].procid, 0) < 0) /* process does not exist */ {
	  printf("Clearing an empty (ghost) shared memory segment for user %s.\n",
	    online[cnt].username);
	  memset(&online[cnt], 0, sizeof(struct ONLINE));
	continue;
      }
      if (protected(online[cnt].username) == 1)
        continue;
      if ((strncasecmp(online[cnt].status, "STOR", 4) == 0 ||
           strncasecmp(online[cnt].status, "APPE", 4) == 0 ||
           strncasecmp(online[cnt].status, "RETR", 4) == 0) &&
           online[cnt].bytes_xfer != 0)
        continue;  /* we don't want to kill transfers in progress */
      timeout = timenow - online[cnt].tstart.tv_sec;
      if (timeout > idle_limit) {
        printf("Killing user %s (idle %d seconds)\n", online[cnt].username, timeout);
        if (logging)
          syslog(LOG_INFO, "User %s timed out after %d seconds at %.24s",
           online[cnt].username, timeout, ctime(&timenow));
	if (kill(online[cnt].procid, SIGTERM) < 0) /* failed */ {
	  if (logging)
	    syslog(LOG_ERR, "Kill failed: errno is %d", errno);
	  printf("Failed to kill pid %d: error %d.",
		online[cnt].procid, errno);
	  printf(" Clearing the shared memory segment..\n");
	  memset(&online[cnt], 0, sizeof(struct ONLINE));
	} /* else glftpd will clear its own shmem segment */
      }
  }
}

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
    if (strncmp(line, "rootpath ", 9) == 0)
      getarg(rootpath, line+9, sizeof(rootpath));
    else if (strncmp(line, "datapath ", 9) == 0)
      getarg(datapath, line+9, sizeof(datapath));
    else if (strncmp(line, "ipc_key ", 8) == 0)
      getarg(ipckey, line+8, sizeof(ipckey));
  }
  fclose(configfile);
  return 1;
}

int main( int argc, char **argv ) {
  uid_t myuid = geteuid();
  char *confpath = NULL;
  int cnt;

  /*make sure we are running as root */
  if (myuid != 0 ) {
   puts("ERROR: Must be run as root");
   return(1);
  } 

  if (argc > 1) { /* decipher flags */
    for (cnt=1; argv[cnt]; cnt++)
      if (strncmp(argv[cnt], "-r", 2) == 0) {
        if (*(argv[cnt]+2))
          confpath = argv[cnt]+2;
        else if (argv[cnt+1])
          confpath = argv[++cnt];
      } else
      if (strcmp(argv[cnt], "-l") == 0) {
        logging=1;
      } else
      if (strcmp(argv[cnt], "-h") == 0) {
        printf("\nkillghost [-r path] [-l] [-h]\n Options:\n"
               " -r /path/file : alternative path to config file\n"
               " -l            : logging on (logs to syslog as INFO)\n"
               " -h            : this screen\n");
        exit(0);
      }
  }
  strcpy(rootpath, "/glftpd");
  strcpy(datapath, "/ftp-data");
  strcpy(ipckey, "0x0000DEAD");
  if (!confpath)
    confpath = GLCONF;

  if (!readconf(confpath))
    printf("Couldn't read config file. Using defaults. Use option -r/path/config.file to override.\n");

  if ((shmid = shmget( (key_t)strtoul(ipckey, NULL, 16), 0, 0)) == -1) {
      printf("Error: shmget failed for key %s: no one's online or key is incorrect!\n", ipckey);
      exit(1);
  }

  if ((online = (struct ONLINE *)shmat( shmid, NULL, 0))
      == (struct ONLINE *)-1) {
      printf("Error: shmat failed!\n");
      exit(1);
  }
  shmctl( shmid, IPC_STAT, &ipcbuf);
  num_users = ipcbuf.shm_segsz / sizeof( struct ONLINE );

  checkusers();
  clearshmem();
  return(0);
}

void clearshmem() {
  shmctl( shmid, IPC_STAT, &ipcbuf);
  if (ipcbuf.shm_nattch <= 1) {
    shmctl( shmid, IPC_RMID, 0);
  }
  shmdt(0);
}

