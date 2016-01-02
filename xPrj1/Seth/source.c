#include "stdio.h"
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <fcntl.h>

#define PORTNUMBER 8080
#define SER_BUFF_SIZE 1000
#define LOGFILE "/myhttpd/logfile"
#define DIRC "/myhttpd"
#define SIZE 200/* Directory to fetch files */
#define ARGSIZE 1000
#define READYQUEUESIZE 10
#define MAXTHREADS 20
#define DIR_SIZE 500
/*dhruv seth*/

/* and global variables */

	static int port = PORTNUMBER; //default port number
	static char RootDirectory[SIZE] = DIRC; //default server directory
	static int qtime = 60; //default wait time
	static int eThread = 4; //default number of execution threads
	static int Schd = 1; //FCFS = 1 & SJF = 2 (default = 1) & 0 means off
	static int mode = 0; //regular mode = 0 (default ie with threads) & debugging mode = 1 & helpmode =2
	static int logmode =0; // no logging = 0(default) & yes logging = 1 & logging =2 on stdout
	static int sfd; //Global server's File Descriptor
	static char LogFile[SIZE] = LOGFILE; //default log file
	static int rdyq_currentsize = 0;
	struct readyqueueBUFF 
	{
		int markforexecution; //0 for no; 1 for yes -- not being used
		int filesize;
		char cmdType[4];
		char requesttype[10]; //LS or CAT or BAD
		char path[SIZE];
		int clientfd;
		time_t arrivaltime;
		time_t responsetime;
		char clientip[250];
	};
	// information to maintain the circular queue data structure
	int p_s_head = 0;
	int p_s_tail = 0;
	int s_ex_head = 0;
	int s_ex_tail=0;
	//shared buffer
	struct readyqueueBUFF rdybuffer[READYQUEUESIZE],exbuffer[READYQUEUESIZE];
	// mutex lock for buffer
	pthread_mutex_t p_s_mutex;
	pthread_mutex_t s_ex_mutex;  
	//semaphores for specifying empty and full indicators
	sem_t p_s_emptyBuffers;
	sem_t p_s_fullBuffer;
	sem_t s_ex_emptyBuffers;
	sem_t s_ex_fullBuffer;

//function declaration
void readParameters(const char *strPara);
void extractFilename(char *s, char *d);
void initialize_locks();
void acceptconnections(int sfd);
void processrequest(char *req, FILE *fpout);
void errormode();
void serverfunc(int mode);
void insertinto_exBUFF(struct readyqueueBUFF *p,int tail);
void copyfromREADYQueue(struct readyqueueBUFF *p,int head);
void get_ex(struct readyqueueBUFF *p,int head);
void extract_extension(char *t,char *d);
void send_cat(char cmdtype[4],char path[SIZE],int filesize,int cfd,char clientip[100], time_t arrtime);
void send_ls(char cmdtype[4],char path[SIZE],int filesize,int cfd,char clientip[100], time_t arrtime);
void send_404(char cmdtype[4],char path[SIZE],int cfd,char clientip[100], time_t arrtime);
void execute(struct readyqueueBUFF *p);
void *consumerfunc(void *p);
void *schedulerfunc(void *p);
void getSJ(struct readyqueueBUFF *p,int head, int tail);
void *producerfunc(void *p);
void insertinto_buff(struct readyqueueBUFF *p,int tail);
void logger(char *clientip,char *arrt, char *rcvt, char *requeststr, char *requeststatus, int requestsize);
//End declaration

