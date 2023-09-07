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

#include "../include/shared_variables.h"
#include "../include/master.h"
#include "../include/sem.h"

/* STRUCT */
Block *pnt_shm_mastro;
Users *users;
Nodes *nodes;

/* STRUCT TIME AND SIG */
struct sigaction act_signal_handler;
struct configuration config;
struct timespec request;
sigset_t  my_mask;

/* VARIABLES */
int sim_count = 1;
int print_budget = 0;

/* ===================================================================== */

void spawn_users(int users_count, int users_rec) { 
    pid_t child_pid;
    int status, i;

    char *user_argv[16] = {"User"};

    /* SHARED MEMORY */
    char shm_mastro_string[(3 * sizeof(shm_mastro)) + 1];
    char shm_user_string[(3 * sizeof(shm_user)) + 1];
    char shm_node_string[(3 * sizeof(shm_node)) + 1];
    char shm_trans_discard_string[(3 * sizeof(shm_trans_discard)) + 1]; 
    char shm_discard_string[(3 * sizeof(shm_discard)) + 1]; 

    /* SEMAPHORES */
    char sem_start_string[(3 * sizeof(sem_start)) + 1];

    /* ID */
    char id_users[(3* sizeof(int)) + 1];

    /* VARIABLES */
    char so_num_user_string[(3 * sizeof(config.SO_USERS_NUM)) + 1];
    char so_budget_string[(3 * sizeof(config.SO_BUDGET_INIT)) + 1];
    char so_reward_string[(3 * sizeof(config.SO_REWARD)) + 1];
    char so_num_node_string[(3 * sizeof(config.SO_NODES_NUM)) + 1];
    char so_retry_string[(3 * sizeof(config.SO_RETRY)) + 1];
    char so_min_trans_gen_nsec_string[(3 * sizeof(config.SO_MIN_TRANS_GEN_NSEC)) + 1];
    char so_max_trans_gen_nsec_string[(3 * sizeof(config.SO_MAX_TRANS_GEN_NSEC)) + 1];

    if (users_rec == 1 && _DEBUG) {
        printf("\n ------------------------------------- \n\n");
        printf("            --- UTENTI ---\n");
        printf("\n ------------------------------------- \n\n");
    }

    for (i = 0; i < users_count; i++) {
        switch (child_pid = fork()) {
        
          case -1:
               printf(" ERRORE \n");
               exit(EXIT_FAILURE);
          break;  
        
          case 0: 
               child_pid = getpid();
               users[i].pid = child_pid;

               sprintf(id_users, "%d", i);

               if (_DEBUG) printf(" --- SPAWNED USERS CON PID: %d \n", users[i].pid);
            
               /* CONVERT PARAMETERS (SHARED MEMORY) TO STRING */
               sprintf(shm_mastro_string, "%d", shm_mastro);
               sprintf(shm_user_string, "%d", shm_user);
               sprintf(shm_node_string, "%d", shm_node);
               sprintf(shm_trans_discard_string, "%d", shm_trans_discard);
               sprintf(shm_discard_string, "%d", shm_discard);
            
               /* CONVERT PARAMETERS (SEMAPHORES) TO STRING */
               sprintf(sem_start_string, "%d", sem_start);            

               /* CONVERT PARAMETERS (VARIABLES) TO STRING */
               sprintf(so_num_user_string, "%d", config.SO_USERS_NUM);
               sprintf(so_budget_string, "%d", config.SO_BUDGET_INIT);
               sprintf(so_reward_string, "%d", config.SO_REWARD);
               sprintf(so_num_node_string, "%d", config.SO_NODES_NUM);
               sprintf(so_retry_string, "%d", config.SO_RETRY);
               sprintf(so_min_trans_gen_nsec_string, "%d", config.SO_MIN_TRANS_GEN_NSEC);
               sprintf(so_max_trans_gen_nsec_string, "%d", config.SO_MAX_TRANS_GEN_NSEC);
               
               user_argv[1] = shm_mastro_string;
               user_argv[2] = shm_user_string;
               user_argv[3] = shm_node_string;
               user_argv[4] = shm_trans_discard_string;
               user_argv[5] = shm_discard_string;
               user_argv[6] = sem_start_string;
               user_argv[7] = id_users;
               user_argv[8] = so_num_user_string;
               user_argv[9] = so_budget_string;
               user_argv[10] = so_reward_string;
               user_argv[11] = so_num_node_string;
               user_argv[12] = so_retry_string;
               user_argv[13] = so_min_trans_gen_nsec_string;
               user_argv[14] = so_max_trans_gen_nsec_string;
               user_argv[15] = NULL;
             
               execve("user", user_argv, NULL);
               exit(EXIT_SUCCESS);

            break;
             
  	        default:
            break;
                      
        }
    }
}

