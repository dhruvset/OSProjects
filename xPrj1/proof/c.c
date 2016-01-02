#include "stdio.h"
#include "string.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#define SER_SIZE 1000
int main()
{
	int sfd,len,result;
	char buff[SER_SIZE],ch;
	FILE *fd,*list;
	struct sockaddr_in c;
	sfd=socket(AF_INET,SOCK_STREAM,0);
	
	c.sin_family=AF_INET;
	c.sin_addr.s_addr=inet_addr("127.0.0.1");
	c.sin_port=9990;
	len=sizeof(c);
	//dialogbox_q("Press OK when ready?");		
	result=connect(sfd,(struct sockaddr *)&c,len);
	if(result==-1)
	{
		printf("The connection cannot be established\nERROR\n");
		exit(1);
	}
	//dialogbox("The connection is being established\nList has been downloaded");
	write(sfd,"Get /file/fi1 http1.1",23);//dir
	//write(sfd,"FFFFFFFFFFFf",10);//table
return 0;
}
