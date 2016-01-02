#include "stdio.h"
#include "string.h"
#include <unistd.h>
void extn(char *t, char *d)
{
	int i=0,j=0;
	for(i=0;i<=30;i++)
		if(t[i]=='.') break;
	i++;
	for(j=0;t[i]!='\0';i++,j++)
		d[j]=t[i];
	d[j]='\0';
}
int main()
{
	char str[30],ex[10];
	strcpy(str,"/root/fefef.html");
	extn(str,ex);
	printf("\n%s",ex);
	gethostname(str,30);
	printf("\n%s",str);
return 0;
}
