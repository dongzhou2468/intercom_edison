/*
	notice app when switch is pressed
*/
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>

#include <string.h>

int main (int argc, char **argv)
{
	int sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
	if(sclient == -1)  
	{  
		printf("socket error !");  
		return 0;  
	}
	struct sockaddr_in sin;
	bzero(&sin, sizeof(sin)); 
    	sin.sin_family = AF_INET;  
    	sin.sin_port = htons(4144);  
    	sin.sin_addr.s_addr = inet_addr("192.168.1.114");   
    	if(connect(sclient, (struct sockaddr *)&sin, sizeof(sin)) == -1)
	{
        	printf("bind error !");
		return 0;
	}
	
   	char *sendData = "1024";   
	send(sclient, sendData, strlen(sendData), 0);

	close(sclient);
	return 0;
}
