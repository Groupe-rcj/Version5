/**
*@file pid_info.h
*/

#ifndef PID_INFO
#define PID_INFO

/**
*@brief Fonction multifonctionnel qui recherche a la fois le nom d un process et le pid d un nom de processus
*/
int search_process(char*name , int pid , char *return_name) ;


/**
*@brief Structure qui permet de stocker les informations sur le processus a tracker 
*/
struct process_cpu
{
    char state[2] ; 
    int ppid ;
    int pgrp ; 
    int psid ;
    unsigned long int utime ;
    unsigned long int stime ;
    int priority ; 
    int nice ; 
    int num_thread ; 
    unsigned long long int time ;
};
typedef struct process_cpu process_cpu ; 

/**
*@brief Fonction qui permet de calculer la consommation totale du CPU a un instant donnee par la lecture du fichier /proc/stat
*lecture de la premiere ligne qui contient l'etat general de tous les coeurs
*/
unsigned long long int  get_total_cpu();

/**
*@brief Fonction qui permet de recuperer les informations sur le PID a tracker par la lecture du fichier /proc/PID/stat
*@param pid : Le PID du procesus a tracker
*@param info : Structure ou stocker les informations obtenues 
*@param exit : Indique si l'on doit quitter le programme en cas d'erreur
*@return Retourne la consommation CPU du processus
*/
unsigned long int  get_cpu_of_pid(int pid , process_cpu *info , int go_out);

/**
*@brief Fonction qui permet de captuerer les informations sur deux intervalls de 1 secondes et calculer les pourcentages en consommation
*/
long double get_percentage_cpu(int pid ,process_cpu *info2) ;

/**
*@brief Fonction qui permet d'afficher les en tetes es informations 
*/
void print_title();

/**
*@brief Fonction qui permet d'obtenir la consommation en RAM du processus en question
*/
unsigned int get_process_ram(int pid);

/**
*@brief Fonction principale du thread de calcul et d'affichage des informations requises
*/
void* trace_pid(void* pid);

/**
*@brief Fonction principale du thread d'attente des entrees claviers
*/
void* wait_pid_input();

/**
*@brief Fonction qui permet de creer, d'executer et d'attendre la terminaison des threads 
*/
int execute_pid_thread();

#endif