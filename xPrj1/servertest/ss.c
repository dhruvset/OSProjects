#include "stdio.h"
#include "string.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#define SER_SIZE 40
int main(int argc, char *argv[])
{
	int sfd,cfd;
	unsigned int len;
	char buff[SER_SIZE],cmd[1000]={'\0'};
	struct sockaddr_in ser,c;
	sfd=socket(AF_INET,SOCK_STREAM,0);
	
	ser.sin_family=AF_INET;
	ser.sin_addr.s_addr=htonl(INADDR_ANY);//inet_addr(argv[1]);
	ser.sin_port=htons(9990);
	len=sizeof(ser);
	
	bind(sfd,(struct sockaddr *)&ser,len);
	listen(sfd,1);
	len=sizeof(c);
	
	while(1)
	{
		printf("\nSERVER WAITING\n");
		cfd=accept(sfd,(struct sockaddr*)&c,&len);
		
		char *whoisit = inet_ntoa(c.sin_addr);
		FILE * r_connection = fdopen(cfd,"r");
		FILE * w_connection = fdopen(cfd,"w");

		fprintf(w_connection,"HELLO WORLD %s\r\n",whoisit);
		fflush(w_connection);
		fclose(w_connection);
		fclose(r_connection);
		close(cfd);
	}
		
return 0;
}