void spawn_nodes(int nodes_count) { 
    pid_t child_pid;
    int status, i;

    char *node_argv[12] = {"Node"};

    /* SHARED MEMORY */
    char shm_mastro_string[(3 * sizeof(shm_mastro)) + 1];
    char shm_node_string[(3 * sizeof(shm_node)) + 1];
    char shm_discard_string[(3 * sizeof(shm_discard)) + 1]; 
    char shm_trans_discard_string[(3 * sizeof(shm_trans_discard)) + 1];
    
    /* SEMAPHORES */
    char sem_start_string[(3 * sizeof(sem_start)) + 1];

    /* ID */
    char id_nodes[(3* sizeof(int)) + 1];

    /* VARIABLES */
    char so_num_node_string[(3 * sizeof(config.SO_NODES_NUM)) + 1];
    char so_tp_size_string[(3 * sizeof(config.SO_TP_SIZE)) + 1];
    char so_min_trans_proc_nsec_string[(3 * sizeof(config.SO_MIN_TRANS_PROC_NSEC)) + 1];
    char so_max_trans_proc_nsec_string[(3 * sizeof(config.SO_MAX_TRANS_PROC_NSEC)) + 1];
    
    if(_DEBUG) {
        printf("\n ------------------------------------- \n\n");
        printf("            --- NODI ---\n");
        printf("\n ------------------------------------- \n\n");
    }

    for (i = 0; i < nodes_count; i++) {
        switch (child_pid = fork()) {
        
            case -1:
                printf(" ERRORE \n");
                exit(EXIT_FAILURE);
            break; 
        
            case 0:
                child_pid = getpid();
                nodes[i].pid = child_pid;

                sprintf(id_nodes, "%d", i);

                if (_DEBUG) printf(" --- SPAWNED NODE CON PID: %d \n", nodes[i].pid);

               /* CONVERT PARAMETERS (SHARED MEMORY) TO STRING */
			   sprintf(shm_mastro_string, "%d", shm_mastro);
               sprintf(shm_node_string, "%d", shm_node);
               sprintf(shm_discard_string, "%d", shm_discard);
			   sprintf(shm_trans_discard_string, "%d", shm_trans_discard);
			   
               /* CONVERT PARAMETERS (SEMAPHORES) TO STRING */
			   sprintf(sem_start_string, "%d", sem_start);

               /* CONVERT PARAMETERS (VARIABLES) TO STRING */
               sprintf(so_num_node_string, "%d", config.SO_NODES_NUM);
			   sprintf(so_tp_size_string, "%d", config.SO_TP_SIZE);
			   sprintf(so_min_trans_proc_nsec_string, "%d", config.SO_MIN_TRANS_PROC_NSEC);
			   sprintf(so_max_trans_proc_nsec_string, "%d", config.SO_MAX_TRANS_PROC_NSEC);
               
               node_argv[1] = shm_mastro_string;
               node_argv[2] = shm_node_string;
               node_argv[3] = shm_discard_string;
               node_argv[4] = shm_trans_discard_string;
               node_argv[5] = sem_start_string;
               node_argv[6] = id_nodes;
               node_argv[7] = so_num_node_string;
               node_argv[8] = so_tp_size_string;
               node_argv[9] = so_min_trans_proc_nsec_string;
               node_argv[10] = so_max_trans_proc_nsec_string;
               node_argv[11] = NULL;
                
               execve("node", node_argv, NULL);
               exit(EXIT_SUCCESS);

            break;

  	        default:
            break;        
        }
    }
}

