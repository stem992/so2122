#ifndef SHARED_VARIABLES_H
#define SHARED_VARIABLES_H

#define SO_BLOCK_SIZE 10 
#define SO_REGISTRY_SIZE 1000
#define MSG_CODES_KEY 0x125432
#define BUFFER_SIZE 4096
#define SENDER -1
#define FULL 1
#define NOTFULL 2
#define MAX_USERS 10
#define MAX_NODES 5
#define NUM_SEM 7

#ifndef _DEBUG 
#define _DEBUG 0
#endif

/* =========================== EXIT_ON_ERROR =============================== */

#define EXIT_ON_ERROR if (errno) {dprintf(STDERR_FILENO, \
			           "%s:%d: PID=%5d: Error %d (%s)\n", \
			           __FILE__,			\
			           __LINE__,			\
			           getpid(),			\
				       errno,			    \
				       strerror(errno));}

/* ======================= STRUTTURA DI CONFIGURAZIONE ===================== */

struct configuration {
    int SO_USERS_NUM;
    int SO_NODES_NUM;
    int SO_BUDGET_INIT;
    int SO_REWARD;
    int SO_MIN_TRANS_GEN_NSEC;
    int SO_MAX_TRANS_GEN_NSEC;
    int SO_RETRY;
    int SO_TP_SIZE;
    int SO_MIN_TRANS_PROC_NSEC;
    int SO_MAX_TRANS_PROC_NSEC;
    long SO_SIM_SEC;   
};

/* =========================== STRUTTURE ================================== */

typedef struct TransactionData {
    long timestamp;
    int sender;
    int receiver;
    int quantita;
    int reward;
} TransData;

typedef struct Blocco {
    int index; 
    TransData transactions[SO_BLOCK_SIZE];
} Block;

typedef struct buffer_transfer {
    long type;
    TransData transaction;
} BufferTransfer;

typedef struct info_user {
    pid_t pid;
    int budget;
    unsigned int dead;
} Users;

typedef struct info_node {
    pid_t pid;
    int mssg;
    int trans_rim;
} Nodes;


/* =========================== VARIABILI ================================ */

/* SHARED MEMORY */
int shm_mastro;
int shm_user;
int shm_node;
int shm_trans_discard;
int shm_discard;
int shm_num_iter;
int node;
int trans;

/* SEMAFORI */
int sem_start;

/* CODE DI MESSAGGI */
int msg_id;

/* ID */
int id_users;
int id_nodes;

/* ALTRE VARIABILI */
int so_tp_size;
int so_num_user;
int so_budget;
int so_reward;
int so_num_node;
int so_retry;
int so_min_trans_gen_nsec;
int so_max_trans_gen_nsec;
int so_min_trans_proc_nsec;
int so_max_trans_proc_nsec;

/* PUNTATORI */
int *pnt_discard;
int *pnt_num_iter;

#endif