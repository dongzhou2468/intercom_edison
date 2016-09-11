/*
  turn on/off fan using relay, controlled by app
  compile: gcc -o fan fan.c -lmraa
  run: ./fan $gpio
*/
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>

#include <string.h>

#include <mraa.h>

int main (int argc, char **argv)
{
	mraa_gpio_context gpio;
	gpio = mraa_gpio_init(atoi(argv[1]));
	mraa_gpio_dir(gpio, MRAA_GPIO_OUT);

	//off at first
	mraa_gpio_write(gpio, 1);

	int slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
	if(slisten == -1)  
	{  
		printf("socket error !");  
		return 0;  
	}
	struct sockaddr_in sin;
	bzero(&sin, sizeof(sin)); 
    	sin.sin_family = AF_INET;  
    	sin.sin_addr.s_addr = htonl(INADDR_ANY);   
    	sin.sin_port = htons(4143);  
    	if(bind(slisten, (struct sockaddr *)&sin, sizeof(sin)) == -1)
	{
        	printf("bind error !");
		return 0;
	}
	if(listen(slisten, 5) == -1)  
    	{  
        	printf("listen error !");  
	        return 0;  
    	}
	
	int sClient;  
	struct sockaddr_in remoteAddr;  
	int nAddrlen = sizeof(remoteAddr);  
   	char revData[20];   
   	while (1)  
    	{  
        	//printf("waiting for connectioné•...\n");  
		sClient = accept(slisten, (struct sockaddr *)&remoteAddr, &nAddrlen);  
        	if(sClient == -1)  
	        {  
        		printf("accept error !");  
       			continue;  
        	}  
        	printf("a connection received: %s \r\n", inet_ntoa(remoteAddr.sin_addr));  
        	recv(sClient, revData, 255, 0);          
	        if(atoi(revData) == 0000)  
        	{
			mraa_gpio_write(gpio, 0);
		        printf("fan on\n");  
        	}
		else if(atoi(revData) == 1111)
		{
			mraa_gpio_write(gpio, 1);
			printf("fan off\n");
		}
        	//char * sendData = "1111";  
        	//send(sClient, sendData, strlen(sendData), 0);  
        	close(sClient);  
    	}
	close(slisten);
	return 0;
}
