#include "us_semaphore.h"

union semun {
  int val;
  struct semid_ds *buf;
  ushort *array;
};

/*
** initsem() -- more-than-inspired by W. Richard Stevens' UNIX Network
** Programming 2nd edition, volume 2, lockvsem.c, page 295.
*/

bool us_semaphore_exists(key_t key, int nsems, int &semid) {
  semid = semget(key, nsems, 0);
  if (semid < 0) {
    return false;
  }
  return true;
}

bool us_semaphore_init(key_t key, int nsems, int &semid) {
  int i;
  union semun arg;
  struct semid_ds buf;
  struct sembuf sb;

  semid = semget(key, nsems, IPC_CREAT | IPC_EXCL | 0666);

  if (semid >= 0) { /* we got it first */
    sb.sem_op = 1;
    sb.sem_flg = 0;
    arg.val = 1;

    printf("got semaphore\n");
    // printf("press return\n"); getchar();

    for (sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num++) {
      /* do a semop() to "free" the semaphores. */
      /* this sets the sem_otime field, as needed below. */
      if (semop(semid, &sb, 1) == -1) {
        int e = errno;
        semctl(semid, 0, IPC_RMID); /* clean up */
        errno = e;
        return false;
      }
    }
  } else {
    if (errno == EEXIST) { /* someone else got it first */
      int ready = 0;

      semid = semget(key, nsems, 0); /* get the id */
      if (semid < 0) {
        return false; /* error, check errno */
      }

      /* wait for other process to initialize the semaphore: */
      arg.buf = &buf;
      for (i = 0; i < MAX_RETRIES && !ready; i++) {
        semctl(semid, nsems - 1, IPC_STAT, arg);
        if (arg.buf->sem_otime != 0) {
          ready = 1;
        } else {
          sleep(1);
        }
      }
      if (!ready) {
        errno = ETIME;
        return false;
      }
    } else {
      return false;
    }
  }

  return true;
}

bool us_semaphore_lock(int semid) {
  struct sembuf sb;

  sb.sem_num = 0;
  sb.sem_op = -1; /* set to allocate resource */
  sb.sem_flg = 0;

  printf("Trying to lock...%d\n", semid);

  if (semop(semid, &sb, 1) == -1) {
    return false;
  }

  printf("Locked...%d\n", semid);

  return true;
}

bool us_semaphore_unlock(int semid) {
  struct sembuf sb;

  sb.sem_num = 0;
  sb.sem_op = 1; /* set to free resource */
  sb.sem_flg = 0;

  printf("Trying to unlock...%d\n", semid);

  if (semop(semid, &sb, 1) == -1) {
    return false;
  }

  printf("Unlocked...%d\n", semid);

  return true;
}

bool us_semaphore_free(int semid) {
  union semun arg;

  printf("Trying to delete...%d\n", semid);
  /* remove it: */
  if (semctl(semid, 0, IPC_RMID, arg) == -1) {
    return false;
  }

  printf("Deleted...%d\n", semid);

  return true;
}

#if defined(TESTING)
int main(int argc, char **argv) {
  if (argc < 3) {
    printf(
        "usage: %s command number\n"
        "commands:\n"
        "exists   check to see if the semaphore exists\n"
        "lock     lock the semaphore\n"
        "unlock   unlock the semaphore\n"
        "delete   delete the semaphore\n",
        argv[0]);
    exit(-1);
  }

  int semnumber = atoi(argv[2]);
  printf("semaphore number is %d\n", semnumber);

  int semid;
  key_t key = (key_t)(0x54000001 + semnumber);

  if (!strncmp(argv[1], "exists", 6)) {
    printf("semaphore 0x%x %d %s\n", key, semid,
           us_semaphore_exists(key, 1, semid) ? "exists" : "does not exist");
    exit(0);
  }

  struct sembuf sb;

  sb.sem_num = 0;
  sb.sem_op = -1; /* set to allocate resource */
  sb.sem_flg = SEM_UNDO;

  if (!us_semaphore_init(key, 1, semid)) {
    perror("us_semaphore_init");
    exit(-2);
  }
  printf("semaphore key 0x%x\n", key);
  printf("semaphore id  %d\n", semid);

  if (!strncmp(argv[1], "lock", 4)) {
    if (!us_semaphore_lock(semid)) {
      perror("us_semaphore_lock");
      exit(-3);
    }
    exit(0);
  }

  if (!strncmp(argv[1], "unlock", 6)) {
    if (!us_semaphore_unlock(semid) == -1) {
      perror("us_semaphore_unlock");
      exit(-4);
    }
    exit(0);
  }

  if (!strncmp(argv[1], "delete", 6)) {
    if (!us_semaphore_free(semid) == -1) {
      perror("us_semaphore_unlock");
      exit(-5);
    }
    exit(0);
  }

  printf("%s: unknown command: %s\n", argv[0], argv[1]);

  return 0;
}
#endif
