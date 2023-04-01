#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "socket.h"

void clean_up(int sock)
{
	close(sock);
	unlink(SOCK_NAME);
}

int main(int argc, char *argv[])
{
	struct sockaddr srvr_addr;
	struct sockaddr rcvr_addr;
	int namelen;

	char buf[MAX_MSG_LEN];
	int sock;
	int bytes;

	if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket failed");
		return ERROR_CREATE_SOCKET;
	}

	srvr_addr.sa_family = AF_UNIX;
	strcpy(srvr_addr.sa_data, SOCK_NAME);

	if (bind(sock, &srvr_addr, LEN_STRUCT_SOCKADDR(srvr_addr)) == -1)
	{
		perror("bind failed");
		return ERROR_BIND_SOCKET;
	}

	while (TRUE)
	{
		bytes = recvfrom(sock, buf, sizeof(buf), 0, &rcvr_addr, &namelen);

		if (bytes < 0)
		{
			perror("recvfrom failed");
			clean_up(sock);
			return ERROR_RECVFROM_SOCKET;
		}

		printf("\nReceived message: %s\n_________", buf);
	}

	clean_up(sock);

	return 0;
}
