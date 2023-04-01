#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#define CHILD 4

int main()
{
    int sockets[2];
    char buf[32];
    int pid;

    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sockets) == -1)
    {
        perror("socketpair() failed");
        return EXIT_FAILURE;
    }


    for (int i = 0; i < CHILD; i++)
    {
        if ((pid = fork()) == -1)
        {
            perror("fork error");
            exit(1);
        }
        else if (pid == 0)
        {   
            char message[16];
            sprintf(message, "%d", getpid());
            printf("Child sent: %s\n", message);
            write(sockets[0], message, sizeof(message));
            sleep(1);
            read(sockets[0], buf, sizeof(buf));
            printf("Child recieved: %s\n", buf);

            return EXIT_SUCCESS;
        }
    }

    for (int i = 0; i < CHILD; i++)
    {
        char message[32];
      
        read(sockets[1], buf, sizeof(buf));
        sprintf(message, "Child - %s, parent - %d", buf, getpid());
        printf("Parent recieved: %s\n", buf);
        write(sockets[1], message, sizeof(message));
        printf("Parent sent: %s\n", message);
    }

    sleep(2);

    return EXIT_SUCCESS;
}