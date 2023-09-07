#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../include/shared_variables.h"
#include "../include/sem.h"

/* CREATE A SEMAPHORE */
int create_semaphore(int sem_id, int sem_num) {   
    union semun sem_val;
    sem_val.val = 1;
    
    return semctl(sem_id, sem_num, SETVAL, sem_val);
}

/* TRY TO ACCESS THE RESOURCE */
int sem_reserve(int sem_id, int sem_num) {
	struct sembuf sops;
	
	sops.sem_num = sem_num;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	return semop(sem_id, &sops, 1);
}

/* RELEASE THE RESOURCE */
int sem_release(int sem_id, int sem_num) {
	struct sembuf sops;
  
	sops.sem_num = sem_num;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	
	return semop(sem_id, &sops, 1);
}

/* SET THE SEMAPHORE TO 0 */
int sem_zero(int sem_id, int sem_num) { 
	struct sembuf sops;

    sops.sem_num = sem_num;
	sops.sem_op = 0;
	sops.sem_flg = 0;

    return semop(sem_id, &sops, 1);
}