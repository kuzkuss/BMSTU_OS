#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


int main(void)
{
    pid_t child_pid[2];

    for (int i = 0; i < 2; i++) {
        if ((child_pid[i] = fork()) == -1)
        {
            perror("\nFork error.\n");
            exit(EXIT_FAILURE);
        }
        else if (child_pid[i] == 0)
        {
            printf("\nChild: PID = %d, PPID = %d, GPID = %d.\n", getpid(), getppid(), getpgrp());

            sleep(2);

            printf("\nChild: PID = %d, PPID = %d, GPID = %d.\n", getpid(), getppid(), getpgrp());

            return EXIT_SUCCESS;
        }
        else
        {
            printf("\nParent: PID = %d, GPID = %d, child PID = %d.\n", getpid(), getpgrp(), child_pid[i]);
        }
    }

    return EXIT_SUCCESS;
}
