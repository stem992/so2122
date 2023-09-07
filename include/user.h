#ifndef USER_H
#define USER_H

#include "shared_variables.h"

/* DICHIARAZIONE DELLE FUNZIONI */
void execution();
void users_alive(pid_t pid_dead);
int getBilancio(); 
void delete_trans();
int chooseRandomUser();
int chooseRandomQuantita(int budget);
int chooseRandomReward(int quantita);
int chooseRandomNode();
void user_handler(int signum);
void clean_up_user();
void shared_attach();

#endif