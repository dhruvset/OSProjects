#include "stdio.h"
#include "string.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#define SER_SIZE 1000
int main()
{
	int sfd,len,result;
	char buff[SER_SIZE],ch;
	FILE *fd,*list;
	struct sockaddr_in c;
	struct hostent *hp;
  char hostname[250];
	sfd=socket(AF_INET,SOCK_STREAM,0);
	
	gethostname(hostname,250);
	hp = gethostbyname(hostname);
	puts(hostname);

	bcopy((void*)hp->h_addr,(void*)&c.sin_addr,hp->h_length);
	c.sin_family=AF_INET;
	//c.sin_addr.s_addr=inet_addr("127.0.0.1");
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
	write(sfd,"Get /root/Desktop/index.html http1.1\r\n",50);//dir
	//write(sfd,"FFFFFFFFFFFf",10);//table
	while(1)
	{
		read(sfd,buff,SER_SIZE);
		puts(buff);
	}
return 0;
}
