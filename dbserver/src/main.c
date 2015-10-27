#include <stdio.h>
#include "sqlite3.h" 
#include "db.h"
#include <WinSock2.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define DEFAULT_BUFLEN 512
#define MAX_THREADS 4

SOCKET listen_socket;

close_socket(SOCKET s);
int init_socket();
int bind_to_socket();
int accept_connections(SOCKET *client_socket);
int query_callback(void *data, int argc, char **argv, char **col_name);
int send_packet(SOCKET socket, char *message, int length);
int receive(SOCKET client_socket);
int server();
int select_on_sockets();
void to_lower(char *statment, int len);
int is_select_stament(char *statment);

int init_socket(){

	WSADATA wsa;
	

	printf("\nInitialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d\n", WSAGetLastError());
		return -1;
	}

	printf("Initialised.\n");


	if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d\n", WSAGetLastError());
		return -1;
	}

	printf("Socket created.\n");
	return 0;
}

int bind_to_socket(){

	struct sockaddr_in server;

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	//Bind
	if (bind(listen_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d\n", WSAGetLastError());
		return -1;
	}

	puts("Bind done\n");

	//Listen
	listen(listen_socket, 3);


	return 0;
}
int select_on_sockets(){

	//set of socket descriptors
	fd_set readfds;
	int max_clients = 30, activity, addrlen, i, valread;
	SOCKET client_socket[30], socket, new_socket;
	struct sockaddr_in address;


	for (i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;
	}

	while (TRUE)
	{

		FD_ZERO(&readfds);
		FD_SET(listen_socket, &readfds);

		//add child sockets
		for (i = 0; i < max_clients; i++)
		{
			socket = client_socket[i];
			if (socket > 0)
			{
				FD_SET(socket, &readfds);
			}
		}

		//Accept and incoming connection
		puts("Waiting for incoming connections...\n");

		//wait for an activity on any of the sockets
		activity = select(0, &readfds, NULL, NULL, NULL);

		if (activity == SOCKET_ERROR)
		{
			printf("select call failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//If something happened
		if (FD_ISSET(listen_socket, &readfds))
		{
			if (accept_connections(&new_socket) != 0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			printf("New connection , socket fd is %d , ip is : %s , port : %d \n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

			//add new socket to array of sockets
			for (i = 0; i < max_clients; i++)
			{
				if (client_socket[i] == 0)
				{
					client_socket[i] = new_socket;
					printf("Adding to list of sockets at index %d \n", i);
					break;
				}
			}
		}

		//else its some IO operation on some other socket :)
		for (i = 0; i < max_clients; i++)
		{
			socket = client_socket[i];
			//if client presend in read sockets             
			if (FD_ISSET(socket, &readfds))
			{
				//get details of the client
				getpeername(socket, (struct sockaddr*)&address, (int*)&addrlen);

				//Check if it was for closing , and also read the incoming message
				valread = receive(socket);
				if (valread <= 0){
					closesocket(socket);
					client_socket[i] = 0;
				
				}

			}
		}
	}


}
int accept_connections(SOCKET *client_socket){

	struct sockaddr_in client;
	
	int c;

	c = sizeof(struct sockaddr_in);
	*client_socket = accept(listen_socket, (struct sockaddr *)&client, &c);
	if (*client_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d\n", WSAGetLastError());
		return -1;
	}

	puts("Accepted connection\n");


	return 0;


}

int query_callback(void *data, int argc, char **argv, char **col_name){

	
	if (argc > 0 && strcmp(argv[0], col_name[0])){
		char packet[500];
		SOCKET client_socket = (SOCKET)data;	
		char begin[] = "{\"row\":{";
		char end[] = "}}";

		//Send beginning of new row
		send_packet(client_socket, begin, strlen(begin));
		//send all columns and their data
		for (int i = 0; i < argc; i++){
			//printf("%s = %s", col_name[i], argv[i] ? argv[i] : "NULL");
			int pos = 0;
			packet[0] = '\"';
			pos += 1;
			strcpy(packet + pos, col_name[i]);
			pos += strlen(col_name[i]);
			packet[pos] = '\"';
			pos += 1;
			packet[pos] = ':';
			pos += 1;
			packet[pos] = '\"';
			pos += 1;
			strcpy(packet + pos, argv[i]);
			pos += strlen(argv[i]);
			packet[pos] = '\"';
			pos += 1;

			if (i < (argc - 1)){
				packet[pos] = ',';
				pos += 1;
			}


			
			send_packet(client_socket, packet, pos);
			packet[pos] = '\0';
			//printf("packet %d of %d, len: %d:  %s \n", i, argc, strlen(packet), packet);
		}
		//End end of row
		send_packet(client_socket, end, strlen(end));
		//printf("\n");
	}
	
	return 0;
}
int send_packet(SOCKET socket, char *message, int length){
	//Send some data

	if (send(socket, message, length, 0) < 0)
	{
		puts("Send failed");
		return -1;
	}

	return 0;
}

int receive(SOCKET client_socket){

	
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	struct sockaddr_in address;
	int addrlen = sizeof(struct sockaddr_in);

	getpeername(client_socket, (struct sockaddr*)&address, (int*)&addrlen);

	// Receive until the peer shuts down the connection
	iResult = recv(client_socket, recvbuf, recvbuflen, 0);
		
	if (iResult > 0) {
		recvbuf[iResult] = '\0';
		printf("Bytes received: %d\n", iResult);
		printf("Received: %s \n", recvbuf);
		if (is_select_stament(recvbuf) == 0){
			query_db(recvbuf, query_callback, client_socket);
		}
		else{
			printf("Was not a select stament\n");		
		}
		send_packet(client_socket, "\"end\"", strlen("\"end\""));
	} else if (iResult == 0){
		printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	}else if (iResult == SOCKET_ERROR){
		int errorcode = WSAGetLastError();
		if (errorcode  == WSAECONNRESET){
			printf("Host disconnected unexpectedly , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
		}
		else{
			printf("recv failed: %d\n", errorcode);
		}
			
	}



	return iResult;

}

int is_select_stament(char *statment){
	int pos = 0;
	
	while (statment[pos] == ' ') pos++;
	to_lower(statment, strlen(select));
	if ((strstr(statment, "select") - statment) == pos){
		return 0;
	}
	else{
		return 1;
	}


}
void to_lower(char *statment, int len){

	for (int i = 0; i < len; i++){
		statment[i] = tolower(statment[i]);
	}

}

int server(){

	if (!init_socket()){
		if (!bind_to_socket()){
			select_on_sockets();
		}	
	}
	WSACleanup();
	return 0;
}

int main(int argc, char* argv[])
{

	open_db();
	server();
	
	close_db();

	return 0;
}
