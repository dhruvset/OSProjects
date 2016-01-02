#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#define MAXFRAMES 100
#define defaultPOLICY "FIFO"
#define defaultFilepath "STDIN"
#define MAX 200
#define ARGSIZE 1000
#define ARRMAX 2000

static int frames = MAXFRAMES;
static char policy[10] = defaultPOLICY;
static char filepath[MAX]= defaultFilepath;
static int policymode=2; //2 = FIFO
static int filemode = 0; //0 for stdin mode and 1 for file mode
static int inputarr[ARRMAX];
static int inputlen=0;
static int opt[MAXFRAMES][2];
static int opt_hit=0;
static int lru[MAXFRAMES][2];
static int lru_hit=0;
static int fifo[MAXFRAMES];
static int fifo_hit=0;
static int fifo_head=0;
static int sc[MAXFRAMES][2];
static int sc_hit=0;
static int lfu[MAXFRAMES][3];
static int lfu_hit=0;
static int lfu_copy[ARRMAX][3];
static int lfu_copy_len=0;
static int lfu_counter=100;

void extractFilename(char *t,char *desti);
void extractFilenametoupper(char *t,char *desti);
void readParameters(const char *strPara);
void extractInput(char *t);
void help_error();

int opt_findpage(int element)
{
  int i=0;
  for(i=0;i<frames;i++)
  {
      if(opt[i][0]==element)
	return 0;
  }
  return 1; //not found
}
int opt_findemptyframe()
{
  int i=0;
  for(i=0;i<frames;i++)
  {
    if(opt[i][0]==-1)
      return i;
  }
  return -1;
}
void opt_insert(int element, int position)
{
  opt_hit++;
  opt[position][0] = element;
}
int opt_findfutureof(int element, int startingfrom)
{
  int i=0,future=0;
  for(i=startingfrom;i<inputlen;i++)
  {
      if(inputarr[i]!=element)
	future++;
      else
	break;
  }
  return future;
}
int opt_locateposition(int from)
{
  int i=0,future=0,least=-1,pos=-1;
  for(i=0;i<frames;i++)
  {
    future = opt_findfutureof(opt[i][0],from);
    opt[i][1]=future;
  }
  least = opt[0][1];pos=0;
  for(i=0;i<frames;i++)
  {
    if(opt[i][1]>least)
    {
      least = opt[i][1];
      pos=i;
    }
  }
return pos;
}
void runopt()
{
  int i=0,position=-1;
  for(i=0;i<frames;i++)
  {  
      opt[i][0]=-1;
      opt[i][1]=0;
  }
  for(i=0;i<inputlen;i++)
  {  
      if(opt_findpage(inputarr[i])) //if not found
      {
	  if((position = opt_findemptyframe())!=-1) //if found
	  {
	      opt_insert(inputarr[i],position);
	  }
	  else
	  {
	      position=opt_locateposition(i);
	      opt_insert(inputarr[i],position);
	  }
      }
  }
}
int lru_findpage(int element)
{
  int i=0;
  for(i=0;i<frames;i++)
  {
      if(lru[i][0]==element)
	return 0;
  }
  return 1; //not found
}
int lru_findemptyframe()
{
  int i=0;
  for(i=0;i<frames;i++)
  {
    if(lru[i][0]==-1)
      return i;
  }
  return -1;
}
void lru_insert(int element, int position,int refreshvalue)
{
  lru_hit++;
  lru[position][0] = element;
  lru[position][1]=refreshvalue;
}
int lru_locationposition()
{
  int i=0,least=-1,pos=0;
  least = lru[0][1];
  for(i=0;i<frames;i++)
  {
    if(lru[i][1]<least)
    {
      least = lru[i][1];
      pos=i;
    }
  }
  return pos;
}
void lru_updateelementat(int element,int at)
{
  int i=0;
  for(i=0;i<frames;i++)
  {
    if(lru[i][0]==element)
    {
	lru[i][1] = at;
	break;
    }
  }
}
void runlru()
{
  int i=0,position=-1;
  for(i=0;i<frames;i++)
  {  
      lru[i][0]=-1;
      lru[i][1]=-1;
  }
  for(i=0;i<inputlen;i++)
  {
    if(lru_findpage(inputarr[i]))
    {
      if((position = lru_findemptyframe())!=-1)
      {
	lru_insert(inputarr[i],position,i); //with refresh value
      }
      else
      {
	position = lru_locationposition();
	lru_insert(inputarr[i],position,i); //with refresh value
      }
    }
    else
    {
      lru_updateelementat(i,inputarr[i]);
    }
  }
}

