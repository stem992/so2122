#ifndef NODE_H
#define NODE_H

#include "shared_variables.h"

/* DICHIARAZIONE DELLE FUNZIONI */
void start();
void execution();
void discard();
int controllo_blocco(TransData tran);
void process_block();
TransData trans_reward();
void node_handler(int signum);
void clean_up_node();
void shared_attach();

#endif