#ifndef US_SEMAPHORE_H
#define US_SEMAPHORE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_RETRIES 10

bool us_semaphore_init(key_t key, int nsems, int &semid);
bool us_semaphore_exists(key_t key, int nsems, int &semid);
bool us_semaphore_lock(int semid);
bool us_semaphore_unlock(int semid);
bool us_semaphore_free(int semid);

#endif