/*
//LFU INPLEMENTATION


int lfu_findpage(int element)
{
  int i=0;
  for(i=0;i<frames;i++)
  {
      if(lfu[i][0]==element)
	return 0;
  }
  return 1; //not found
}
int lfu_findemptyframe()
{
  int i=0;
  for(i=0;i<frames;i++)
  {
    if(lfu[i][0]==-1)
      return i;
  }
  return -1;
}
void lfu_insert(int element, int position)
{
  lfu_hit++;
  lfu[position][0] = element;
  lfu[position][1] = 1;
}
int lfu_locationposition()
{
  int i=0,least=-1,pos=0;
  least = lfu[0][1];
  for(i=0;i<frames;i++)
  {
    if(lfu[i][1]<least)
    {
      least = lfu[i][1];
      pos=i;
    }
  }
  return pos;
}
void lfu_updateelementat(int element)
{
  int i=0;
  for(i=0;i<frames;i++)
  {
    if(lfu[i][0]==element)
    {
	lfu[i][1]++;
	break;
    }
  }
}
void runlfu()
{
  int i=0,position=-1;
  for(i=0;i<frames;i++)
  {  
      lfu[i][0]=-1;
      lfu[i][1]=-1;
  }
  for(i=0;i<inputlen;i++)
  {
    //update_lrucopy(inputarr[i]);
    if(lfu_findpage(inputarr[i]))
    {
      if((position = lfu_findemptyframe())!=-1)
      {
	lfu_insert(inputarr[i],position);
      }
      else
      {
	position = lfu_locationposition();
	lfu_insert(inputarr[i],position);
      }
    }
    else
    {
      lfu_updateelementat(i);
    }
  }
}*/
//LFU v2
int lfu_findpage(int element)
{
  int i=0;
  for(i=0;i<frames;i++)
  {
      if(lfu[i][0]==element)
	return 0;
  }
  return 1; //not found
}
int lfu_findemptyframe()
{
  int i=0;
  for(i=0;i<frames;i++)
  {
    if(lfu[i][0]==-1)
      return i;
  }
  return -1;
}
void lfu_insert(int element, int position)
{
  lfu_hit++;
  lfu[position][0] = element;
  lfu[position][1] = getlfu_frequency(element);
  lfu[position][2] = getlfu_counter(element);
}
int lfu_locationposition()
{
  int i=0,least_fr=-1,least_counter=-1,pos=0;
  least_fr = lfu[0][1];
  for(i=0;i<frames;i++)
  {
    if(lfu[i][1]<least_fr)
    {
      least_fr = lfu[i][1];
    }
  }
  least_counter = lfu[0][2];
  for(i=0;i<frames;i++)
  {
    if(lfu[i][2]<least_counter && lfu[i][1]==least_fr)
    {
      least_counter = lfu[i][2];
      pos=i;
    }
  }
  return pos;
}
void updatelfu_counter(int element, int counter)
{
   int i=0;
   for(i=0;i<frames;i++)
   {
      if(lfu[i][0]==element)
      {
         lfu[i][2]=counter;
         break;
      }
   }
}
void updatelfu_fr(int element,int fr)
{
  int i=0;
  for(i=0;i<frames;i++)
  {
    if(lfu[i][0]==element)
    {
      lfu[i][1]=fr;
      break;
    }
  }
}
void update_lfucopy(int element)
{
  int i=0,new=1;
  for(i=0;i<lfu_copy_len;i++)
  {
      if(lfu_copy[i][0]==element)
      {
	new=0;
	lfu_copy[i][1] = lfu_copy[i][1]+1;
        lfu_copy[i][2] = lfu_counter + 1;
        updatelfu_counter(element,lfu_copy[i][2]);
        updatelfu_fr(element,lfu_copy[i][1]);
        break;
      }
  }
  if(new)
  {
    lfu_copy_len++;
    lfu_copy[i][0]=element;
    lfu_copy[i][1]=1;
    lfu_copy[i][2]=lfu_counter++;
  }
}
int getlfu_counter(int element)
{
  int i=0;
  for(i=0;i<lfu_copy_len;i++)
  {
    if(lfu_copy[i][0]==element)
      return lfu_copy[i][2];
  }
}
int getlfu_frequency(int element)
{
  int i=0;
  for(i=0;i<lfu_copy_len;i++)
  {
    if(lfu_copy[i][0]==element)
      return lfu_copy[i][1];
  }
}
void runlfu()
{
  int i=0,position=-1;
  for(i=0;i<frames;i++)
  {  
      lfu[i][0]=-1;
      lfu[i][1]=-1;
      lfu[i][2]=-1;
  }
  for(i=0;i<inputlen;i++)
  {
    update_lfucopy(inputarr[i]);
    if(lfu_findpage(inputarr[i]))
    {
      if((position = lfu_findemptyframe())!=-1)
      {
	lfu_insert(inputarr[i],position);
      }
      else
      {
	position = lfu_locationposition();
	lfu_insert(inputarr[i],position);
      }
    }
  }
}
int fifo_findpage(int element)
{
  int i=0;
  for(i=0;i<frames;i++)
  {
      if(fifo[i]==element)
	return 0;
  }
  return 1; //not found
}
int fifo_findemptyframe()
{
  int i=0;
  for(i=0;i<frames;i++)
  {
    if(fifo[i]==-1)
      return i;
  }
  return -1;
}
int fifo_locationposition(int element)
{
  int i=0,insertyorn=0;
  for(i=0;i<frames;i++)
  {
    if(fifo[i]==element)
      insertyorn=-1;
  }
  if(insertyorn!=-1)
  {
      fifo_head = (fifo_head+1)%frames;
  }
  return fifo_head;
}
void fifo_insert(int element)
{
  fifo_hit++;
  fifo[fifo_head] = element;
}
void runfifo()
{
  int i=0,position=-1;
  for(i=0;i<frames;i++)
  {  
      fifo[i]=-1;
  }
  for(i=0;i<inputlen;i++)
  {
    if(fifo_findpage(inputarr[i]))
    {
      if((position = fifo_findemptyframe())!=-1)
      {
	fifo_head=position;
	fifo_insert(inputarr[i]);
      }
      else
      {
	fifo_head=fifo_locationposition(inputarr[i]);
	fifo_insert(inputarr[i]);
      }
    }
  }
}
int sc_findpage(int element)
{
  int i=0;
  for(i=0;i<frames;i++)
  {
      if(sc[i][0]==element)
      {
	sc[i][1]=1;
	return i;
      }
  }
  return -1; //not found
}
int sc_findemptyframe()
{
  int i=0;
  for(i=0;i<frames;i++)
  {
    if(sc[i][0]==-1)
      return i;
  }
  return -1;
}
void sc_insert(int element,int at)
{
  sc_hit++;
  sc[at][0] = element;
  sc[at][1] = 1; //redundant
}
int sc_locationposition(int element)
{
  int i=0,insertyorn=0,pos=0;
  for(i=0;i<frames;i++)
  {
    if(sc[i][1]!=0)
      sc[i][1]=0;
  }
  for(i=0;i<frames;i++)
  {
    if(sc[i][1]==0)
    {
      sc[i][0]=element;
      sc[i][1]=1;
      pos=i;
      break;
    }
  }
  return pos;
}
void runsc()
{
  int i=0,position=-1;
  int j=0,pointer=0;
  for(i=0;i<frames;i++)
  {  
      sc[i][0]=-1;
      sc[i][1]=0;
  }
  for(i=0;i<inputlen;i++)
  {
    if((position = sc_findpage(inputarr[i]))==-1)
    {
      do
	{
	    pointer=(pointer+1)%frames;
	}while (!(sc[pointer][1]==1?sc[pointer][1]=0:1));
	sc[pointer][0]=inputarr[i];
	sc[pointer][1]=0;
	sc_hit++;
    }
    else
      sc[position][1]=1;
  }    
    /*if(sc_findpage(inputarr[i]))
    {
      if((position = sc_findemptyframe())!=-1)
      {
	sc_insert(inputarr[i],position);
      }
      else
      {
	position=sc_locationposition(inputarr[i]);
	sc_insert(inputarr[i],position);
      }
    }
  }*/
}
void readParameters(const char *strPara)
{
	char *tmp=NULL;
	FILE *fp;
	int error_flag=1;
	fp=NULL;
	frames =5; //default value
		tmp = strstr(strPara, "-h");
		if(tmp!=NULL)
		{	
			help_error();
			tmp=NULL;
			return;
		}
		tmp = strstr(strPara, "-f");
		if(tmp!=NULL)
		{	char tmpframes[10];
			extractFilename(tmp,tmpframes);
			frames = atoi(tmpframes);
			if(frames<=0 || frames>=MAXFRAMES)
			{
			  fprintf(stderr,"\nERROR: Frame cannot be set to %d\n",frames);
			  help_error();
			}
			tmp=NULL;
			error_flag=0;
		}
		tmp = strstr(strPara, "-r");
		if(tmp!=NULL)
		{	
			extractFilenametoupper(tmp,policy);
			if(strcmp(policy,"LRU")==0 || strcmp(policy,"LFU")==0 || strcmp(policy,"FIFO")==0 || strcmp(policy,"SC")==0)
			  tmp=NULL;
			else
			{
			  fprintf(stderr,"\nERROR: Invalid policy selected %s\n",policy);
			  help_error();
			}	
			error_flag=0;
		}
		tmp = strstr(strPara, "-i");
		if(tmp!=NULL)
		{
			int i=0,j=0;
			extractFilename(tmp,filepath);
			fp=fopen(filepath,"r");
			if(fp==NULL)
			{
			  fprintf(stderr,"\nERROR: INPUT FILE DOESNOT EXISTS: %s\n",filepath);
			  exit(1);
			}
			filemode = 1;
			while(!feof(fp))
			{
			  fscanf(fp,"%d",&inputarr[i++]);
			}
			inputlen=i-1;
			tmp=NULL;
			error_flag=0;
		}
		if (fp==NULL)
		{
		      char strinput[ARRMAX];int i=0;
		      printf("\nEnter the page reference sequence separated by a space (limit: %d)\nPress enter to continue:\n",i=ARRMAX);
		      fgets(strinput,ARRMAX,stdin);
		      extractInput(strinput);
		      error_flag=0;
		}
		if(error_flag)
		{
		    fprintf(stderr,"\nWrong choice\n",filepath);
		    help_error();
		}
}
void extractInput(char *t)
{
  int i=0,j=0,k=0;
  char sub[3]; //3 digits
  for(i=0;t[i]!='\0';i++)
  {
    if(t[i]!=' ')
    {
      sub[j++]=t[i];
    }
    else
    {
      inputarr[k]=atoi(sub);
      k++;
      bzero(sub,3);
      j=0;
    }
  }
  inputarr[k]=atoi(sub);
  inputlen=k+1;

}
void extractFilename(char *t,char *desti)
{
	int i =0;
	t=t+3; //cross -x and a space
	for(i=0;t[i]!=' ';i++) 
		desti[i]=t[i];
	desti[i]='\0';
}
void extractFilenametoupper(char *t,char *desti)
{
	int i =0;
	t=t+3; //cross -x and a space
	for(i=0;t[i]!=' ';i++) 
		desti[i]=toupper(t[i]);
	desti[i]='\0';
}
void output(int hits)
{
  float f=0.0;
  hits = hits-frames>=0?hits-frames:0;
  opt_hit = opt_hit-frames>=0?opt_hit-frames:0;
  f = hits-opt_hit;
  if(f>0.0)
    f = f/(float)opt_hit;
  f = f*100.0;
  printf("\n# of page replacements with %s :\t%d",policy,hits);
  printf("\n# of page replacements with OPT:\t%d",opt_hit);
  printf("\n%%penalty using %s:\t%0.2f%%\n\n",policy,f);
}
void help_error()
{
  printf("\nVIRTUAL MEMORY MANAGER\nUsage:");
  printf("\n-h:\t\tHelp/usage");
  printf("\n-f number:\tSet the number of frames. Default is 5");
  printf("\n-r policy:\tSet page replacement policy. Can be FIFO, SC, LFU, LRU. Default is FIFO");
  printf("\n-i input_file:\tInput containing the page reference sequence. Default is STDIN\n");
  exit(1);
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
	runopt(); //OPT runs here
	if(strcmp(policy,"LRU")==0)
	{
	    runlru();
	    policymode=1;
	}
	else if(strcmp(policy,"FIFO")==0)
	{
	    runfifo();
	    policymode=2;
	}
	else if(strcmp(policy,"SC")==0)
	{
	    runsc();
	    policymode=3;
	}
	else if(strcmp(policy,"LFU")==0)
	{
	    runlfu();
	    policymode=4;
	}
	switch (policymode)
	{
	case 4: output(lfu_hit); break;
	case 3: output(sc_hit); break;
	case 2:	output(fifo_hit); break;
	case 1:	output(lru_hit); break;
	}
return 0;
}
