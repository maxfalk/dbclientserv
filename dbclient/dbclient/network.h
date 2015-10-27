#ifndef _NETWORK_H__
#define _NETWORK_H__

#ifdef __cplusplus
extern"C" {
#endif

/****************************************************************************
*    FUNCTION: network_init()
*
*    PURPOSE: initializes the socket
****************************************************************************/
int network_init();

/****************************************************************************
*    FUNCTION: network_create_socket();
*
*    PURPOSE: creates a global socket.
****************************************************************************/
int network_create_socket();

/****************************************************************************
*    FUNCTION: network_connect_to_host(const char *, int port)
*
*    PURPOSE: Connects to a host at ipaddr and port, on the prepared socket
****************************************************************************/
int network_connect_to_host(const char *, int port);

/****************************************************************************
*    FUNCTION: network_send_packet(char *, int length)
*
*    PURPOSE: Sends a packet to the connected host
****************************************************************************/
int network_send_packet(char *, int length);

/****************************************************************************
*    FUNCTION: network_receive_packet(char *server_reply, int length)
*
*    PURPOSE: receives length number of packets from buffer 
****************************************************************************/
int network_receive_packet(char *server_reply, int length);




	
#ifdef __cplusplus 
} 
#endif 

#endif