void read_input_file() {
  int c;
  FILE *reader;

  printf("\nCHE TIPOLOGIA DI CONFIG VORRESTI UTILIZZARE ? \n\n");
  printf("DIGITA 1 PER APRIRE CONFIG 1 \n");
  printf("DIGITA 2 PER APRIRE CONFIG 2 \n");
  printf("DIGITA 3 PER APRIRE CONFIG 3 \n");
  printf("DIGITA 4 PER INSERIRE I DATI MANUALMENTE \n\n");
  printf("DIGITA IL NUMERO: \n");

  scanf("%d",&c);
  
  if(c<1 || c>4) {
  	printf("ERRORE: HAI INSERITO UN NUMERO NON CORRETTO... RIPROVA... \n");
  	exit(EXIT_FAILURE);
  }
  
  switch(c) {
  	case -1:
        printf(" ERRORE \n");
        exit(EXIT_FAILURE);
    break;
  
  	case 1: 
  		if((reader = fopen("config/config1.txt", "r")) == NULL ) {
  		    fprintf(stderr, "Errore in apertura \"%s\".\n",	"config1.txt");
		    exit(EXIT_FAILURE);
  		}
    break;

  	case 2: 
  		if((reader = fopen("config/config2.txt", "r")) == NULL ) {
  		    fprintf(stderr, "Errore in apertura \"%s\".\n",	"config2.txt");
		    exit(EXIT_FAILURE);
  		}
    break;

  	case 3: 
  		if((reader = fopen("config/config3.txt", "r")) == NULL ) {
  		    fprintf(stderr, "Errore in apertura \"%s\".\n",	"config3.txt");
		    exit(EXIT_FAILURE);
  		}
    break;
  			
 	case 4: 
  		printf("Inserisci numero utenti:\n");
  		scanf("%d",&config.SO_USERS_NUM);
  		if (config.SO_USERS_NUM <= 0) {
         	     printf("ERRORE: una o piu varibili non rispetta le richieste\n");
                    exit(EXIT_FAILURE); 
                }
  		
  		printf("Inserisci numero nodi:\n");
  		scanf("%d",&config.SO_NODES_NUM);
  		if (config.SO_NODES_NUM <= 0) {
         	     printf("ERRORE: una o piu varibili non rispetta le richieste\n");
                    exit(EXIT_FAILURE); 
                }
                
  		printf("Inserisci il budget iniziale:\n");
  		scanf("%d",&config.SO_BUDGET_INIT);
  		if (config.SO_BUDGET_INIT <= 0) {
         	     printf("ERRORE: una o piu varibili non rispetta le richieste\n");
                    exit(EXIT_FAILURE); 
                }
                 		
  		printf("Inserisci la percentuale di reward:\n");
  		scanf("%d",&config.SO_REWARD);
  		if (config.SO_REWARD <= 0) {
         	     printf("ERRORE: una o piu varibili non rispetta le richieste\n");
                    exit(EXIT_FAILURE); 
                }
                  		
  		printf("Inserisci il minimo valore del tempo:\n");
  		scanf("%d",&config.SO_MIN_TRANS_GEN_NSEC);
  		if (config.SO_MIN_TRANS_GEN_NSEC <= 0) {
         	     printf("ERRORE: una o piu varibili non rispetta le richieste\n");
                    exit(EXIT_FAILURE); 
                }
                  		
  		printf("Inserisci il massimo valore del tempo:\n");
  		scanf("%d",&config.SO_MAX_TRANS_GEN_NSEC);
  		if (config.SO_MAX_TRANS_GEN_NSEC <= 0) {
         	     printf("ERRORE: una o piu varibili non rispetta le richieste\n");
                    exit(EXIT_FAILURE); 
                }
                  		
  		printf("Inserisci il numero massimo di fallimenti consecutivi:\n");
  		scanf("%d",&config.SO_RETRY);
  		if (config.SO_RETRY <= 0) {
         	     printf("ERRORE: una o piu varibili non rispetta le richieste\n");
                    exit(EXIT_FAILURE); 
                }
                
  		printf("Inserisci il numero massimo di transazioni:\n");
  		scanf("%d",&config.SO_TP_SIZE);
  		if (config.SO_TP_SIZE <= 0) {
         	     printf("ERRORE: una o piu varibili non rispetta le richieste\n");
                    exit(EXIT_FAILURE); 
                }
                  		
  		printf("Inserisci il minimo valore del tempo simulato:\n");
  		scanf("%d",&config.SO_MIN_TRANS_PROC_NSEC);
  		if (config.SO_MIN_TRANS_PROC_NSEC <= 0) {
         	     printf("ERRORE: una o piu varibili non rispetta le richieste\n");
                    exit(EXIT_FAILURE); 
                }
                 		
  		printf("Inserisci il massimo valore del tempo simulato:\n");
  		scanf("%d",&config.SO_MAX_TRANS_PROC_NSEC);
  		if (config.SO_MAX_TRANS_PROC_NSEC <= 0) {
         	     printf("ERRORE: una o piu varibili non rispetta le richieste\n");
                    exit(EXIT_FAILURE); 
                }
                  		
  		printf("Inserisci la durata della simulazione:\n");
  		scanf("%ld",&config.SO_SIM_SEC);
  		if (config.SO_SIM_SEC <= 0) {
         	     printf("ERRORE: una o piu varibili non rispetta le richieste\n");
                    exit(EXIT_FAILURE); 
                }    		
  	break;

  	default: 
  	break;

  }
  
  if(c!=4) {
	
     fscanf(reader,"%d\n",&config.SO_USERS_NUM);
     fscanf(reader,"%d\n",&config.SO_NODES_NUM);
     fscanf(reader,"%d\n",&config.SO_BUDGET_INIT);
     fscanf(reader,"%d\n",&config.SO_REWARD);
     fscanf(reader,"%d\n",&config.SO_MIN_TRANS_GEN_NSEC);
     fscanf(reader,"%d\n",&config.SO_MAX_TRANS_GEN_NSEC);
     fscanf(reader,"%d\n",&config.SO_RETRY);
     fscanf(reader,"%d\n",&config.SO_TP_SIZE);
     fscanf(reader,"%d\n",&config.SO_MIN_TRANS_PROC_NSEC);
     fscanf(reader,"%d\n",&config.SO_MAX_TRANS_PROC_NSEC);
     fscanf(reader,"%ld\n",&config.SO_SIM_SEC);

     fclose(reader);

  }
   
}

