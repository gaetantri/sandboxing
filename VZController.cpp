/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string>

#define LISTEN_IP "127.0.0.1"
#define LISTEN_PORT 33336
#define CLIENT_IP "127.0.0.1"

#define NB_VZ 5
const int VZ_LIST[NB_VZ] = { 220,221,222,223,224};
int sockfd;

#define OK  1
#define NOK 0

void error(const char *msg)
{
	perror(msg);
	close(sockfd);
	exit(1);
}

int checkvzid(int vzid) {
	int ret = NOK;
	for(int i = 0 ; ret == NOK && i < NB_VZ ; i++) {
		printf("[VZID] target:%d ;;; test:%d", vzid, VZ_LIST[i]);
		if(VZ_LIST[i] == vzid)
			ret = OK;
	}
	return ret;
}


void* listenClient(void * arg) {
	int socketfd = (int) arg;
	char cmd[256];
	int n;
	int dupa, dupb;
	bzero(cmd,256);
	n = read(socketfd,cmd,255);
	if (n < 0) error("ERROR reading from socket");
	printf("Here is the message: %s\n",cmd);
	//Testing
	int vzid 	  = 0;
	int sessionid = 0;
	sscanf(cmd,"%d %d",&vzid, &sessionid);
	if(checkvzid(vzid) == OK) {
		char script[256];
		char toexec[256];
		sprintf(script, "/root/%d/run.sh",sessionid);
		sprintf(toexec, "vzctl exec %d \"%s\" \n", vzid,script);
		dupa = dup(1);
		dupb = dup(2);
		dup2(socketfd,1);
		dup2(socketfd,2);
		system(toexec);
		dup2(dupa,1);
		dup2(dupb,2);
	} else {
		error("COMMAND FORMAT NOT VALID!");
	}
	close(socketfd);
}

void listenClientnewThread(int socketfd) {
	pthread_t pthread;
	pthread_create(&pthread,NULL,listenClient,(void*)socketfd);
}


int main(int argc, char *argv[])
{
	int newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = LISTEN_PORT;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(LISTEN_IP);
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd,5);
	for(;;) {
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd,
				(struct sockaddr *) &cli_addr,
				&clilen);

		if (newsockfd < 0)
			error("ERROR on accept");
		else
			printf("Client connected. IP : %s \n",inet_ntoa(cli_addr.sin_addr));

		if(cli_addr.sin_addr.s_addr != inet_addr(CLIENT_IP)) {
			perror("Reject IP client");
			close(newsockfd);
		} else {
			listenClientnewThread(newsockfd);
		}
	}
	close(sockfd);
	return 0;
}
