#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define SMALL_BUF 100

void send_data(int sock, char* ct, char* file_name);
char* content_type(char* file);
void send_error(int sock);
int get_file_size(char* path);

void error_handling(char* msg);
void read_childproc(int sig);   

int main(int argc, char* argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;   

	pid_t pid;
	struct sigaction act;
	socklen_t addr_sz;
	int str_len, state;
	char buf[BUF_SIZE];

	if( argc != 2 )
	{
	printf("usage : %s <port>\n", argv[0]);
	exit(1);
	}
	   
	act.sa_handler = read_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	state = sigaction(SIGCHLD, &act, 0);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1 )
		error_handling("bind() error");
	if( listen(serv_sock, 5) == -1 )
		error_handling("listen() error");

	printf("[ server ready ]\n");

	while(1)
	{
		addr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &addr_sz);

		if( clnt_sock == -1 )
			continue;
	
		pid = fork();
		
		// parent process
		if( pid == -1 )
		{
			close(clnt_sock);
			continue;
		}

		
		// child process
		if( pid == 0 )
		{
			close(serv_sock);
			
			char req_line[SMALL_BUF];
			FILE* clnt_read;

			char method[10];			
			char ct[15];
			char file_name[30];

			clnt_read=fdopen(dup(clnt_sock), "r");
			fgets(req_line, SMALL_BUF, clnt_read);	

			if(strstr(req_line, "HTTP/")==NULL)
			{
				send_error(clnt_sock);
				fclose(clnt_read);
				return 0;
			}
			
			strcpy(method, strtok(req_line, " /"));
			strcpy(file_name, strtok(NULL, " /"));
			strcpy(ct, content_type(file_name));
			
			if(strcmp(method, "GET")!=0)
			{
				send_error(clnt_sock);
				fclose(clnt_read);			
			}

			fclose(clnt_read);
			send_data(clnt_sock, ct, file_name);
			
			return 0;
		}
		else
			close(clnt_sock);

	}	   

	close(serv_sock);
	return 0;
}  

void send_data(int sock, char* ct, char* file_name)
{
	char msg[500]="HTTP/1.1 200 OK\r\n";
	char cnt_type[SMALL_BUF];
	char buf[BUF_SIZE];
	FILE* send_file;
	int len;

	int size;
	char size_char[30];

	sprintf(cnt_type, "Content-type:%s\r\n", ct);
	send_file=fopen(file_name, "r");
	
	if(send_file==NULL)
	{
	send_error(sock);
	return;
	}

	size = get_file_size(file_name);
	sprintf(size_char, "%d", size);

	printf("file name : %s\n", file_name);
	printf("file size : %d\n", size);
	

	strcat(msg,cnt_type);
	strcat(msg,"Content-length:");
	strcat(msg,size_char);
	strcat(msg,"\r\n\r\n");

	send(sock, msg, strlen(msg), 0);
	//printf("%s\n",msg);

	while((len=fread(buf,sizeof(char),BUF_SIZE,send_file))> 0)
	{
		send(sock,buf,len,0);
	}
	
}

char* content_type(char* file)
{
	char extension[SMALL_BUF];
	char file_name[SMALL_BUF];
	strcpy(file_name, file);
	strtok(file_name, ".");
	strcpy(extension, strtok(NULL, "."));
	
	if(!strcmp(extension, "html")||!strcmp(extension, "htm"))
		return "text/html";
	else if(!strcmp(extension, "jpg")||!strcmp(extension, "jp"))
		return "image/jpg";
	else if(!strcmp(extension, "png")||!strcmp(extension, "pn"))
		return "image/png";
	else
		return "text/plain";	
}

void send_error(int sock)
{
	char msg[500]="HTTP/1.1 404 Not Found\r\n";
	strcat(msg,"Content-type: text/html \r\n");
	strcat(msg,"Content-length:180\r\n\r\n");
	strcat(msg,"<html><head><title> NETWORK </title></head>"
			"<body> No file Error, check your input </body></html>");
	
	send(sock, msg, strlen(msg), 0);
	//printf("%s\n",msg);	

	close(sock);
	
}

int get_file_size(char* path)
{
	int size;
	FILE *fp = fopen(path, "r");
	
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	
	fclose(fp);

	return size;
}


void read_childproc(int sig)

{
	pid_t pid;
	int status;
	pid = waitpid(-1, &status, WNOHANG);	
}

void error_handling(char* msg)

{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
