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


#define MAXBUFLEN 2000


int main(int argc, char *argv[] )
{

	int sockfd;
	char buf[MAXBUFLEN];
	int numberbytes;
	struct sockaddr_in their_addr;
	socklen_t addr_len;

	struct addrinfo hints, *myHost, *p;


	 //if (argc != 1) {
	 //printf("%s\n","usage: UDP listen port\n");
	 //return 1;
	 //}

	// Open SOCKET to transfer file
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; 
	 
	getaddrinfo(NULL, argv[1], &hints, &myHost);
	//struct sockaddr_in *clientaddr = (struct sockaddr_in*)&myHost->ai_addr;
	//unsigned char *ip = (unsigned char *)&clientaddr->sin_addr.s_addr;
	//printf ("server: %d %d %d %d \n",ip[0], ip[1], ip[2], ip[3]);
			

	for(p = myHost; p!= NULL; p = p -> ai_next){

		if ((sockfd= socket(myHost-> ai_family, myHost->ai_socktype, myHost->ai_protocol)) == -1){
			
			close(sockfd);
	 		perror("listener: bind");
	 		continue;

		}

		//printf("%d\n", sockfd);
		if(bind(sockfd, myHost->ai_addr, myHost->ai_addrlen) == -1){
			
			close(sockfd);
	 		perror("listener: bind");
	 		continue;

		}

		printf("%d\n", sockfd);
		break;
	}
	// get the socket info that opened by the deliver
	struct addrinfo hints2, *myClient;
	int ACKfd;
	memset(&hints2, 0, sizeof hints2);
	hints2.ai_family = AF_UNSPEC; 
	hints2.ai_socktype = SOCK_DGRAM;
	hints2.ai_flags = AI_PASSIVE; 

	char* msg1= "ACK";
	char* msg2= "NACK";
	addr_len = sizeof their_addr;
	int LastUpdatedNumber = -1;
	int EXtractFileName = 0;
	int total_frag,frag_no,size;
	char filename[100];
	char filedata[1000];
	char tempdata[1000];
	char *start = NULL;
	char *Namestart = NULL;
	FILE *fp = NULL;
	int OpenSocket = 0;


	do {

		numberbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,(struct sockaddr *)&their_addr, &addr_len);

		if (OpenSocket == 0){


			
			int NewPort;
			sscanf(argv[1], "%d", &NewPort);
			NewPort += 1;
			char temp[sizeof(int)];
			sprintf(temp,"%d",NewPort);

			getaddrinfo(inet_ntoa(their_addr.sin_addr), temp, &hints2, &myClient);
			ACKfd = socket(myClient->ai_family, myClient->ai_socktype, myClient->ai_protocol);
			OpenSocket = 1;

		}

		buf[numberbytes] = '\0';
		
		if (EXtractFileName == 0){
			start = buf;
			Namestart = start;
			int count = 0;

			while(1){
				start ++;
				if(*start == ':'){
					count ++;
			
					if (count == 3) {

						char* tmp = start;
						Namestart = tmp + 1;
					}

					if (count == 4){
						break;
					}
				}
		
			}

			memcpy(filename, Namestart, start - Namestart);
			filename[start - Namestart] = '\0';
			printf("%s \n",filename);
			EXtractFileName = 1;

			//open a new file
			const char * f = filename;
			fp = fopen(filename,"a");
			printf("%s\n",filename);

		}

		else {
			start = buf;
			int count = 0;

			while(1){
				start ++;
				if(*start == ':'){
					count ++;
					if (count == 4){
						break;
					}
				}
		
			}


		}
		// extracct metadata
		sscanf(buf,"%d:%d:%d:",&total_frag,&frag_no,&size);
		// extract file data
		memcpy(filedata, start + 1, size);

		// send ACK and write the file

		if (frag_no == LastUpdatedNumber + 1)
		{
			// the packet is correct
				LastUpdatedNumber++;
				fwrite(filedata,1,size,fp);
				numberbytes = sendto(ACKfd, msg1, strlen(msg1), 0, myClient->ai_addr, myClient->ai_addrlen);
				
				if (frag_no == total_frag-1)
					break;
		}

		else 
		{	// the packet is wrong
				numberbytes = sendto(ACKfd, msg2, strlen(msg2), 0, myClient->ai_addr, myClient->ai_addrlen);

		}
	

	}while(1);





	freeaddrinfo(myHost);
	fclose(fp);
	close(sockfd);
	close(ACKfd);
	return 0;

}