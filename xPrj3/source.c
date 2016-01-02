#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <ctype.h>
#define defaultPOLICY "FCFS"
#define defaultFilepath "STDIN"
#define MAXCY 1000
#define ARRMAX 1000

static int hits=0; //i know it starts with 0
static int cy = 200-1; //default
static int inputarr[ARRMAX];
static int inputlen=0;
static int shead = 0;
static char policy[10] = defaultPOLICY;
static char filepath[ARRMAX]= defaultFilepath;
static int filemode = 0; //0 for stdin mode and 1 for file mode

void runfcfs()
{
  int i=0;
  hits = abs(inputarr[i]-shead);
  printf("\nFCFS: ");
  for(i=0;i<inputlen-1;i++)
  {
    hits+=abs(inputarr[i]-inputarr[i+1]);
    printf("%d ",inputarr[i]);
  }
  printf("%d ",inputarr[i]);
}
int sstf_closest(int element)
{
  int i=0, ele=-1;
  int dis=0,pos=-1;
  for(i=0;i<inputlen;i++)
  {
     if(inputarr[i]!=-1)
     {
        dis=inputarr[i];
        break;
     }
  }
  for(i=0;i<inputlen;i++)
  {
     if(dis>abs(inputarr[i]-element) && inputarr[i]!=-1)
      {
	dis = abs(inputarr[i]-element);
	pos = i;
      }
  }
  return pos;
}
void runsstf()
{
  int i=0,d=0;
  int element=shead;
  printf("\nSSTF: ");
  for(i=0;i<inputlen;i++)
  {
     d=sstf_closest(element);
     printf("%d ",inputarr[d]);
     hits+= abs(element-inputarr[d]);
     element = inputarr[d];
     inputarr[d]=-1;
  }
}
void sort_input()
{
  int i,j,temp;
  for(i=0;i<inputlen;i++)
  {
    for(j=0;j<inputlen-1;j++)
    {
      if(inputarr[j]>inputarr[j+1])
      {
	  temp = inputarr[j+1];
	  inputarr[j+1] = inputarr[j];
	  inputarr[j] = temp;
      }
    }
  }
}
void runcscan()
{
  int i,j=0,next=-1;
  int element = shead;
  printf("\nC-SCAN: %d ",shead);
  inputarr[inputlen++]=0;
  sort_input();
  for(i=0;i<inputlen;i++)
     if(inputarr[i]==cy)
        j=1;
  if(j!=1)
     inputarr[inputlen++]=cy; //added the values
  for(i=0;i<inputlen;i++)
    if(inputarr[i]>element)
      break;
  inputlen--;
  for(j=i;j<inputlen;j++)
  {
      printf("%d ",inputarr[j]);
      hits+=abs(inputarr[j]-element);
      element = inputarr[j];
      inputarr[j]=-1;
  }
  printf("%d ",inputarr[j]);
  hits+=abs(inputarr[j]-element);
  //break the hits if all elements are visited, i.e equal to -1
  for(i=0,j=0;i<inputlen;i++)
  {
     if(inputarr[i]==-1)
        j++;
  }
  if(j==inputlen-1)
    return;
  
  hits+=cy;
  element=0;
  for(i=0;i<inputlen;i++)
  {
     if(inputarr[i]!=-1)
     {
        printf("%d ",inputarr[i]);
        hits+=abs(inputarr[i]-element);
        element = inputarr[i];
        inputarr[j]=-1;
     }
     else
        break;
  }
}
void runlook()
{
  int i=0,j,second,last1;
  int element = shead;
  printf("\nLOOK: %d ",shead);
  sort_input();
  
  for(i=0;i<inputlen;i++)
    if(inputarr[i]>element)
      break;
  second=i-1;    
  for(j=i;j<inputlen;j++)
  {
      printf("%d ",inputarr[j]);
      hits+=abs(inputarr[j]-element);
      element = inputarr[j];
      inputarr[j]=-1;   
  }
  hits+=abs(inputarr[second]-element);
  //printf("x%d ",inputarr[second]);
  element = inputarr[second-1];
  hits+=abs(inputarr[second]-element);
  inputarr[second]=-1;
  for(i=second-2;i>0;i--)
  {
      printf("%d ",element);
      hits+=abs(inputarr[i]-element);
      element = inputarr[i];
      inputarr[i]=-1;
  }
  printf("%d %d",element,inputarr[i]);
  hits+=abs(inputarr[i]-element);
}
void extractInput(char *t)
{
  int i=0,j=0,k=0;
  char sub[3]; //3 digits
  for(i=0;t[i]!='\0' && t[i]!='\n';i++)
  {
    if(t[i]!=' ')
    {
     if(isalpha(t[i]))
     {
        printf("\nWarning: Illegal input %c. Try again\n",t[i]);
	exit(1);
     }
     else
      sub[j++]=t[i];
    }
    else
    {
      inputarr[k]=atoi(sub);
      if(inputarr[k]<0)
      {
         printf("\nWarning: Illegal input %d. Try again\n",inputarr[k]);
	 exit(1);
      }
      else if(inputarr[k]>=cy)
      {
	printf("\nWarning: Illegal input %d, exceeds the number of cylinders. Try again\n",inputarr[k]);
	exit(1);
      }
      else
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
void help_error()
{
  printf("\nDisk Scheduling simulator\nUsage:");
  printf("\n-h:\t\tHelp/usage");
  printf("\n-n number of cylinders:\tSet the number of cylinders. Default is 200.");
  printf("\n-d policy:\tSet the disk scheduling policy. Can be FCFS, SSTF, C-SCAN, LOOK. Default is FCFS");
  printf("\n-t start_head_at:\tSet the starting of the head at position. Default is set to 0.");
  printf("\n-i input_file:\tInput containing the disk reference sequence. Default is STDIN\n");
  exit(1);
}
void readParameters(const char *strPara)
{
	char *tmp=NULL;
	FILE *fp;
	int error_flag=1,flag_for_t=1;
	fp=NULL;
	cy =199; // 200 = default value
		tmp = strstr(strPara, "-h");
		if(tmp!=NULL)
		{	
			help_error();
			tmp=NULL;
			return;
		}
		tmp = strstr(strPara, "-t");
		if(tmp!=NULL)
		{	
			char tmphead[10];
			extractFilename(tmp,tmphead);
			shead = atoi(tmphead);
			tmp=NULL;
			flag_for_t=0; //not visited hence set inputarr[0] to shead
		}
		tmp = strstr(strPara, "-n");
		if(tmp!=NULL)
		{	char tmpcy[10];
			extractFilename(tmp,tmpcy);
			cy = atoi(tmpcy);
			if(cy<=0 || cy>=MAXCY)
			{
			  fprintf(stderr,"\nERROR: Cylinders cannot be set to %d\n",cy);
			  help_error();
			}
			cy = cy-1; //reducing the number
			tmp=NULL;
			error_flag=0;
		}
		tmp = strstr(strPara, "-d");
		if(tmp!=NULL)
		{	
			extractFilenametoupper(tmp,policy);
			if(strcmp(policy,"FCFS")==0 || strcmp(policy,"SSTF")==0 || strcmp(policy,"C-SCAN")==0 || strcmp(policy,"LOOK")==0)
			  tmp=NULL;
			else
			{
			  fprintf(stderr,"\nERROR: Invalid disk policy selected %s\n",policy);
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
			if(flag_for_t)
			  shead=inputarr[0];
		}
		if (fp==NULL)
		{
		      char strinput[ARRMAX];int i=0;
		      printf("\nEnter the page reference sequence separated by a space (limit: %d)\nPress enter to continue:\n",i=ARRMAX);
		      fgets(strinput,ARRMAX,stdin);
		      extractInput(strinput);
		      error_flag=0;
		      if(flag_for_t)
			  shead=inputarr[0];
		}
		if(error_flag)
		{
		    fprintf(stderr,"\nWrong choice\n",filepath);
		    help_error();
		}
}
void output(int hits)
{
  printf("\nHead travelled %d \n",hits);
}
int main(int argc, char *argv[])
{
	char args[ARRMAX];
	int i=0;
	for(i=0;i<argc;i++)
	{
	  strcat(args,argv[i]);
	  strcat(args," ");
	}
	readParameters(args);
	if(strcmp(policy,"FCFS")==0)
           runfcfs();
	else if(strcmp(policy,"SSTF")==0)
	   runsstf();        
	else if(strcmp(policy,"C-SCAN")==0)
           runcscan();
	else if(strcmp(policy,"LOOK")==0)
           runlook();
	output(hits);
return 0;
}
