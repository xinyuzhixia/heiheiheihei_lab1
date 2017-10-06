#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>


#define MAXBUFLEN 2000


int main(int argc, char *argv[] )
{

int sockfd;
char buf[MAXBUFLEN];
int numbytes;
struct sockaddr_storage their_addr;
socklen_t addr_len;
char FileName[100];
struct addrinfo hints, *myServer;



char FileBuff[1024*1024];

 if (argc != 3) {
 fprintf(stderr,"usage: UDP listen port\n");
 return 1;
 }

// SOCKET TO TRANSFER FILE
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; 
hints.ai_socktype = SOCK_DGRAM;
hints.ai_flags = AI_PASSIVE; 
getaddrinfo(argv[1], argv[2], &hints, &myServer);

sockfd = socket(myServer->ai_family, myServer->ai_socktype, myServer->ai_protocol);

printf("%s: ","ftp");
scanf("%s", FileName);

FILE *fp = fopen(FileName, "r");

// open another socket to listen ACK from server
struct addrinfo hints2,*myHost, *p;
int ACKfd;

memset(&hints2, 0, sizeof hints2);
hints2.ai_family = AF_UNSPEC; 
hints2.ai_socktype = SOCK_DGRAM;
hints2.ai_flags = AI_PASSIVE; 

int NewPort;
sscanf(argv[2], "%d", &NewPort);
NewPort += 1;
char temp[sizeof(int)];
sprintf(temp,"%d",NewPort);

getaddrinfo(NULL, temp, &hints2, &myHost);


	for(p = myHost; p!= NULL; p = p -> ai_next){

		if ((ACKfd= socket(myHost-> ai_family, myHost->ai_socktype, myHost->ai_protocol)) == -1){
			
			close(ACKfd);
	 		perror("listener: bind");
	 		continue;

		}

		//printf("%d\n", sockfd);
		if(bind(ACKfd, myHost->ai_addr, myHost->ai_addrlen) == -1){
			
			close(ACKfd);
	 		perror("listener: bind");
	 		continue;

		}

		printf("%d\n", ACKfd);
		break;
	}

//printf("%s\n",FileName );
char* msg = "ftp";

if(fp){

	fseek(fp, 0L, SEEK_END);
	long int sz = ftell(fp);
	rewind(fp);
	if (fread(FileBuff, sz, 1, fp) != 1){

		printf("%s\n", "Error!");
	}
 	
	int numpkt = sz/1000 + 1;
	int pck = 0;
	for (;pck < numpkt;pck++)
	{
		int size;
		if (pck != numpkt - 1)
		{
			size = 1000;
			
		}
		else 
		{
			 size = sz%1000;
			
		}

		char SentPacket[2000];
		sprintf(SentPacket,"%d:%d:%d:%s:",numpkt,pck,size,FileName);
		int packetheaderlen = strlen(SentPacket);
		memcpy(SentPacket+packetheaderlen,FileBuff+(pck*1000),size);
		//SentPacket[packetheaderlen+size] = '\0';
		numbytes = sendto(sockfd, SentPacket, sizeof(SentPacket), 0, myServer->ai_addr, myServer->ai_addrlen);
		numbytes = recvfrom(ACKfd, buf, MAXBUFLEN-1, 0,(struct sockaddr *)&their_addr, &addr_len);

		while (strcmp(buf, "ACK")){	
			// not ACK
			numbytes = recvfrom(ACKfd, buf, MAXBUFLEN-1, 0,(struct sockaddr *)&their_addr, &addr_len);

	 	}
	}

 	fclose(fp);

}


freeaddrinfo(myServer);
close(sockfd);
close(ACKfd);
return 0;



}