//initialze the locks
void initialize_locks()
{
    pthread_mutex_init(&p_s_mutex,NULL);
    sem_init(&p_s_emptyBuffers,0,READYQUEUESIZE);
    sem_init(&p_s_fullBuffer,0,0);

    pthread_mutex_init(&s_ex_mutex,NULL);
    sem_init(&s_ex_emptyBuffers,0,READYQUEUESIZE);
    sem_init(&s_ex_fullBuffer,0,0);
}
void readParameters(const char *strPara)
{
	char *tmp=NULL;
	FILE *fproot;
		//l :log file
		strcat(RootDirectory,"/"); //to append '/' to the rootdirectory
		tmp = strstr(strPara, "-l");
		if(tmp!=NULL)
		{
			FILE *fp;
			logmode = 1;
			extractFilename(tmp,LogFile);
			fp = fopen(LogFile,"a");
			if(fp==NULL)
			{
			  fprintf (stderr,"\nERROR: LOGFILE cannot be created or appended\n");
			  mode=2;
			  return;
			}
			fclose(fp);
			tmp=NULL;
		}
                //p: port numner
		tmp = strstr(strPara,"-p");
		if(tmp!=NULL)
		{
			char chrport[10];
			extractFilename(tmp,chrport);
			port=atoi(chrport);
			tmp=NULL;
			if(port<=1024)
			{
			  fprintf (stderr,"\nERROR: PORT cannot be less than 1024\nRecommended port#: 8080");
			  mode=2;
			  return;
			}
		}
		//r: root directory
		tmp = strstr(strPara,"-r");
		if(tmp!=NULL)
		{
			FILE *fp;
			extractFilename(tmp,RootDirectory);
			strcat(RootDirectory,"/");
			tmp=NULL;
			fp = fopen(RootDirectory,"r");
			if(fp==NULL)
			{
			  fprintf (stderr,"\nERROR: ROOT DIRECTORY DOES NOT EXISTS\n");
			  mode=2;
			  return;
			}
			fclose(fp);
		}
		//t: time to queue
		tmp = strstr(strPara,"-t");
		if(tmp!=NULL)
		{
			char chrtime[10];
			extractFilename(tmp,chrtime);
			qtime = atoi(chrtime);
			tmp=NULL;
			if(qtime<0)
			{
			  fprintf (stderr,"\nERROR: Queuing time is incorrect\n");
			  mode=2;
			  return;
			}
		}
		//n: e Threads
		tmp = strstr(strPara,"-n");
		if(tmp!=NULL)
		{
			char chrThreads[5];
			extractFilename(tmp,chrThreads);
			eThread = atoi(chrThreads);
			tmp=NULL;
			if(eThread<=0 || eThread>MAXTHREADS)
			{
			  fprintf (stderr,"\nERROR: Number of threads should be 0<threads<%d\n",eThread=MAXTHREADS);
			  mode=2;
			  return;
			}
		}
		//s: Scheduler algo
		tmp = strstr(strPara,"-s");
		if(tmp!=NULL)
		{
			char chrSchd[10];
			int i=0;
			for(i=0;tmp[i]!='\0';i++)
			  tmp[i] = tolower(tmp[i]);
			extractFilename(tmp,chrSchd);
			if(strcmp(chrSchd,"sjf")==0)
			  Schd = 2; //SJF, else the default value
			else if (strcmp(chrSchd,"fcfs")==0)
			  Schd = 1;
			else
			{
			  fprintf (stderr,"\nERROR: Please choose between SJF or FCFS scheduling algorithm\n");
			  mode=2;
			  return;
			}
			tmp = NULL;
		}
		fproot = fopen(RootDirectory,"r");
		if(fproot==NULL)
		{
		  fprintf(stderr,"\nERROR: webserver root directory is not accessible/present: %s\n",RootDirectory);
		  mode=2; return;
		}
		fclose(fproot);
		//h: help
		tmp = strstr(strPara,"-h");
		if(tmp!=NULL)
		{
			mode = 2; tmp =NULL; return; //no use setting parameters here, just exit
		}
		//d: debug mode
		tmp = strstr(strPara, "-d");
		if(tmp!=NULL)
		{
			mode = 1; tmp=NULL; eThread =1; qtime=0; Schd = 1; logmode=2; bzero(LogFile,SIZE); return;
		}
}
void extractFilename(char *t,char *desti)
{
	int i =0;
	t=t+3; //cross -x and a space
	for(i=0;t[i]!=' ';i++) 
		desti[i]=t[i];
	desti[i]='\0';
}
void errormode()
{
 	printf("\nMYHTTPD USAGE:\n");
	fprintf(stderr,"\n-d		:Debug mode. Server will accept one connection as a time.");
	fprintf(stderr,"\n-h		:help");
	fprintf(stderr,"\n-l filepath	:Log file path. Default is no logging.");
	fprintf(stderr,"\n-p port		:port. Default is 8080.");
	fprintf(stderr,"\n-r dir		:set root directory. Default is ~/myhttpd");
	fprintf(stderr,"\n-t time		:Queuing time to t seconds. Default 60 seconds");
	fprintf(stderr,"\n-n threadnum	:Set number of threads waiting in the execution thread pool. \n\t\t Default is 4 execution threads.");
	fprintf(stderr,"\n-s sched	:Set the scheduling policy. It can be either FCFS or SJF. \n\t\t Default will in be FCFS.\n");
}
void logger(char *clientip,char *arrt, char *rcvt, char *requeststr, char *requeststatus, int requestsize)
{
  if(logmode !=0) //no logging
  {
      FILE *fplog;
      if(logmode == 1) //logging to a file
      {
	fplog = fopen(LogFile,"a");
	fprintf(fplog,"\n%s - [%s] [%s] \"%s\" %s %d\n",clientip,arrt,rcvt,requeststr,requeststatus,requestsize);
	fclose(fplog);
      }
      else if (logmode == 2) //logging on stdout
      {
	fprintf(stderr,"\n%s - [%s] [%s] \"%s\" %s %d\n",clientip,arrt,rcvt,requeststr,requeststatus,requestsize);
      }      
  }
}
void serverfunc(int mode)
{
      char chrSchd[10];

      if(Schd==0 || Schd==1)
	strcpy(chrSchd,"FCFS");
      else if (Schd==2)
	strcpy(chrSchd,"SJF");

	if(mode == 0)
	  printf("\nDAEMON MODE (with settings)\n\n");
	else
	  printf("\nDEBUG MODE (with settings)\n\n");
	printf("PORT: %d\nSERVER DEFAULT DIRECTORY: %s\nWAIT BEFORE SCHEDULING TIME: %d\nTHREADS#: %d\nSCHEDULE MODE: %s\nLOGGING MODE: %d\nLOG FILE AT: %s\n\n",
port,
RootDirectory,
qtime,
eThread,
chrSchd,
logmode,
LogFile);


pthread_t producer_id, consumer_id[MAXTHREADS], scheduler_id;
int i=0;
  initialize_locks();
  //Start the producer threads
  if (pthread_create(&producer_id,NULL,producerfunc,NULL) != 0)
    fprintf (stderr, "no producer thread\n");

   //Start scheduler thread with timer
  if(pthread_create(&scheduler_id,NULL,schedulerfunc,NULL)!=0)
    fprintf(stderr,"No scheduler thread\n");

  //Start the consumer thread(s)
  for(i = 0; i < eThread; i++)
    if (pthread_create(&consumer_id[i],NULL,consumerfunc,NULL) != 0)
      fprintf (stderr, "No consumer thread\n");

(void) pthread_join(producer_id,NULL);
(void) pthread_join(scheduler_id,NULL);
for(i=0;i< eThread;i++)
  (void) pthread_join(consumer_id[i],NULL);
}
void insertinto_exBUFF(struct readyqueueBUFF *p,int tail)
{
  //puts("\nEX buffer\n");
  exbuffer[tail].markforexecution = p->markforexecution;
  exbuffer[tail].filesize = p->filesize;
  strcpy(exbuffer[tail].cmdType,p->cmdType);
  strcpy(exbuffer[tail].requesttype,p->requesttype);
  strcpy(exbuffer[tail].path,p->path);
  exbuffer[tail].clientfd = p->clientfd;
  exbuffer[tail].arrivaltime = p->arrivaltime;
  exbuffer[tail].responsetime = p->responsetime;
  strcpy(exbuffer[tail].clientip,p->clientip);
}
void copyfromREADYQueue(struct readyqueueBUFF *p,int head)
{
  p->markforexecution=rdybuffer[head].markforexecution;
  p->filesize=rdybuffer[head].filesize; 
  strcpy(p->cmdType,rdybuffer[head].cmdType);
  strcpy(p->requesttype,rdybuffer[head].requesttype);
  strcpy(p->path,rdybuffer[head].path);
  p->clientfd = rdybuffer[head].clientfd;
  p->arrivaltime = rdybuffer[head].arrivaltime;
  p->responsetime = rdybuffer[head].responsetime;
  strcpy(p->clientip,rdybuffer[head].clientip);
}
void get_ex(struct readyqueueBUFF *p,int head)
{
  p->markforexecution=exbuffer[head].markforexecution;
  p->filesize=exbuffer[head].filesize;
  strcpy(p->cmdType,exbuffer[head].cmdType);
  strcpy(p->requesttype,exbuffer[head].requesttype);
  strcpy(p->path,exbuffer[head].path);
  p->clientfd=exbuffer[head].clientfd;
  p->arrivaltime=exbuffer[head].arrivaltime;
  p->responsetime=exbuffer[head].responsetime;
  strcpy(p->clientip,exbuffer[head].clientip);

  //puts("\nEXEC: Inserted into FINAL Array\n");
}
void extract_extension(char *t,char *d)
{
	int i=0,j=0;
	for(i=0;i<=30;i++)
		if(t[i]=='.') break;
	i++;
	for(j=0;t[i]!='\0';i++,j++)
		d[j]=t[i];
	d[j]='\0';
}
void send_cat(char cmdtype[4],char path[SIZE],int filesize,int cfd,char clientip[100], time_t arrtime)
{
  char timearr[128],timeresponse[128],timelastmodi[128],c,extn[10],content[100],servername[100],strReq[400];
  struct stat statbuff;
  FILE *fp,*cfp;
  int ff;
  time_t now,t_response;

  bzero(strReq,400);
  strcat(strReq,cmdtype);
  strcat(strReq," ");
  strcat(strReq,path);
  strcat(strReq," HTTP/1.0");
 
  if(stat(path,&statbuff)!=0)
     fprintf (stderr, "STAT Error\n");

  gethostname(servername,500);

  cfp = fdopen(cfd,"w");
  if(cfp == NULL)
    fprintf (stderr, "File Error\n");
  
  extract_extension(path,extn);
  
  if(strcmp(extn,"html")==0)
    strcpy(content,"text/html");
  else if (strcmp(extn,"gif")==0)
    strcpy(content,"image/gif");
  else if ((strcmp(extn,"jpeg")==0) || (strcmp(extn,"jpg")==0))
    strcpy(content,"image/jpeg");
 
  ff = open(path,O_RDONLY);
  fp = fdopen(ff,"r");
  
  if(fp!=NULL)
  {
    fprintf(cfp,"HTTP/1.0 200 OK\r\n");
    fprintf(cfp,"Server: %s\r\n",servername);
    now  = time(NULL);
    t_response = now;
    strftime(timeresponse, sizeof(timeresponse), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
    fprintf(cfp,"Date: %s\r\n",timeresponse);
    fprintf(cfp,"Content-Type: %s\r\n",content);
    fprintf(cfp,"Content-Length: %d\r\n",filesize);
    
    strftime(timelastmodi, sizeof(timelastmodi), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&statbuff.st_mtime));
    fprintf(cfp,"Last-Modified: %s\r\n",timelastmodi);
    fprintf(cfp,"Connection: close\r\n");
    fprintf(cfp, "\r\n");

    if(strcmp(cmdtype,"GET")==0)
    {
      int n;
      char data[4096];
      while ((n = fread(data, 1, sizeof(data), fp)) > 0) fwrite(data, 1, n, cfp);
    fclose(fp);
    }
    strftime(timeresponse, sizeof(timeresponse), "%d/%b/%Y:%H:%M:%S %z", gmtime(&t_response));
    strftime(timearr, sizeof(timearr), "%d/%b/%Y:%H:%M:%S %z", gmtime(&arrtime));
    logger(clientip,timearr, timeresponse, strReq, "200", filesize);
  }
  fflush(cfp);
  fclose(cfp);
  close(cfd);
  close(ff);
}
void send_ls(char cmdtype[4],char path[SIZE],int filesize,int cfd,char clientip[100], time_t arrtime)
{
  char timearr[128],timeresponse[128],timelastmodi[128],content[100],servername[100],strReq[400];
  struct stat statbuff;
  struct dirent *entry;
  DIR *dp;
  FILE *cfp;
  time_t now,t_response;

  bzero(strReq,400);
  strcat(strReq,cmdtype);
  strcat(strReq," ");
  strcat(strReq,path);
  strcat(strReq," HTTP/1.0");
  
  stat(path,&statbuff);  
  gethostname(servername,100);
  cfp = fdopen(cfd,"w");
  strcpy(content,"text/html");
  
  dp = opendir(path);  
  if(dp!=NULL)
  {
    fprintf(cfp,"HTTP/1.0 200 OK\r\n");
    fprintf(cfp,"Server: %s\r\n",servername);
    now  = time(NULL);
    t_response = now;
    strftime(timeresponse, sizeof(timeresponse), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
    fprintf(cfp,"Date: %s\r\n",timeresponse);
    fprintf(cfp,"Content-Type: %s\r\n",content);
    fprintf(cfp,"Content-Length: %d\r\n",filesize);
    
    strftime(timelastmodi, sizeof(timelastmodi), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&statbuff.st_mtime));
    fprintf(cfp,"Last-Modified: %s\r\n",timelastmodi);
    fprintf(cfp,"Connection: close\r\n");
    fprintf(cfp, "\r\n");

    fprintf(cfp,"<HTML><BODY><TITLE>DIRECTORY LIST: %s</TITLE><h1>DIRECTORY LIST: %s </h1><table>",path,path);
// 
    if(strcmp(cmdtype,"GET")==0)
    {
      struct f
      {
	char filename[DIR_SIZE];
      }ff[DIR_SIZE];
      char temp[DIR_SIZE];
      int i=0,j=0,len=0,min=0;
      while((entry = readdir(dp)))
      {
	  if(!(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0))
	  {
	    strcpy(ff[i].filename,entry->d_name);
	    i++;
	  }
      }
      len=i;
      for(i=0;i<len;i++) //bubble sort
	for(j=0;j<len-i;j++)
	  if(strcmp(ff[j].filename,ff[j+1].filename)<0)
	  {
	    strcpy(temp,ff[j+1].filename);
	    strcpy(ff[j+1].filename,ff[j].filename);
	    strcpy(ff[j].filename,temp);
	  }
      while(len!=-1)
      {
	  fprintf(cfp,"<tr><td>%s</td></tr>",ff[len].filename);
	  len--;
      }
      fprintf(cfp,"</table></BODY></HTML>");
    }
    closedir(dp);
    fclose(cfp);
  }
   //for logging
  strftime(timeresponse, sizeof(timeresponse), "%d/%b/%Y:%H:%M:%S %z", gmtime(&t_response));
  strftime(timearr, sizeof(timearr), "%d/%b/%Y:%H:%M:%S %z", gmtime(&arrtime));
    logger(clientip,timearr, timeresponse, strReq, "200", filesize);
}
void send_404(char cmdtype[4],char path[SIZE],int cfd,char clientip[100], time_t arrtime)
{
  char timearr[128],timeresponse[128],timelastmodi[128],servername[100],strReq[400];
  FILE *cfp;
  time_t now,t_response;

  bzero(strReq,400);
  strcat(strReq,cmdtype);
  strcat(strReq," ");
  strcat(strReq,path);
  strcat(strReq," HTTP/1.0");
  
  gethostname(servername,100);
  cfp = fdopen(cfd,"w");

  fprintf(cfp,"HTTP/1.0 404 NOT FOUND\r\n");
  fprintf(cfp,"Server: %s\r\n",servername);
  now  = time(NULL);
  t_response = now;

  strftime(timeresponse, sizeof(timeresponse), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
  fprintf(cfp,"Date: %s\r\n",timeresponse);
  fprintf(cfp,"Content-Type: %s\r\n","text/html");
  fprintf(cfp,"Connection: close\r\n");
  
  fprintf(cfp, "\r\n");
  
  fprintf(cfp,"404 ERROR NOT FOUND: %s; webserver directory: %s",path,RootDirectory);

  fflush(cfp);
  fclose(cfp);
  //for logging
  strftime(timeresponse, sizeof(timeresponse), "%d/%b/%Y:%H:%M:%S %z", gmtime(&t_response));
  strftime(timearr, sizeof(timearr), "%d/%b/%Y:%H:%M:%S %z", gmtime(&arrtime));
  logger(clientip,timearr, timeresponse, strReq, "404", 0);
}
void execute(struct readyqueueBUFF *p)
{
  int cfd;
  cfd = p->clientfd;
  
  if(strcmp(p->requesttype,"LS")==0)
    {
      send_ls(p->cmdType,p->path,p->filesize,cfd,p->clientip,p->arrivaltime);
    }
    else if(strcmp(p->requesttype,"CAT")==0)
    {
      send_cat(p->cmdType,p->path,p->filesize,cfd,p->clientip,p->arrivaltime);
    }
    else if(strcmp(p->requesttype,"BAD")==0)
    {
      send_404(p->cmdType,p->path,cfd,p->clientip,p->arrivaltime);
    }  
}
void *consumerfunc(void *p)
{
  struct readyqueueBUFF tmp_element;
  while(1)
  {
      sem_wait(&s_ex_fullBuffer);
      pthread_mutex_lock(&s_ex_mutex);
      
      //puts("\nEXEC thread: CR Section\n");
      
      get_ex(&tmp_element,s_ex_head);
      s_ex_head = (s_ex_head + 1) % READYQUEUESIZE;
      execute(&tmp_element);
      
      //puts("EXEC Done");

      pthread_mutex_unlock(&s_ex_mutex);
      sem_post(&s_ex_emptyBuffers);
  }
}
void *schedulerfunc(void *p)
{
  while(qtime>0)
  {
    qtime--;
    sleep(1);
  }
  if(Schd==1 || Schd==0) //FCFS or no scheduling are equal
  {
    struct readyqueueBUFF tmp_element; 
    while (1)
    { 

      //puts("\nSCHD Thread: Entered\n");

      sem_wait(&p_s_fullBuffer);
      pthread_mutex_lock(&p_s_mutex);
      
      copyfromREADYQueue(&tmp_element,p_s_head);
      p_s_head = ( p_s_head + 1) % READYQUEUESIZE;
      
      //puts("\nSCHD Thread: Level 1\n");
      //starting semaphore for execution
      
      sem_wait(&s_ex_emptyBuffers);
      pthread_mutex_lock(&s_ex_mutex);

      insertinto_exBUFF(&tmp_element,s_ex_tail);
      //rdybuffer[tail] = item ;
      s_ex_tail = (s_ex_tail+1) % READYQUEUESIZE;
      //printf ("producer: inserted %d \n", item); fflush (stdout);
      
      //puts("\nSCHD Thread: Level 2\n");
      
      pthread_mutex_unlock(&s_ex_mutex);
      sem_post(&s_ex_fullBuffer);
      //execution thread semaphore unlocked

      pthread_mutex_unlock(&p_s_mutex);
      sem_post(&p_s_emptyBuffers);
      //puts("\nSCHD Thread: Level 3\n");
    }
  }
  else if (Schd==2) //SJF
  {
    struct readyqueueBUFF tmp_element; 
    while (1)
    {
      //puts("\nSCHD Thread algo2: Entered\n");

      sem_wait(&p_s_fullBuffer);
      pthread_mutex_lock(&p_s_mutex);
      
      getSJ(&tmp_element,p_s_head,p_s_tail);
      p_s_head = ( p_s_head + 1) % READYQUEUESIZE;
      
      //puts("\nSCHD Thread algo2: Level 1\n");
      //starting semaphore for execution
      sem_wait(&s_ex_emptyBuffers);
      pthread_mutex_lock(&s_ex_mutex);
      insertinto_exBUFF(&tmp_element,s_ex_tail);
      
      //rdybuffer[tail] = item ;
      s_ex_tail = (s_ex_tail+1) % READYQUEUESIZE;
      //printf ("producer: inserted %d \n", item); fflush (stdout);
      
      pthread_mutex_unlock(&s_ex_mutex);
      sem_post(&s_ex_fullBuffer);
      //execution thread semaphore unlocked

      pthread_mutex_unlock(&p_s_mutex);
      sem_post(&p_s_emptyBuffers);
      //puts("\nSCHD Thread algo2: Level final\n");
    }
  }
}
void getSJ(struct readyqueueBUFF *p,int head, int tail)
{
struct readyqueueBUFF temp, tmp_arr[READYQUEUESIZE];
int i=0,j=0;
tail--;
if(head==tail);
else if(head<tail)
{
  for(i=head;i<=tail;i++)
    for(j=head;j<tail;j++)
      if(rdybuffer[j].filesize>rdybuffer[j+1].filesize)
	{
	//temp=arr[j+1]; 
	temp.markforexecution = rdybuffer[j+1].markforexecution;
	temp.filesize = rdybuffer[j+1].filesize;
	strcpy(temp.cmdType,rdybuffer[j+1].cmdType);
	strcpy(temp.requesttype,rdybuffer[j+1].requesttype);
	strcpy(temp.path,rdybuffer[j+1].path);
	temp.clientfd = rdybuffer[j+1].clientfd;
	temp.arrivaltime = rdybuffer[j+1].arrivaltime;
	temp.responsetime = rdybuffer[j+1].responsetime;
	strcpy(temp.clientip,rdybuffer[j+1].clientip);
	//arr[j+1]=arr[j];
	rdybuffer[j+1].markforexecution = rdybuffer[j].markforexecution;
	rdybuffer[j+1].filesize = rdybuffer[j].filesize;
	strcpy(rdybuffer[j+1].cmdType,rdybuffer[j].cmdType);
	strcpy(rdybuffer[j+1].requesttype,rdybuffer[j].requesttype);
	strcpy(rdybuffer[j+1].path,rdybuffer[j].path);
	rdybuffer[j+1].clientfd = rdybuffer[j].clientfd;
	rdybuffer[j+1].arrivaltime = rdybuffer[j].arrivaltime;
	rdybuffer[j+1].responsetime = rdybuffer[j].responsetime;
	strcpy(rdybuffer[j+1].clientip,rdybuffer[j+1].clientip);
	//arr[j]=temp;
	rdybuffer[j].markforexecution = temp.markforexecution;
	rdybuffer[j].filesize = temp.filesize;
	strcpy(rdybuffer[j].cmdType,temp.cmdType);
	strcpy(rdybuffer[j].requesttype,temp.requesttype);
	strcpy(rdybuffer[j].path,temp.path);
	rdybuffer[j].clientfd = temp.clientfd;
	rdybuffer[j].arrivaltime = temp.arrivaltime;
	rdybuffer[j].responsetime = temp.responsetime;
	strcpy(rdybuffer[j].clientip,temp.clientip);
	}
}
else if(head>tail)
{
   int temp_arr_size = 0;
   int i=0,j=0;
   temp_arr_size = ((READYQUEUESIZE-1)-head+1) + (tail+1);
   //copy to tmp_arr
   for(i=head;i<READYQUEUESIZE;i++)
   {
      tmp_arr[i].markforexecution = rdybuffer[i].markforexecution;
      tmp_arr[i].filesize = rdybuffer[i].filesize;
      strcpy(tmp_arr[i].cmdType,rdybuffer[i].cmdType);
      strcpy(tmp_arr[i].requesttype,rdybuffer[i].requesttype);
      strcpy(tmp_arr[i].path,rdybuffer[i].path);
      tmp_arr[i].clientfd = rdybuffer[i].clientfd;
      tmp_arr[i].arrivaltime = rdybuffer[i].arrivaltime;
      tmp_arr[i].responsetime = rdybuffer[i].responsetime;
      strcpy(tmp_arr[i].clientip,rdybuffer[i].clientip);
   }
   //again copy the remaining elements to tmp_arr
   for(i=0;i<=tail;i++)
   {
      tmp_arr[i].markforexecution = rdybuffer[i].markforexecution;
      tmp_arr[i].filesize = rdybuffer[i].filesize;
      strcpy(tmp_arr[i].cmdType,rdybuffer[i].cmdType);
      strcpy(tmp_arr[i].requesttype,rdybuffer[i].requesttype);
      strcpy(tmp_arr[i].path,rdybuffer[i].path);
      tmp_arr[i].clientfd = rdybuffer[i].clientfd;
      tmp_arr[i].arrivaltime = rdybuffer[i].arrivaltime;
      tmp_arr[i].responsetime = rdybuffer[i].responsetime;
      strcpy(tmp_arr[i].clientip,rdybuffer[i].clientip);
    }
    //now sort tmp_arr
    for(i=0;i<=temp_arr_size;i++)
      for(j=0;j<temp_arr_size;j++)
	if(tmp_arr[j].filesize>tmp_arr[j+1].filesize)
	{
	  //temp=arr[j+1];
	  temp.markforexecution = tmp_arr[j+1].markforexecution;
	  temp.filesize = tmp_arr[j+1].filesize;
	  strcpy(temp.cmdType,tmp_arr[j+1].cmdType);
	  strcpy(temp.requesttype,tmp_arr[j+1].requesttype);
	  strcpy(temp.path,tmp_arr[j+1].path);
	  temp.clientfd = tmp_arr[j+1].clientfd;
	  temp.arrivaltime = tmp_arr[j+1].arrivaltime;
	  temp.responsetime = tmp_arr[j+1].responsetime;
	  strcpy(temp.clientip,tmp_arr[j+1].clientip);
	  //arr[j+1]=arr[j];
	  tmp_arr[j+1].markforexecution = tmp_arr[j].markforexecution;
	  tmp_arr[j+1].filesize = tmp_arr[j].filesize;
	  strcpy(tmp_arr[j+1].cmdType,tmp_arr[j].cmdType);
	  strcpy(tmp_arr[j+1].requesttype,tmp_arr[j].requesttype);
	  strcpy(tmp_arr[j+1].path,tmp_arr[j].path);
	  tmp_arr[j+1].clientfd = tmp_arr[j].clientfd;
	  tmp_arr[j+1].arrivaltime = tmp_arr[j].arrivaltime;
	  tmp_arr[j+1].responsetime = tmp_arr[j].responsetime;
	  strcpy(tmp_arr[j+1].clientip,tmp_arr[j].clientip);
	  //arr[j]=temp;
	  tmp_arr[j].markforexecution = temp.markforexecution;
	  tmp_arr[j].filesize = temp.filesize;
	  strcpy(tmp_arr[j].cmdType,temp.cmdType);
	  strcpy(tmp_arr[j].requesttype,temp.requesttype);
	  strcpy(tmp_arr[j].path,temp.path);
	  tmp_arr[j].clientfd = temp.clientfd;
	  tmp_arr[j].arrivaltime = temp.arrivaltime;
	  tmp_arr[j].responsetime = temp.responsetime;
	  strcpy(tmp_arr[j].clientip,temp.clientip);
	}
    //now move the elements to rdyqueue
    for(i=head;i<READYQUEUESIZE;i++)
    {
      rdybuffer[i].markforexecution = tmp_arr[i].markforexecution;
      rdybuffer[i].filesize = tmp_arr[i].filesize;
      strcpy(rdybuffer[i].cmdType,tmp_arr[i].cmdType);
      strcpy(rdybuffer[i].requesttype,tmp_arr[i].requesttype);
      strcpy(rdybuffer[i].path,tmp_arr[i].path);
      rdybuffer[i].clientfd = tmp_arr[i].clientfd;
      rdybuffer[i].arrivaltime = tmp_arr[i].arrivaltime;
      rdybuffer[i].responsetime = tmp_arr[i].responsetime;
      strcpy(rdybuffer[i].clientip,tmp_arr[i].clientip);
    }
    j=i; //important
    for(i=0;i<=tail;i++,j++)
    {
      rdybuffer[i].markforexecution = tmp_arr[j].markforexecution;
      rdybuffer[i].filesize = tmp_arr[j].filesize;
      strcpy(rdybuffer[i].cmdType,tmp_arr[j].cmdType);
      strcpy(rdybuffer[i].requesttype,tmp_arr[j].requesttype);
      strcpy(rdybuffer[i].path,tmp_arr[j].path);
      rdybuffer[i].clientfd = tmp_arr[j].clientfd;
      rdybuffer[i].arrivaltime = tmp_arr[j].arrivaltime;
      rdybuffer[i].responsetime = tmp_arr[j].responsetime;
      strcpy(rdybuffer[i].clientip,tmp_arr[j].clientip);
    }
}
p->markforexecution = rdybuffer[head].markforexecution;
p->filesize = rdybuffer[head].filesize;
strcpy(p->cmdType,rdybuffer[head].cmdType);
strcpy(p->requesttype,rdybuffer[head].requesttype);
strcpy(p->path,rdybuffer[head].path);
p->clientfd = rdybuffer[head].clientfd;
p->arrivaltime = rdybuffer[head].arrivaltime;
p->responsetime = rdybuffer[head].responsetime;
strcpy(p->clientip,rdybuffer[head].clientip);

/*for(i=0;i<3;i++)
  printf("\nFilesize:(%d) %d",i,rdybuffer[i].filesize);*/
}
void insertinto_buff(struct readyqueueBUFF *p,int tail)
{
  fflush(stdout);
  rdybuffer[tail].markforexecution = p->markforexecution;
  rdybuffer[tail].filesize = p->filesize;
  strcpy(rdybuffer[tail].cmdType,p->cmdType);
  strcpy(rdybuffer[tail].requesttype,p->requesttype);
  strcpy(rdybuffer[tail].path,p->path);
  rdybuffer[tail].clientfd = p->clientfd;
  rdybuffer[tail].arrivaltime = p->arrivaltime;
  rdybuffer[tail].responsetime = p->responsetime;
  strcpy(rdybuffer[tail].clientip,p->clientip);
}
void *producerfunc(void *p)
{
  int cfd,a=0,b= sizeof(int),i=0;
  char requestblk[SER_BUFF_SIZE];
  char out[SIZE],cmdType[4],filerequest[SIZE],httpversion[100],hostname[250], *clientipstr=NULL;
  mode_t modes;
  FILE *fpin,*fp;
  unsigned int len;
  time_t now;
  struct readyqueueBUFF tmp_element;
  struct stat statbuf;
  struct sockaddr_in ser,c;
  struct hostent *hp;

	gethostname(hostname,250);
	hp = gethostbyname(hostname);
	
	sfd=socket(AF_INET,SOCK_STREAM,0);
	ser.sin_family=AF_INET;
	ser.sin_addr.s_addr=htonl(INADDR_ANY);
	ser.sin_port=htons(port);
	len=sizeof(ser);
	if((bind(sfd,(struct sockaddr *)&ser,len))==-1)
	{
	  fprintf (stderr, "SOCKET BIND ERROR\n");
	  exit(1);
	}
	if(listen(sfd,1)==-1) //started listening
	{
	  fprintf (stderr, "SOCKET LISTEN ERROR\n");
	  exit(1);
	}
  while(1)
  {
	  struct sockaddr_in c;
	  FILE *fpout;
	  len = sizeof(c);
  
      strcpy(tmp_element.cmdType,"");
      tmp_element.filesize=0;
      strcpy(tmp_element.path,"");
      strcpy(tmp_element.requesttype,"");
      tmp_element.arrivaltime = 0;
      
      //printf("hello %d",sfd);
      if((cfd=accept(sfd,(struct sockaddr *)&c,&len))!=-1)
      
      fpin = fdopen(cfd,"r");
      //fpout = fdopen(cfd,"w");
      //fprintf(fpout,"gggg\r\n");
      //fflush(fpout);
      fgets(requestblk,SER_BUFF_SIZE,fpin);
      //fclose(fpin);
      
      //puts(requestblk);      
      
      sscanf(requestblk,"%s %s %s",&cmdType,&filerequest,&httpversion);
      //printf("\n%s %s %s\n",cmdType,filerequest,httpversion);
      //memset for structure
      //strcpy(filerequest,"/root/index.html");
      tmp_element.arrivaltime = time(&now);
      strcpy(tmp_element.cmdType,cmdType);

      clientipstr=inet_ntoa(c.sin_addr);
      strcpy(tmp_element.clientip,clientipstr);

      tmp_element.clientfd = cfd;
      if(filerequest[0] == '.')
      {
	strcpy(tmp_element.requesttype,"BAD");
	tmp_element.filesize=0;
	strcat(tmp_element.path,filerequest);
      }
      else
      {
	int i;
	char *s = (char*)getenv("HOME"); //fetch
	if(filerequest[0] == '~')
	{
	  strcat(tmp_element.path,s);
	  strcat(tmp_element.path,RootDirectory);
	  for(i=0;i<strlen(filerequest);i++)
	    filerequest[i]=filerequest[i+2];
	  filerequest[i]='\0';
	}
	else if (filerequest[1] == '~')
	{
	  strcat(tmp_element.path,s);
	  strcat(tmp_element.path,RootDirectory);
	  for(i=0;i<strlen(filerequest);i++)
	    filerequest[i]=filerequest[i+3];
	  filerequest[i]='\0';
	}
	else
	  strcat(tmp_element.path,RootDirectory);
	strcat(tmp_element.path,filerequest);

	if(stat(tmp_element.path,&statbuf)!=0)
	{
	  strcpy(tmp_element.requesttype,"BAD");
	  tmp_element.filesize=0;
	}
	else
	{
	  modes=statbuf.st_mode;
	  if(S_ISDIR(modes))
	  {
	    char tmpFile[SIZE]; FILE *fp;
	    strcpy(tmpFile,tmp_element.path);
	    strcat(tmpFile,"/index.html");
	    fp=fopen(tmpFile,"r");
	    if(fp!=NULL) //file found
	    {
	      strcpy(tmp_element.path,tmpFile);
	      strcpy(tmp_element.requesttype,"CAT");
	    }
	    else
	    {
	      strcpy(tmp_element.requesttype,"LS");
	    }
	    stat(tmp_element.path,&statbuf);
	    tmp_element.filesize = statbuf.st_size;
	  }
	  else
	  {
	    strcpy(tmp_element.requesttype,"CAT");
	    tmp_element.filesize = statbuf.st_size;
	  }
	}
      }
      
      //puts("\nPROD: PARSED ");
      //printf("%d %s %s %s\n",tmp_element.filesize,tmp_element.requesttype,tmp_element.path,tmp_element.cmdType);
      
      //semaphore for readyqueue
      sem_wait(&p_s_emptyBuffers);
      pthread_mutex_lock(&p_s_mutex);

      insertinto_buff(&tmp_element,p_s_tail);
      
      //puts("\nPROD: Level 1\n");
      //rdybuffer[tail] = item ;
      
      p_s_tail = (p_s_tail+1) % READYQUEUESIZE;
      //printf ("producer: inserted %d \n", item); fflush (stdout);

      pthread_mutex_unlock(&p_s_mutex);
      sem_post(&p_s_fullBuffer);
      //semaphore unlocked

  }  
}
int main(int argc, char *argv[])
{
	char args[ARGSIZE];
	int i=0;
	pid_t pid;
	for(i=0;i<argc;i++)
	{
	  strcat(args,argv[i]);
	  strcat(args," ");
	}
	readParameters(args);
	switch (mode)
	{
	case 2:	errormode(); break;
	case 1:	serverfunc(mode); break; //single mode
	case 0:	pid = fork(); if(pid==0) {serverfunc(mode); break;} //daemon mode
	}
return 0;
}