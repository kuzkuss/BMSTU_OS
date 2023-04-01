#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "socket.h"

void form_message(char buf[MAX_MSG_LEN], int argc, char *argv[]) {
    sprintf(buf, "\nClient: %d\nMessage: ", getpid());

    if (argc < 2)
        strcat(buf, "Hello!");
    else
        strcat(buf, argv[1]);
}

int main(int argc, char *argv[])
{
	struct sockaddr srvr_addr;
	int sock;

	char buf[MAX_MSG_LEN];
	form_message(buf, argc, argv);

	if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket failed");
		return ERROR_CREATE_SOCKET;
	}

	srvr_addr.sa_family = AF_UNIX;
	strcpy(srvr_addr.sa_data, SOCK_NAME);

	if (sendto(sock, buf, strlen(buf) + 1, 0, &srvr_addr, LEN_STRUCT_SOCKADDR(srvr_addr)) == -1)
	{
		perror("sendto failed");
		return ERROR_SENDTO_SOCKET;
	}

	close(sock);
	printf("Send message: %s", buf);
	return 0;
}