void calculate_budget_user() {
    int i,j,k,m;
    int min_budget = 0;
    int min_pid = 0;
    int max_budget = 0;
    int max_pid = 0;
 
    if(print_budget == 1){
        printf("\n ===================================== \n");
        printf("\n   --- BILANCIO UTENTI E NODI : ---\n");
        printf("\n ===================================== \n\n");
    }   

        for(m = 0; m < config.SO_USERS_NUM; m++) {
            
            if(config.SO_USERS_NUM <= MAX_USERS || print_budget == 1) {
               printf(" -- [%d]: PID USER: %d ",m+1, users[m].pid);
               printf(" BUDGET ATTUALE: %d \n", users[m].budget);
            } else {
       
                if(users[m].budget < min_budget || min_budget == 0) {
                   min_budget = users[m].budget;
                   min_pid = m;
                } 
    
                if(users[m].budget > max_budget && users[m].dead != config.SO_USERS_NUM) {
                   max_budget = users[m].budget;
                   max_pid = m;
                } 
              }
        }

        if(print_budget == 0){
            printf(" UTENTE CON PID %d HA IL MAX BUDGET, che e': %d \n",users[max_pid].pid,max_budget);
            printf(" UTENTE CON PID %d HA IL MIN BUDGET, che e': %d \n",users[min_pid].pid,min_budget);
        } 

}

