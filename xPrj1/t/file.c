#include "stdio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//void readit(FILE *fp);
void readit(int fp)
{
	int c;
	FILE *f;
	char buff[10];
	c = fp;

	f = fdopen(c,"r");
	fgets(buff,10,f);
	printf(":%s:",buff);
}
int main()
{
	FILE *fp;
	int f;
	f = open("/index.html",O_RDONLY);
	//fp = fopen("/index.html","r");
	readit(f);
}
