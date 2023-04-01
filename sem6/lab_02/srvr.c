#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define DEFAULT_PORT    8080
#define MAX_CONN        16
#define MAX_EVENTS      32
#define BUF_SIZE        16
#define MAX_LINE        256

int fd;

static void set_sockaddr(struct sockaddr_in *addr)
{
	bzero((char *)addr, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = INADDR_ANY;
	addr->sin_port = htons(DEFAULT_PORT);
}

static void epoll_ctl_add(int epfd, int fd, uint32_t events)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) 
	{
		perror("epoll_ctl()\n");
		exit(1);
	}
}

static int setnonblocking(int sockfd)
{
	if (fcntl(sockfd, F_SETFD, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1) 
	{
		return -1;
	}
	return 0;
}

int main(void)
{
	int epfd, nfds;
	int listen_sock;
	int conn_sock;
	int socklen;
	char buf[BUF_SIZE];
	struct sockaddr_in srvr_addr;
	struct sockaddr_in clnt_addr;
	struct epoll_event events[MAX_EVENTS];

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	set_sockaddr(&srvr_addr);
	bind(listen_sock, (struct sockaddr *)&srvr_addr, sizeof(srvr_addr));

	setnonblocking(listen_sock);
	listen(listen_sock, MAX_CONN);

	epfd = epoll_create(1);
	epoll_ctl_add(epfd, listen_sock, EPOLLIN | EPOLLOUT | EPOLLET);

	socklen = sizeof(clnt_addr);
	while (1) 
    {
		nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (nfds == -1) 
		{
            perror("epoll_wait");
            exit(1);
        }
		for (int i = 0; i < nfds; i++) 
        {
			if (events[i].data.fd == listen_sock) 
            {
				conn_sock = accept(listen_sock, (struct sockaddr *)&clnt_addr, &socklen);
				if (conn_sock == -1) 
				{
                    perror("accept");
                    exit(1);
                }
				inet_ntop(AF_INET, (char *)&(clnt_addr.sin_addr), buf, sizeof(clnt_addr));
				printf("Client: %s:%d\n", buf, ntohs(clnt_addr.sin_port));

				setnonblocking(conn_sock);
				epoll_ctl_add(epfd, conn_sock, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP);
			} 
            else if (events[i].events & EPOLLIN) 
            {
				while (1)
				{
				    bzero(buf, sizeof(buf));
				    if (read(events[i].data.fd, buf, sizeof(buf)) <= 0) 
				    	break;
                    else 
                    {
					    printf("Client pid: %s", buf);
					    write(events[i].data.fd, buf, strlen(buf));
				    }
				}
			}
            else 
            {
				printf("Error\n");
			}
			
			if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) 
            {
				printf("Connection closed\n\n");
				epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
				close(events[i].data.fd);
				continue;
			}
		}
	}
}
