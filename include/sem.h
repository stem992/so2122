#ifndef SEM_H
#define SEM_H

/* UNION */
union semun {
	int              val;    /* VALUE FOR SETVAL */
	struct semid_ds *buf;    /* BUFFER FOR IPC_STAT, IPC_SET */
	unsigned short  *array;  /* ARRAY FOR GETALL, SETALL */
	struct seminfo  *__buf;  /* BUFFER FOR IPC_INFO */
};

/* DICHIARAZIONE DELLE FUNZIONI */
int create_semaphore(int sem_id, int sem_num);
int sem_reserve(int sem_id, int sem_num);
int sem_release(int sem_id, int sem_num);
int sem_zero(int sem_id, int sem_num);

#endif