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

    if ((child_pid[0] = fork()) == -1)
    {
        perror("\nCan't fork child 1.\n");
        exit(EXIT_FAILURE);
    }
    else if (child_pid[0] == 0)
    {
        printf("\nChild 1 START: PID = %d, PPID = %d, GPID = %d.\n", getpid(), getppid(), getpgrp());

        if (execl("./count_digits.out", "./count_digits.out", NULL) == -1)
        {
            perror("\nERROR: child 1 can not execute exec().\n");

            exit(EXIT_FAILURE);
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
        printf("\n\n\nChild 2 START: PID = %d, PPID = %d, GPID = %d.\n", getpid(), getppid(), getpgrp());

        if (execl("./process_array.out", "./process_array.out", NULL) == -1)
        {
            perror("\nERROR: child 2 can not execute exec().\n");

            exit(EXIT_FAILURE);
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

            printf("\nParent: PID = %d, GPID = %d, child PID = %d.\n", getpid(), getpgrp(), child_pid[i]);
        }
    }

    return EXIT_SUCCESS;
}
