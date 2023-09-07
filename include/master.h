#ifndef MASTER_H
#define MASTER_H

/* DICHIARAZIONE DELLE FUNZIONI */
void spawn_users(int SO_USERS_NUM, int users_rec);
void spawn_nodes(int SO_NODES_NUM);
void read_input_file();
void calculate_budget_user();
void calculate_budget_node();
void simulation_started();
void monitoring_simulation();
void stats();
void signals_handler(int signum);
void clean_up();

#endif