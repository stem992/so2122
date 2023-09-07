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

#include "../include/user.h"
#include "../include/shared_variables.h"
#include "../include/sem.h"

/* STRUCT */
Block *pnt_shm_mastro;
Users *users_us;
Nodes *nodes_us;
TransData *pnt_trans_discard;
TransData *bal_no_reg; 
TransData transaction;
BufferTransfer buff;

/* STRUCT TIME AND SIG */
struct sigaction act_signal_handler_usr;
struct timespec time_trans;
sigset_t  my_mask;

/* VARIABLES */
int mess;
int balance;
int attempts;
int flag;

/* VARIABLES FOR RANDOM */
int rand_user;
int rand_node;
int rand_quantita;
int rand_reward;

/* VARIABLES FOR COMUNICATION */
struct timespec tim;
int timerand;

/* ===================================================================== */

void execution() {
    while(1){

        sem_reserve(sem_start,5);
        delete_trans();
        sem_release(sem_start,5);

        if(_DEBUG){printf(" TENTATIVI: %d.. SO RETRY: %d\n\n", attempts, so_retry);}

        users_us[id_users].budget = getBilancio();

        if(_DEBUG && users_us[id_users].budget >= 2){
            printf(" SONO %d E IL MIO BUDGET E': %d\n", users_us[id_users].pid, users_us[id_users].budget);
        }

        if(users_us[id_users].budget >= 2) {
            rand_node = chooseRandomNode();

            if(_DEBUG) {
                printf(" -- ID NODO RANDOM %d --\n", nodes_us[rand_node].mssg);
                printf("SONO %d E VOGLIO INVIARE A: %d\n", getpid(), nodes_us[rand_node].mssg);
            }

            rand_user = chooseRandomUser();
            if(_DEBUG){printf(" -- PID UTENTE RANDOM %d --\n", rand_user);}

            rand_quantita = chooseRandomQuantita(users_us[id_users].budget);
            if(_DEBUG){printf(" -- PID QUANTITA RANDOM %d --\n", rand_quantita);}

            rand_reward = chooseRandomReward(rand_quantita);
            if(_DEBUG){printf(" -- PID REWARD RANDOM %d --\n", rand_reward);}

            transaction.receiver = rand_user;
            transaction.sender = getpid();
            transaction.reward = rand_reward;
            transaction.quantita = rand_quantita - rand_reward;

            if(clock_gettime(CLOCK_MONOTONIC, &time_trans) == -1) EXIT_ON_ERROR;
            transaction.timestamp = time_trans.tv_nsec;
            
            sem_reserve(sem_start,4);

            buff.type = 1;
            buff.transaction = transaction;

            if(_DEBUG){printf("[%d] -- INVIO MESSAGGIO NELLA CODA...\n", getpid());}
                        
            mess = msgsnd(nodes_us[rand_node].mssg, &buff, sizeof(buff.transaction), 0);

            sem_release(sem_start,4);

            if(mess != -1) {
                bal_no_reg[balance] = transaction;
                balance+=1;
                attempts = 0;
            }

            nanosleep(&tim, NULL);

        } else attempts++;
        
        if(attempts == so_retry) {
            users_alive(getpid());
        }

        if(flag==1){flag=0;} 
        else{
            srand(getpid() + time(NULL));
            timerand = (so_min_trans_gen_nsec-so_max_trans_gen_nsec)+1;
            tim.tv_sec = 0;
            tim.tv_nsec = rand() % timerand + so_min_trans_gen_nsec;
            nanosleep(&tim, NULL);
        }
    }   
}

void users_alive(pid_t pid_dead){
  int i,cont = 0;

    if(users_us[id_users].pid == pid_dead){
      sem_reserve(sem_start,1);
      users_us[id_users].budget = getBilancio();
      users_us[0].dead = users_us[0].dead - 1 ;
      if(users_us[0].dead == 0) {kill(getppid(),SIGUSR2);}
      sem_release(sem_start,1);
    }
    exit(EXIT_SUCCESS);
}

int getBilancio() { 
    int i,j,k;
    int bilancio = so_budget;
    int temp;
    int m = 0;
    
        for(i = 0; i < pnt_shm_mastro[0].index; i++) {
            for(j = 0; j < SO_BLOCK_SIZE-1; j++) {
                if(pnt_shm_mastro[i].transactions[j].receiver == getpid()) {
                    bilancio = bilancio + pnt_shm_mastro[i].transactions[j].quantita;   
                }
            }
        }

        for(k = 0; k < balance; k++) {
            bilancio = bilancio - (bal_no_reg[k].quantita + bal_no_reg[k].reward);
        }

    return bilancio;
}