void calculate_budget_node() {
    int i,j,k,m;
    int current_balance;
    int min_budget = 0;
    int min_pid = 0;
    int max_budget = 0;
    int max_pid = 0;

        for(i=0; i < config.SO_NODES_NUM; i++) {
            current_balance = 0;

            for(j=0; j < pnt_shm_mastro[0].index; j++) {
                if (pnt_shm_mastro[j].transactions[SO_BLOCK_SIZE-1].receiver == nodes[i].pid) {
                    current_balance = current_balance 
                    + pnt_shm_mastro[j].transactions[SO_BLOCK_SIZE-1].quantita;
                }
            } 
            
            if(config.SO_NODES_NUM <= MAX_NODES || print_budget == 1) {
                 printf(" ++ [%d]: PID NODE: %d ",i+1, nodes[i].pid);
                 printf("  BUDGET ATTUALE: %d \n", current_balance);
                 
                } else {

                    if(current_balance < min_budget || i == 0) {
                       min_budget = current_balance;
                       min_pid = i;
                    } 
    
                    if(current_balance > max_budget) {
                       max_budget = current_balance;
                       max_pid = i;
                    } 
                  }
        }

        if(print_budget == 0){
            printf(" NODO CON PID %d HA IL MAX BUDGET, che e': %d \n",nodes[max_pid].pid,max_budget);
            printf(" NODO CON PID %d HA IL MIN BUDGET, che e': %d \n",nodes[min_pid].pid,min_budget);
        }  
}

void monitoring_simulation() {
    printf(" --- COUNTER SIMULATION: (%d secondi)\n", sim_count++);

    sem_reserve(sem_start,2);
    if(pnt_shm_mastro[0].index == SO_REGISTRY_SIZE-1) { kill(getpid(), SIGUSR1);}
    sem_release(sem_start,2);

    print_budget = 0;
    calculate_budget_user();
    calculate_budget_node();

    sem_reserve(sem_start,1);
    printf(" -- NUM UTENTI: %d  -- NUM NODI: %d \n", users[0].dead, config.SO_NODES_NUM);
    if(users[0].dead==0) { kill(getpid(), SIGUSR2);}
    sem_release(sem_start,1);
    
    printf("\n ===================================== \n\n");
}

void simulation_started() {

    printf("\n ===================================== \n\n");
    printf("      --- AVVIO DELLA SIMULAZIONE --- \n");
    printf("\n ===================================== \n\n");
    
    printf(" TEMPO SIMULAZIONE : %ld secondi\n\n", config.SO_SIM_SEC);

    alarm(1);
    while(1){}

    request.tv_nsec = 0;
    request.tv_sec = 1;
    nanosleep(&request, NULL);
}

