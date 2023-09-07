#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#include "../include/node.h"
#include "../include/shared_variables.h"
#include "../include/sem.h"

/* STRUCT */
Block *pnt_shm_mastro;
Nodes *nodes_node;
TransData *trans_pool;
TransData *pnt_trans_discard;
TransData transaction;
BufferTransfer buff;

/* STRUCT TIME AND SIG */
struct sigaction act_signal_handler_node;
struct timespec tim, tim2;
sigset_t  my_mask;

/* VARIABLES */
int status;
int timerand;
int mess;
int flag = 1;

/* ===================================================================== */

void execution() {

    while(1 && flag == 1){
        mess = msgrcv(msg_id, &buff, sizeof(buff.transaction), 0, IPC_NOWAIT);
        if (mess != -1) {
            if(_DEBUG){printf("\nRICEVUTO MESSAGGIO DA: %d\n",buff.transaction.sender);}
                
                sem_reserve(sem_start,0);
                if(pnt_num_iter[0] < so_tp_size){
                    trans_pool[pnt_num_iter[0]] = buff.transaction;
                    pnt_num_iter[0]++;
                } else {discard();}

                nodes_node[id_nodes].trans_rim = pnt_num_iter[0];
                sem_release(sem_start,0);
        
            
            status = controllo_blocco(buff.transaction);
            switch(status){
                case NOTFULL:
                    sem_reserve(sem_start,0);             
                    if(pnt_num_iter[0] >= SO_BLOCK_SIZE -1) {
                        sem_reserve(sem_start,2);
                        process_block();
                        sem_release(sem_start,2);

                        timerand = (so_min_trans_proc_nsec-so_max_trans_proc_nsec)+1;
                        tim.tv_sec = 0;
                        tim.tv_nsec = rand() % timerand + so_min_trans_gen_nsec;
                                
                        if(nanosleep(&tim, &tim2)<0) { EXIT_ON_ERROR; } 
                    }
                    sem_release(sem_start,0);

                break;

                case FULL:
                    flag = 0;
                    kill(getppid(),SIGUSR1);
                    clean_up_node();
                break;

                default:
                break;
            }
        }
    }
        
}
  
  void discard() {
        sem_reserve(sem_start,5);
        pnt_trans_discard[pnt_discard[0]] = buff.transaction;
        pnt_discard[0]++;
        sem_release(sem_start,5);
  }

  int controllo_blocco(TransData tran) {
    int registro;
    int i,j;

    if(pnt_shm_mastro[0].index < SO_REGISTRY_SIZE) {
        registro = NOTFULL;
    } else {registro = FULL;}

    return registro;
}

void process_block() {
    int i;

    for(i=0; i < SO_BLOCK_SIZE-1; i++) {
        pnt_shm_mastro[pnt_shm_mastro[0].index].transactions[i] = trans_pool[i];

    }

    if(_DEBUG) printf(" NUM ITER IN PROCESS BLOCK: %d\n",*(pnt_num_iter));

    pnt_shm_mastro[pnt_shm_mastro[0].index].transactions[SO_BLOCK_SIZE-1] = trans_reward(); 
    pnt_shm_mastro[pnt_shm_mastro[0].index].index = pnt_shm_mastro[0].index;
    pnt_shm_mastro[0].index = pnt_shm_mastro[0].index+ 1;
}

TransData trans_reward() {
    TransData transaction_reward;
    int k; int quantita = 0;
    struct timespec time_node;

    for(k=0; k < SO_BLOCK_SIZE-1; k++){
        quantita += trans_pool[k].reward;
        memset(&trans_pool[k], 0, sizeof(TransData)); 
    }
    
    *pnt_num_iter = *pnt_num_iter - (SO_BLOCK_SIZE-1);
    
    if(clock_gettime(CLOCK_REALTIME, &time_node) == -1) EXIT_ON_ERROR;

    transaction_reward.timestamp = time_node.tv_nsec + time_node.tv_sec;
    transaction_reward.sender = SENDER;
    transaction_reward.receiver = getpid();
    transaction_reward.quantita = quantita;
    transaction_reward.reward = 0;

    return transaction_reward;
}

/* ===================================================================== */

void node_handler(int signum) {
    switch (signum) {
        case SIGINT: 
            clean_up_node();
            exit(EXIT_FAILURE);
        break;

        case SIGTERM:
            flag = 0;
            clean_up_node();
            exit(EXIT_SUCCESS);
        break;

        default:
        break;
    }
}

void clean_up_node() {
    semctl(sem_start,NUM_SEM,IPC_RMID);
    shmdt(pnt_shm_mastro);
    shmdt(nodes_node);
    shmdt(pnt_trans_discard); 
    shmdt(pnt_num_iter);
    msgctl(msg_id, IPC_RMID, NULL); 
}

void shared_attach() {
    pnt_shm_mastro = shmat (shm_mastro, NULL, 0); 
    EXIT_ON_ERROR;
    nodes_node = shmat(shm_node, NULL, 0);
    EXIT_ON_ERROR;
    trans_pool = shmat(trans, NULL, 0);
    EXIT_ON_ERROR;
    pnt_trans_discard = shmat(shm_trans_discard, NULL,0);
    EXIT_ON_ERROR;
    pnt_discard = shmat(shm_discard, NULL,0);
    EXIT_ON_ERROR;
    pnt_num_iter = shmat(shm_num_iter, NULL, 0);
    EXIT_ON_ERROR;
}

int main(int argc, char *argv[]) {

    /* SHARED MEMORY */
    shm_mastro              = atoi(argv[1]); 
    shm_node                = atoi(argv[2]); 
    shm_discard             = atoi(argv[3]); 
    shm_trans_discard       = atoi(argv[4]); 

    /* SEMAPHORES */
    sem_start               = atoi(argv[5]); 

    /* ID */
    id_nodes                = atoi(argv[6]);

    /* VARIABLES */
    so_num_node             = atoi(argv[7]); 
    so_tp_size              = atoi(argv[8]);
    so_min_trans_proc_nsec  = atoi(argv[9]); 
    so_max_trans_proc_nsec  = atoi(argv[10]);
    
    if(_DEBUG){printf("I'M %d AND I'M A NODE\n", getpid());}

    shm_num_iter = shmget(IPC_PRIVATE, sizeof(int), 0666 | IPC_CREAT);
    trans = shmget(IPC_PRIVATE, sizeof(TransData)*so_tp_size, 0666 | IPC_CREAT);

    shared_attach();
    pnt_num_iter[0] = 0;

    msg_id = msgget(IPC_PRIVATE, IPC_CREAT| IPC_EXCL | 0666);
    if (_DEBUG) printf("\n CODA DI MESSAGGI CREATA: %d\n", msg_id);
    nodes_node[id_nodes].mssg = msg_id;
    
    /* SINCRONIZZAZIONE CON IL MASTER */
    sem_reserve(sem_start,6);
    sem_zero(sem_start,6);

    act_signal_handler_node.sa_handler = node_handler;
    act_signal_handler_node.sa_flags = 0;

    sigemptyset(&my_mask);
    act_signal_handler_node.sa_mask = my_mask;
    sigaction(SIGINT, &act_signal_handler_node, NULL);
    sigaction(SIGTERM, &act_signal_handler_node, NULL);

    execution();

    return 0;   
}
