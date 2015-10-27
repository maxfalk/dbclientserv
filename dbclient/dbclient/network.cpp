#include "network.h"
#include <WinSock2.h>
#include <stdio.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

SOCKET msocket;
struct sockaddr_in mserver;

int network_init(){

	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return WSAGetLastError();
	}

	return 0;
}
int network_create_socket(){



	//Create a socket
	if ((msocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		return WSAGetLastError();
	}

	return 0;

}
int network_connect_to_host(const char *ipaddr, int port){

	inet_pton(AF_INET, ipaddr, &(mserver.sin_addr.s_addr));
	mserver.sin_family = AF_INET;
	mserver.sin_port = htons(port);

	//Connect to remote server
	if (connect(msocket, (struct sockaddr *)&mserver, sizeof(mserver)) < 0)
	{
		return -1;
	}


	return 0;

}
int network_send_packet(char *message, int length){
	//Send some data

	if (send(msocket, message, length, 0) < 0)
	{

		return -1;
	}

	return 0;
}

int network_receive_packet(char *server_reply, int length){
	//Receive a reply from the server
	int recv_size;


	if ((recv_size = recv(msocket, server_reply, length - 1, 0)) == SOCKET_ERROR)
	{
		return -1;
	}

	//Add a NULL terminating character to make it a proper string before printing
	server_reply[recv_size] = '\0';
	puts(server_reply);


	return 0;

}