void delete_trans() {
    int i,j,x,y;

    for(i=0; i < balance; i++) { 
        for(j=0; j < pnt_discard[0]; j++) {
            if (bal_no_reg[i].sender == pnt_trans_discard[j].sender && 
                bal_no_reg[i].receiver == pnt_trans_discard[j].receiver &&
                bal_no_reg[i].timestamp == pnt_trans_discard[j].timestamp) {
                for(x=i; x < balance-1; x++) { bal_no_reg[i] = bal_no_reg[i+1];}
                balance--;

                for(y=j; j < pnt_discard[0]; y++) { pnt_trans_discard[y] = pnt_trans_discard[y+1];}

                pnt_discard[0]--;
                j = pnt_discard[0];
                i = i-1;
            }

        }
    }
}

/* ===================================================================== */

int chooseRandomUser() {
    int index;
    srand(getpid() + time(NULL));
    
    index = rand() % so_num_user;
    if(index == id_users) {
        index = (index+1) % so_num_user;
    }

    return users_us[index].pid;
}

int chooseRandomQuantita(int budget) {
    int x;
    srand(getpid() + time(NULL));
    x = budget - 1; 
    return (rand() % x) + 2;
}

int chooseRandomReward(int quantita) {
    int rew;
    rew = quantita * so_reward / 100;

    if(rew < 1) { 
        rew = 1; 
        return rew;
    } else return rew;
}

int chooseRandomNode() {
    int i;
    srand(getpid() + time(NULL));

    i = rand() % so_num_node;
    return i;
}

/* ===================================================================== */

void user_handler(int signum) {
    flag=1;
    switch (signum) {
        case SIGINT:
            flag=1;
            clean_up_user();
            exit(EXIT_FAILURE);
        break;

        case SIGTERM:
            flag=1;
            clean_up_user();
            exit(EXIT_SUCCESS);
        break;

        default:
        break;
    }
}

void clean_up_user() {
    semctl(sem_start,NUM_SEM,IPC_RMID);
    shmdt(pnt_shm_mastro);
    shmdt(users_us);
    shmdt(nodes_us);
    shmdt(pnt_trans_discard); 
    exit(EXIT_SUCCESS);
}

void shared_attach() {
    pnt_shm_mastro = shmat (shm_mastro, NULL, 0); 
    EXIT_ON_ERROR;
    users_us = shmat (shm_user, NULL, 0); 
    EXIT_ON_ERROR; 
    nodes_us = shmat(shm_node, NULL, 0);
    EXIT_ON_ERROR;
    pnt_trans_discard = shmat(shm_trans_discard, NULL,0);
    EXIT_ON_ERROR;
    pnt_discard = shmat(shm_discard, NULL, 0);
    EXIT_ON_ERROR;
}

int main(int argc, char *argv[]) {

    /* SHARED MEMORY */
    shm_mastro            = atoi(argv[1]); 
    shm_user              = atoi(argv[2]); 
    shm_node              = atoi(argv[3]); 
    shm_trans_discard     = atoi(argv[4]); 
    shm_discard           = atoi(argv[5]); 

    /* SEMAPHORES */
    sem_start             = atoi(argv[6]); 

    /* ID */
    id_users              = atoi(argv[7]);

    /* VARIABLES */
    so_num_user           = atoi(argv[8]);
    so_budget             = atoi(argv[9]);
    so_reward             = atoi(argv[10]); 
    so_num_node           = atoi(argv[11]); 
    so_retry              = atoi(argv[12]);
    so_min_trans_gen_nsec = atoi(argv[13]);
    so_max_trans_gen_nsec = atoi(argv[14]);

    if(_DEBUG){printf("OK, I'LL USER FROM %d (SHARED MEMORY %d) \n", getpid(), shm_mastro);}

    shared_attach();

    bal_no_reg = (TransData *)malloc(sizeof(TransData)* SO_REGISTRY_SIZE*SO_BLOCK_SIZE);
    balance = 0;

    /* SINCRONIZZAZIONE CON IL MASTER */
    sem_reserve(sem_start,6);
    sem_zero(sem_start,6);

    act_signal_handler_usr.sa_handler = user_handler;
    act_signal_handler_usr.sa_flags = 0;

    sigemptyset(&my_mask);
    act_signal_handler_usr.sa_mask = my_mask;
    sigaction(SIGINT, &act_signal_handler_usr, NULL);
    sigaction(SIGTERM, &act_signal_handler_usr, NULL);
    
    attempts = 0;

    execution();

    return 0;
}