void stats() {
    int i,l,m;
    int choose;

    for(i=0; i<config.SO_USERS_NUM; i++){kill(users[i].pid, SIGTERM);}
    for(i=0; i<config.SO_NODES_NUM; i++){kill(nodes[i].pid, SIGTERM);}

    sem_reserve(sem_start,1);
    printf(" UTENTI TERMINATI PREMATURAMENTE: %d\n", (config.SO_USERS_NUM - users[0].dead));
    sem_release(sem_start,1);

    printf("\n NUMERO BLOCCHI SCRITTI: %d SU %d\n\n", pnt_shm_mastro[0].index-1, SO_REGISTRY_SIZE);

    alarm(0);

    printf("\n\n COME VUOI PROCEDERE ? \n");
    printf(" DIGITA 1 PER STAMPARE I BLOCCHI \n");
    printf(" DIGITA 2 PER STAMPARE LE STATISTICHE FINALI \n");
    scanf(" %d", &choose);

    switch(choose) {
        case 1: 
            
            for(l = 0; l < pnt_shm_mastro[0].index; l++) { 
                printf(" -- BLOCCO NUM: %d -- \n", l);

                for(m = 0; m < SO_BLOCK_SIZE; m++) {
                    printf("TRANS N: %d PID SENDER: %d PID RECEIVER : %d TIMESTAMP: %ld QUANTITA: %d REWARD: %d\n",
                    m,
                    pnt_shm_mastro[l].transactions[m].sender,
                    pnt_shm_mastro[l].transactions[m].receiver,
                    pnt_shm_mastro[l].transactions[m].timestamp, 
                    pnt_shm_mastro[l].transactions[m].quantita,
                    pnt_shm_mastro[l].transactions[m].reward);
                }
            }

            

            print_budget = 1;
            calculate_budget_user();
            printf("\n");
            calculate_budget_node();
            
            printf("\n NUMERO TRANSAZIONI PRESENTI IN POOL PER OGNI SINGOLO NODO:\n");
            for(i=0; i < config.SO_NODES_NUM; i++) {
                printf("\n [%d]: %d", i+1, nodes[i].trans_rim);
            }

            printf("\n");
        break;

        case 2: 
            print_budget = 1;
            calculate_budget_user();
            printf("\n");
            calculate_budget_node();
            
            printf("\n NUMERO TRANSAZIONI PRESENTI IN POOL PER OGNI SINGOLO NODO:\n");
            for(i=0; i < config.SO_NODES_NUM; i++) {
                printf("\n [%d]: %d", i+1, nodes[i].trans_rim);
            }

            printf("\n");
        break;

        default:
            printf("HAI DIGITATO UN NUMERO SBAGLIATO \n");
            clean_up(0);
            exit(-1);
        break;
    }

    printf("\n");

}
/* ===================================================================== */

void signals_handler(int signum) {
    int i;
    switch (signum) {
        case SIGINT:
            printf("\n MESS: HAI DIGITATO CTRL+C \n");
            for(i=0; i<config.SO_USERS_NUM; i++){kill(users[i].pid, SIGINT);}
            for(i=0; i<config.SO_NODES_NUM; i++){kill(nodes[i].pid, SIGINT);}
            clean_up(1);
        break;

        case SIGALRM:
            monitoring_simulation();
            if (sim_count == (config.SO_SIM_SEC + 1)) {
                printf("\n MESS: SONO PASSATI %d SECONDI \n",sim_count-1);
                stats();
                clean_up(0);
            } else {alarm(1);}      
        break;

        case SIGUSR1:
            printf("\n MESS: SPAZIO ESAURITO (LIBRO MASTRO) \n");
            alarm(0);
            stats();
            clean_up(0);
        break;

        case SIGUSR2:
            printf("\n MESS: NUM USERS INSUFFICIENTI \n");
            alarm(0);
            stats();
            clean_up(0);
        break;    

        default:
        break;
    }
}

