#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


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


int main(void)
{
    pid_t child_pid[2];
    pid_t child;
    int status;

    for (int i = 0; i < 2; i++) {
        if ((child_pid[i] = fork()) == -1)
        {
            perror("\nFork error.\n");
            exit(EXIT_FAILURE);
        }
        else if (child_pid[i] == 0)
        {
            printf("\nChild: PID = %d, PPID = %d, GPID = %d.\n", getpid(), getppid(), getpgrp());

            return EXIT_SUCCESS;
        }
        else
        {
            child = wait(&status);
            if (child == -1)
            {
                perror("\nWait error.\n");
                exit(EXIT_FAILURE);
            }
            printf("\nChild has fihished: PID = %d.\n", child);
            check_status(status);

            printf("\nParent: PID = %d, GPID = %d, child PID = %d.\n", getpid(), getpgrp(), child_pid[i]);
        }
    }

    return EXIT_SUCCESS;
}

