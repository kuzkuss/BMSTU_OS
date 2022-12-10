#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>

#define N 10

#define N_READERS 4
#define N_WRITERS 3

#define ACTIVE_READERS 0
#define CAN_WRITE 1
#define CAN_READ 2
#define WAITING_WRITERS 3
#define BIN_SEM 4

struct sembuf SEM_START_READ[] =
{
    {CAN_WRITE, 0, 0},
    {CAN_READ, 1, 0},
    {WAITING_WRITERS, 0, 0},
    {ACTIVE_READERS, 1, 0},
    {CAN_READ, -1, 0},
};

struct sembuf SEM_STOP_READ[] =
{
    {ACTIVE_READERS, -1, 0},
};

struct sembuf SEM_START_WRITE[] =
{
    {WAITING_WRITERS, 1, 0},
    {ACTIVE_READERS, 0, 0},
    {CAN_WRITE, 0, 0},
    {CAN_WRITE, 1, 0},
    {WAITING_WRITERS, -1, 0},
    {BIN_SEM, -1, 0},
};

struct sembuf SEM_STOP_WRITE[] =
{
    {CAN_WRITE, -1, 0},
    {BIN_SEM, 1, 0},
};

int start_read(int s_id)
{
    return semop(s_id, SEM_START_READ, 5) != -1;
}

int stop_read(int s_id)
{
    return semop(s_id, SEM_STOP_READ, 1) != -1;
}

int start_write(int s_id)
{
    return semop(s_id, SEM_START_WRITE, 5) != -1;
}

int stop_write(int s_id)
{
    return semop(s_id, SEM_STOP_WRITE, 1) != -1;
}

int reader_run(int *const shared_mem, const int s_id, const int reader_id)
{
    int sleep_time;
    if (!shared_mem)
    {
        return 1;
    }
    srand(time(NULL) + reader_id);
    for (size_t i = 0; i < N; i++)
    {
        sleep_time = rand() % 7 + 1;
        sleep(sleep_time);
        if (!start_read(s_id))
        {
            perror("start_read error");
            exit(1);
        }
        int val = *shared_mem;
        printf("Reader #%d  read: %2d\n", reader_id, val);
        if (!stop_read(s_id))
        {
            perror("stop_read error");
            exit(1);
        }
    }
    return 0;
}

int writer_run(int *const shared_mem, const int s_id, const int writer_id)
{
    int sleep_time;
    if (!shared_mem)
    {
        return 1;
    }
    srand(time(NULL) + writer_id + N_READERS);
    for (size_t i = 0; i < N; i++)
    {
        sleep_time = rand() % 3 + 1;
        sleep(sleep_time);
        if (!start_write(s_id))
        {
            perror("start_write error");
            exit(1);
        }
        int val = ++(*shared_mem);
        printf("Writer #%d write: %2d\n", writer_id, val);
        if (!stop_write(s_id))
        {
            perror("stop_write error");
            exit(1);
        }
    }
    return 0;
}

int main()
{
    setbuf(stdout, NULL);
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int fd = shmget(IPC_PRIVATE, sizeof(int), perms | IPC_CREAT);
    if (fd == -1)
    {
        perror("shmget failed");
        exit(1);
    }
    int *shared_mem = shmat(fd, 0, 0);
    if (shared_mem == (void *)-1)
    {
        perror("shmat error");
        exit(1);
    }
    int s_id = semget(IPC_PRIVATE, 5, perms | IPC_CREAT);
    if (s_id == -1)
    {
        perror("semget error");
        exit(1);
    }
    if (semctl(s_id, ACTIVE_READERS, SETVAL, 0) == -1 ||
        semctl(s_id, CAN_WRITE, SETVAL, 0) == -1 ||
        semctl(s_id, WAITING_WRITERS, SETVAL, 0) == -1 ||
        semctl(s_id, CAN_READ, SETVAL, 0) == -1)
    {
        perror("sem init error");
        exit(1);
    }
    if (semctl(s_id, BIN_SEM, SETVAL, 1) == -1)
	{
		perror("semctl error");
		exit(1);
	}
    for (size_t i = 0; i < N_WRITERS; ++i)
    {
        int child_pid = fork();
        if (child_pid == -1)
        {
            perror("fork writer error");
            exit(1);
        }
        else if (child_pid  == 0)
        {
            writer_run(shared_mem, s_id, i);
            return 0;
        }
    }
    for (size_t i = 0; i < N_READERS; i++)
    {
        int child_pid = fork();
        if (child_pid == -1)
        {
            perror("fork reader error");
            exit(1);
        }
        else if (child_pid  == 0)
        {
            reader_run(shared_mem, s_id, i);
            return 0;
        }
    }
    for (size_t i = 0; i < N_WRITERS + N_READERS; i++)
    {
        int ch_status;
        int child_pid = wait(&ch_status);

        if (child_pid == -1)
        {
            perror("wait error");
            exit(1);
        }
        if (!WIFEXITED(ch_status))
        {
            fprintf(stderr, "Child process %d terminated abnormally", child_pid);
        }
    }
    if (shmdt(shared_mem) == -1)
    {
        perror("shmdt error");
        exit(1);
    }
    if (shmctl(fd, IPC_RMID, NULL) == -1)
    {
        perror("shmctl with command IPC_RMID error");
        exit(1);
    }
    if (semctl(s_id, 0, IPC_RMID) == -1)
    {
        perror("semctl with command IPC_RMID error");
        exit(1);
    }
    return 0;
}
