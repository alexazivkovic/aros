#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#define BLOCK_SIZE 256
#define MAX_MESSAGES 10
#define FILE_NAME "file"
#define ERROR (-1)
#define SEM_READ_NAME "/sem_read2"
#define SEM_WRITE_NAME "/sem_write2"
#define SEM_MUTEX_NAME "/sem_mutex2"

sem_t *sem_read;
sem_t *sem_write;
sem_t *sem_mutex;
char *block;

static int get_shared_block(char *filename, int size) {
    key_t key = ftok(filename, 0);
    if (key == ERROR) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    return shmget(key, size, 0644 | IPC_CREAT);
}

char * attach_memory_block(char *filename, int size) {
    int shared_block_id = get_shared_block(filename, size);
    if (shared_block_id == ERROR) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    char *result;
    result = shmat(shared_block_id, NULL, 0);
    if (result == (char *)(-1)) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    return result;
}

bool detach_memory_block(char *block) {
    return (shmdt(block) != ERROR);
}

sem_t* initialize_semaphore(char* name, int val) {
    sem_t* sem = sem_open(name, O_CREAT | O_EXCL, 0666, val);
    if (sem == (sem_t*)ERROR) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    return sem;
}

bool destroy_semaphore(sem_t *sem, char* name) {
    return (sem_close(sem) != ERROR && sem_unlink(name) != ERROR);
}

int main(int argc, char* argv[]) {
    block = attach_memory_block(FILE_NAME, BLOCK_SIZE * MAX_MESSAGES);
    if (block == NULL) {
        printf("[-] Error! Unable to get shared memory block!");
        return -1;
    } else {
        printf("[+] Shared memory block retrieved successfully!\n");
    }

    sem_read = initialize_semaphore(SEM_READ_NAME, 0);
    sem_write = initialize_semaphore(SEM_WRITE_NAME,10);
    sem_mutex = initialize_semaphore(SEM_MUTEX_NAME, 1);
    if (sem_read != SEM_FAILED && sem_write != SEM_FAILED && sem_mutex != SEM_FAILED) {
        printf("[+] Semaphores initialized successfully!\n");
    } else {
        printf("[-] Error! Unable to initialize semaphores!\n");;
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else {
        for (int i = 0; i < 15; i++) {
            if (pid == 0) {
                sem_wait(sem_write);
                sem_wait(sem_mutex);
                int n;
                sem_getvalue(sem_read, &n);
                strcpy(block + n * BLOCK_SIZE, "bobi");
                printf("[+] Process %d wrote bobi to shared memory on %d. place in array\n", getpid(), n);
                sem_post(sem_mutex);
                sem_post(sem_read);
                //sleep(1);
            } else {
                sem_wait(sem_read);
                sem_wait(sem_mutex);
                int n;
                sem_getvalue(sem_read, &n);
                char message[BLOCK_SIZE];
                strcpy(message, block + n * BLOCK_SIZE);
                printf("[+] Process %d read %s from shared memory on %d. place in array\n", getpid(), message, n);
                sem_post(sem_mutex);
                sem_post(sem_write);
                //sleep(1);
            }
        }
    }

    if(pid == 0){
        exit(0);
    }else{
        wait(NULL);
    }

    if (destroy_semaphore(sem_read, SEM_READ_NAME) &&
        destroy_semaphore(sem_write, SEM_WRITE_NAME) &&
        destroy_semaphore(sem_mutex, SEM_MUTEX_NAME) &&
        detach_memory_block(block)) {
        printf("[+] Shared memory and semaphores destroyed successfully!\n");
    } else {
        printf("[-] Error! Unable to destroy shared memory and semaphores!\n");
        return -1;
    }

    return 0;
}
