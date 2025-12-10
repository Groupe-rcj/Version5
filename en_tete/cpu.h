/**
*@file cpu.h
*/

#ifndef CPU
#define CPU

#define LINE_SIZE 2048
#include"../en_tete/ram.h"

/**
*@brief Structure qui represente les activites du CPU plus percisement des coeurs du processeurs et du proceseurs en general
*/
struct cpu_usage
{
    unsigned long long int user_consommation ;
    unsigned long long int modified_nice ;
    unsigned long long int system_consommation ;
    unsigned long long int inactivity ;
    unsigned long long int iowait ;
    unsigned long long int irq ;  
    unsigned long long int soft_irq ;
    unsigned long long int stoled_hyp ;
    unsigned long long int vm ;
    unsigned long long int nice_vm ; 
} ; 
typedef struct cpu_usage cpu_usage ;

struct double_cpu_usage
{
    long double user_consommation ;
    long double modified_nice ;
    long double system_consommation ;
    long double inactivity ;
    long double iowait ;
    long double irq ; 
    long double soft_irq ;
    long double stoled_hyp ;
    long double vm ;
    long double nice_vm ; 
} ; 
typedef struct double_cpu_usage double_cpu_usage ;

/**
*@brief Focntion qui permet de compter les coeurs d'un processeur en coptant les nombres de lignes contenant 
*le mot cpu au debut des lignes dans le fichier "/pros/stat"
*@return int : le nombre de coeur 
*/
int core_number() ;

/**
*@brief Fonction qui permmet de parser une ligne pour obtenir les informations sur le processeur
*@param char* ligne :
*@param cpu_usage *cpu_info : pointeur ou stocker les informations obtenues 
*@param int i : identifiant pour les coeurs
*/
void get_info( char* ligne , cpu_usage *cpu_info , int i ) ;

/**
*@brief Fonction de simple affichage des en tetes dan le tableau d'affichage de sinformations sur le CPU
*@param decalage : Le declaga entre l'affichage des en tetes des CPU
*/
void affiche_en_tete(int decalage) ;

/**
*@brief Fonction pour l'affichage des informations sur les coeurs et le processeurs en general
*/
void print_cpu(int cpu_number , int y , double_cpu_usage cpu , long double consommation , float* frq , int decalage) ;


/**
*@brief Fonction qui permet de calculer les informations su l'usage de CPU en pourcentage
*On capture les iformations sur deux intervalles de temo\ps pour l'evaluation et on  fait la difference
*/

long double get_statistic(  int print , int i  , cpu_usage time1 , cpu_usage time2 , int nb_coeur , int cpu_number , int y , float* frq , int decalage ) ;

/**
*@brief Fonction qui permet de creer une structure processus pour le triage des processus selon l'usage de CPU
*@param pid : le PID du procesus a creer
*@param name : nom du processus a creer
*@param cpu_consommation : la consommation en CPU en % du processus
*@return process* : retourne le processus creer 
*/
process* create_process_cpu(char* pid , char* name , float cpu_consommation );

/**
*@brief Fonction permettant d'ajouter d'element a la liste chainee des processus a trier pour savoir les consommations CPU
*@param tete : la tete dde la liste chainee ou ajouter les elements  
*@param nv : Nouveau processus a ajouter
*/
void add_element(process** tete , process* nv ) ;

/** 
*@brief Fonction qui permet d'afficher les listes chaines des processus
*@param tete : l'adresse du premer element de la liste des processus
*@param y : ligne ou on commence a affciher la liste des processus
*@param x : colonne ou on affiche la liste
*@param hauteur : hauteur de l'ecran pour ne pas deborder l'affichage
*/
void print_process_cpu(process* tete, int y, int x, int hauteur) ;

/**
*@brief Fonction qui permet de trier les processus selon l'usage du CPU par ordre decroissant 
*Il s'agit de calculer la consommation sur deux intervalles separees et inserer les vraies valeur apres avoir fait la difference
*@param tete1 : adresse du premier element de la liste chaine de la premiere prise d'information
*@param tete2 : adresse du premier element de la liste chaine de la seconde prise d'information
*@param tete : liste chaine ou arranger reellement les elements
*/
void insert_process_by_cpu(process* tete1, process* tete2, process** tete , unsigned int total) ;

/**
*@brief Fonction qui permet de faire les prises d'information sur les deux intervalles et afficher les processus 
*les plus gourmands en processeurs
*@param hauteur : hauteur de l'ecran pour eviter le debordement dans l'affichage des processus
*@param x : colonne ou afficher la liste
*@param y : ligne ou on commence a afficher la liste de processus 
*/
process* get_heavy_proc_cpu(int hauteur , int x , int y , int print) ;

/**
*@brief Fonction qui permet de prendre les frequences de chaque coeur cad le nombre d'operations par secondes
*Ces informations de trouvenst dans "/sys/devices/system/cpu/cpu./cpufreq/scaling_cur_freq" qui et en HZ que cette fonction transforme en Gigahertz par seconde
*@param nb_coeur : nombre de coeur sur le systeme
*@param *frq : tableau ou stocker les frequences de chaque coeur
*/
void get_frq(int nb_coeur , float* frq) ;

/**
*@brief Fonction qui pernet de savoir la charge du CPU cad la moyenne des processsus actuellement traites par le processeurs
*en temps reel et qui affiche aussi le PID du dernier processus cree
*/
void print_load_avg() ;


/**
*@brief Fonction principale du thread qui se charge de l'obtention de l'etat du CPU en temps reel
*/
void* get_cpu_state();

/** 
*@brief Fonction qui permet de creer les threads et les executer , elle attend la fin de ces threads
*/
void execute_cpu_thread() ;

#endif




