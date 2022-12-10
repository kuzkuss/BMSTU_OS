#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <time.h>

#define CONSUMER_DELAY_TIME 7
#define PRODUCER_DELAY_TIME 4

#define N 24
#define ITERS_NUM 8
#define PRODUCER_NUM 3
#define CONSUMER_NUM 3

#define BUF_FULL  0
#define BUF_EMPTY 1
#define BIN_SEM   2

struct sembuf consumer_lock[2] = 
{
	{BUF_FULL, -1, 0},
	{BIN_SEM, -1, 0}
};

struct sembuf consumer_release[2] = 
{
	{BIN_SEM, 1, 0},
	{BUF_EMPTY, 1, 0}
};

struct sembuf producer_lock[2] = 
{
	{BUF_EMPTY, -1, 0},
	{BIN_SEM, -1, 0}
};

struct sembuf producer_release[2] = 
{
	{BIN_SEM, 1, 0},
	{BUF_FULL, 1, 0}
};

void run_producer(char **ptr, char *ch, const int sid, const int pdid)
{
	srand(time(NULL) + pdid);
	int sleep_time = rand() % PRODUCER_DELAY_TIME + 1;
	sleep(sleep_time);

	if (semop(sid, producer_lock, 2) == -1)
	{
		perror("producer lock error");
		exit(1);
	}

    (*ch)++;
	(*ptr)++;
	**ptr = *ch;

    printf("Producer #%d write: %c\n", pdid, *ch);

	if (semop(sid, producer_release, 2) == -1)
	{
		perror("procucer release error");
		exit(1);
	}
}

void create_producer(char **ptr, char *ch, const int sid, const int pdid)
{
	pid_t childpid;
	if ((childpid = fork()) == -1)
	{
		perror("fork producer error");
		exit(1);
	}
	else if (childpid == 0)
	{
		for (int i = 0; i < ITERS_NUM; i++)
			run_producer(ptr, ch, sid, pdid);

		exit(0);
	}
}

void run_consumer(char **ptr, const int sid, const int cid)
{
	srand(time(NULL) + cid);
	int sleep_time = rand() % CONSUMER_DELAY_TIME + 1;
	sleep(sleep_time);

	if (semop(sid, consumer_lock, 2) == -1)
	{
		perror("consumer lock error");
		exit(1);
	}

    char ch = **ptr;
	(*ptr)++;
	
    printf("Consumer #%d  read: %c\n", cid, ch);

	if (semop(sid, consumer_release, 2) == -1)
	{
		perror("consumer release error");
		exit(1);
	}
}

void create_consumer(char **ptr, const int sid, const int cid)
{
	pid_t childpid;
	if ((childpid = fork()) == -1)
	{
		perror("fork error");
		exit(1);
	}
	
    if (childpid == 0)
	{
		for (int i = 0; i < ITERS_NUM; i++)
			run_consumer(ptr, sid, cid);

		exit(0);
	}
}

int main(void)
{
	setbuf(stdout, NULL);
	int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int shmid = shmget(IPC_PRIVATE, 2 * sizeof(char*) + sizeof(char) + sizeof(char) * N, IPC_CREAT | perms);
    if (shmid == -1) 
	{
        perror("shmget error");
        exit(1);
    }

    char *addr = shmat(shmid, 0, 0);
    if (addr == (char*) -1) 
	{
        perror("shmat error");
        exit(1);
    }

	char **producer_ptr = (char **) addr;
    char **consumer_ptr = (char **) addr + 1;
    *producer_ptr = addr + 2 * sizeof(char*);
    *consumer_ptr = addr + 2 * sizeof(char*) + sizeof(char);

    char *ch = addr + 2 * sizeof(char*);
    *ch = 'a' - 1;

    int sem_fd = semget(IPC_PRIVATE, 3, IPC_CREAT | perms);
	if (sem_fd == -1)
	{
		perror("semget error");
		exit(1);
	}
	
	if (semctl(sem_fd, BIN_SEM, SETVAL, 1) == -1)
	{
		perror("semctl error");
		exit(1);
	}

    if (semctl(sem_fd, BUF_FULL, SETVAL, 0) == -1)
	{
		perror("semctl error");
		exit(1);
	}

    if (semctl(sem_fd, BUF_EMPTY, SETVAL, N) == -1)
	{
		perror("semctl error");
		exit(1);
	}

    for (int i = 0; i < PRODUCER_NUM; i++)
		create_producer(producer_ptr, ch, sem_fd, i + 1);

	for (int i = 0; i < CONSUMER_NUM; i++)
		create_consumer(consumer_ptr, sem_fd, i + 1);

    for (size_t i = 0; i < PRODUCER_NUM + CONSUMER_NUM; i++)
    {
        int status;
        if (wait(&status) == -1) 
		{
            perror("wait error");
            exit(1);
        }

        if (!WIFEXITED(status))
            printf("one of children terminated abnormally\n");
    }

	if (shmdt(addr) == -1)
    {
        perror("shmdt error");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl with command IPC_RMID error");
        exit(1);
    }
    if (semctl(sem_fd, 0, IPC_RMID) == -1)
    {
        perror("semctl with command IPC_RMID error");
        exit(1);
    }

	return 0;
}
