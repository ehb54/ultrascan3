#ifndef US_SEMAPHORE_H
#define US_SEMAPHORE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>

#define MAX_RETRIES 10

bool us_semaphore_init  ( key_t key, int nsems, int &semid );
bool us_semaphore_exists( key_t key, int nsems, int &semid );
bool us_semaphore_lock  ( int semid );
bool us_semaphore_unlock( int semid );
bool us_semaphore_free  ( int semid );

#endif