void clean_up(int k) {
 
    /* SEMAFORI */
    semctl(sem_start,NUM_SEM,IPC_RMID);

    /* SHARED MEMORY */
    shmdt(pnt_shm_mastro);
    shmctl(shm_mastro, IPC_RMID, NULL);
    shmdt(users);
    shmctl(shm_user, IPC_RMID, NULL);
    shmdt(nodes);
    shmctl(shm_node, IPC_RMID, NULL);

    shmctl(shm_trans_discard, IPC_RMID, NULL);
    shmdt(pnt_discard);
    shmctl(shm_discard, IPC_RMID, NULL);

    printf("\n ===================================== \n");
    printf("\n    --- PULIZIA : COMPLETATO --- \n");
    printf("\n ===================================== \n\n");

    if(k==0){exit(EXIT_SUCCESS);} else if(k==1) exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int k, choose;

    printf("BENVENUTO !! VUOI CONTINUARE CON L'APERTURA DEL FILE ???? \n");
    printf("DIGITA 1 PER CONTINUARE \n");
    printf("DIGITA 2 PER TERMINARE \n");
    scanf(" %d", &choose);

    switch(choose) {
        case 1: 
            read_input_file();
        break;

        case 2: 
            printf("OK HAI TERMINATO \n");
            exit(-1);
        break;

        default:
            printf("HAI DIGITATO UN NUMERO SBAGLIATO \n");
            exit(-1);
        break;
    }

    if (_DEBUG) printf("\n ALLOCO LA SHARED MEMORY...\n");

    /* ============ INIZIALIZZO LE MEMORIE CONDIVISE ================ */

    shm_mastro = shmget(IPC_PRIVATE, sizeof(Block)*SO_REGISTRY_SIZE, 0666|IPC_CREAT);
    EXIT_ON_ERROR;
    pnt_shm_mastro = shmat (shm_mastro, NULL, 0); 
    EXIT_ON_ERROR;
    pnt_shm_mastro[0].index = 0;

    shm_user = shmget (IPC_PRIVATE, sizeof(Users)*(config.SO_USERS_NUM), 0666|IPC_CREAT);
    EXIT_ON_ERROR;                                                                      
    users = shmat (shm_user, NULL, 0); 
    EXIT_ON_ERROR;    
    users[0].dead = config.SO_USERS_NUM;

    shm_node = shmget (IPC_PRIVATE, sizeof(Nodes)*(config.SO_NODES_NUM), 0666|IPC_CREAT);  
    EXIT_ON_ERROR;
    nodes = shmat (shm_node, NULL, 0);                                            
    EXIT_ON_ERROR;
    
    shm_trans_discard = shmget(IPC_PRIVATE, sizeof(TransData)*BUFFER_SIZE, 0666|IPC_CREAT);
    EXIT_ON_ERROR;

    shm_discard = shmget(IPC_PRIVATE, sizeof(TransData)*BUFFER_SIZE, 0666|IPC_CREAT);
    EXIT_ON_ERROR;
    pnt_discard = shmat(shm_discard, NULL, 0);
    EXIT_ON_ERROR;
    pnt_discard[0] = 0;

    if(_DEBUG) printf("\n AREA DI MEMORIA CONDIVISA AGGANCIATA...\n");
        
    /* ===================== INIZIALIZZO I SEMAFORI ======================== */

    if (_DEBUG) printf("\n CREAZIONE DEI SEMAFORI...\n");

    if ((sem_start = semget(IPC_PRIVATE, NUM_SEM, IPC_CREAT | IPC_EXCL |0666)) == -1) { EXIT_ON_ERROR;}

    for (k=0; k<NUM_SEM; k++){create_semaphore(sem_start,k);}
    semctl(sem_start,6,SETVAL,((config.SO_USERS_NUM)+(config.SO_NODES_NUM)+1)-2);

    if (_DEBUG) printf("\n SEMAFORI CREATI \n\n");
    
    /* ===================================================================== */

    spawn_users(config.SO_USERS_NUM, 1);
    spawn_nodes(config.SO_NODES_NUM);

    sem_release(sem_start,6);
    sem_zero(sem_start,6);

    act_signal_handler.sa_handler = signals_handler;
    act_signal_handler.sa_flags = 0;
    sigemptyset(&my_mask);
    act_signal_handler.sa_mask = my_mask;

    sigaction(SIGINT, &act_signal_handler, NULL);
    sigaction(SIGALRM, &act_signal_handler, NULL);
    sigaction(SIGUSR1, &act_signal_handler, NULL);
    sigaction(SIGUSR2, &act_signal_handler, NULL);

    simulation_started();
    clean_up(0);

    return 0;
}