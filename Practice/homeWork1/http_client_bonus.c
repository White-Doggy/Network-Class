#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

void error_handling(char *message);

int main(int argc, char* argv[]) {
	
int sock;
	struct sockaddr_in serv_addr;
	char message[512];
	int str_len=0;
	int idx=0, read_len=0;

	if(argc!=3){
		printf("Usage : %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	sock=socket(PF_INET, SOCK_STREAM, 0);

	if(sock ==-1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error!");
	
	char msg[500]="GET /webhp HTTP/1.1\r\n";
	strcat(msg,"User-Agent: Mozilla/4.0\r\n");
	strcat(msg,"content-type:text/html\r\n");
	strcat(msg,"Connection: close\r\n\r\n");

	send(sock, msg, strlen(msg), 0);

	FILE *fl = fopen("hw1.html","w");

	while(read_len=read(sock, message,512))
	{
		if(read_len==-1)
		{
			error_handling("read() error!");
			break;
		}
		
		fprintf(fl, "%s",message);
		memset(message, 0, 512);

	}

	close(sock);
	return 0;
}


void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

