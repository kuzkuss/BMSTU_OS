#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>


int flag = 0;


void check_status(int status)
{
    if (WIFEXITED(status))
    {
        printf("Child exited correctly with code %d.\n", WEXITSTATUS(status));

        return;
    }
    else if (WIFSIGNALED(status))
    {
        printf("Child received non-interceptable signal %d.\n", WTERMSIG(status));

        return;
    }
    else if (WIFSTOPPED(status))
    {
        printf("Child received signal %d.\n", WSTOPSIG(status));

        return;
    }
}


void catch_ctrlz(int signal)
{
    flag = 1;
    printf("\nReceived signal = %d.\n", signal);
}


int main(void)
{
    if (signal(SIGTSTP, catch_ctrlz) == SIG_ERR) {
        perror("\nCan't signal.\n");
        exit(EXIT_FAILURE);
    }
    sleep(4);

    int fd[2];

    pid_t child_pid[2];
    pid_t child;
    int status;

    char msgs[50] = {0};

    if (pipe(fd) == -1)
    {
        perror("\nCan't pipe.\n");
        exit(EXIT_FAILURE);
    }


    if ((child_pid[0] = fork()) == -1)
    {
        perror("\nCan't fork child 1.\n");
        exit(EXIT_FAILURE);
    }
    else if (child_pid[0] == 0)
    {
        printf("\nChild 1: PID = %d, PPID = %d, GPID = %d.\n", getpid(), getppid(), getpgrp());

        if (flag == 1)
        {
            close(fd[0]);
            write(fd[1], "qwert\n", strlen("qwert\n"));
        }

        return EXIT_SUCCESS;
    }


    if ((child_pid[1] = fork()) == -1)
    {
        perror("\nCan't fork child 2.\n");
        exit(EXIT_FAILURE);
    }
    else if (child_pid[1] == 0)
    {
        printf("Child 2: PID = %d, PPID = %d, GPID = %d.\n", getpid(), getppid(), getpgrp());

        if (flag == 1)
        {
            close(fd[0]);
            write(fd[1], "qwertqwertqwertqwertqwert\n", strlen("qwertqwertqwertqwertqwert\n"));
        }

        return EXIT_SUCCESS;
    }

    else
    {
        for (int i = 0; i < 2; i++)
        {
            child = waitpid(child_pid[i], &status, 0);
            if (child == -1)
            {
                perror("\nWaitpid error.\n");
                exit(EXIT_FAILURE);
            }
            printf("\n\nChild has fihished: PID = %d.\n", child);
            check_status(status);
        }
    }

    close(fd[1]);
    read(fd[0], msgs, 50);
    printf("\nChilds wrote :\n%s\n", msgs);

    return EXIT_SUCCESS;
}