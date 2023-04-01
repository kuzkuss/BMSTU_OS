#ifndef _SOCKET_H_
#define _SOCKET_H_

#define SOCK_NAME "socket.soc"

#define MAX_MSG_LEN 128
#define MAX_LEN_ERR_MSG 256

#define LEN_STRUCT_SOCKADDR(a) strlen(a.sa_data) + sizeof(a.sa_family) + 1

#define TRUE 1

#define ERROR_CREATE_SOCKET 1
#define ERROR_BIND_SOCKET 2
#define ERROR_RECVFROM_SOCKET 3
#define ERROR_SENDTO_SOCKET 4

#endif