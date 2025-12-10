/**
*@file network.h
*/

#ifndef NETWORK
#define NETWORK

#include<stdio.h>


/**
*@brief Structure permettant de stocker le sinformations sur une interface reseaux comme wlan , lo 
*/
struct net_device_info
{
    char dev_name[30] ; 
    unsigned long int received_bytes ; 
    unsigned long int received_packets ; 
    unsigned long int received_erros ; 
    unsigned long int received_drop ; 
    unsigned long int transmitted_bytes ; 
    unsigned long int transmitted_packets ; 
    unsigned long int transmitted_erros ; 
    unsigned long int transmitted_drop ; 
};
typedef struct net_device_info net_device_info ;

/**
*@brief Strcture qui permet de stocker les informations sur une connexion etablie avec la machine que ce soit une connnexion
*UDP ou TCP
*/
struct connexion_info
{
    char local_port_addr[20] ; 
    char remote_port_addr[20] ; 
    char rx_tx_queue[20] ; 
    char status[5] ; 
    int uid ;
    int inode ;  
};
typedef struct connexion_info connexion_info ; 

/** 
*@brief Fonction  permettant d'ouvrir un fichier pour eviter les erreurs et de repeter le code d'erreurs a chaque fois
*@param filename : Le nom du fichier a ouvrir 
*@param mode : le mode d'ouverture 0 pour readonly et 1 pour write
*@return FILE* : l'adresse du fichier ouvert obtenu en cas de succes
*/
FILE* open_file(char* filename , int mode) ;

/** 
*@brief Fonction permettant de chercher le processus proprietaire d'un descripteur de fichier pour detecter a qui appariennent 
*les connexions etabliess par le parcours de /proc/PID/fd des processus actuellement actif et lier les liens symboliques 
*de ces fichiers necessitant les droits root
*@param fd : le decripteur de fichier a rechercher le proprietaire
*/
char* search_process_owner(int fd) ;


/**
*@brief Fonction permettant de trauire les adresses sous formes hexadecimales groupes 2 par 2 en decimale lisibles d'abord puis de les arranger 
*dans l'ordre inverse par le principe de little indian pour resoudre les adresses IP 
*@param hex : l'adresse en hexadecimale a traduire
*@param port : le port traduit 
*@param char* retourne l'adresse IP
*/
char* hex_IPV4(char* hex , char* port );

/**
*@brief Fonction permettant de connaitre le nombre d'interfaces reseaux existants sur la machine
*par la lecture du nombre de ligne dans le fichier /proc/net/dev  
*/
int interface_number() ;

/**
*@brief Fonction qui permet de permet de donner les informations sur toutes les connexions etablies avec la machine
*@param filename Le nom du fichier a parser car il peut y avoir pluiseurs protocoles sur le PC (UDP , TCP , ...)
*@param mode pour savoir quels en tete de protocoles doit on affciher
*/
void get_connexion_state(char* filename , int mode) ;

/**
*@brief Fonction qui affiche les en_tetes des devices
*/
void print_device_head(int largeur , int x , int y ) ;

void print_correctly(unsigned long int info , int y, int x ) ;

/**
*@brief Fonction qui affiche les informations par interfaces
*/
void print_dev(net_device_info info , int espacement , int y, int x ) ;

/**
*@brief Fonction qui prend et parse les inormations des connexions dans le fichier /proc/net/dev ou chaque lign est une interface 
*/
void get_network_state(int from_start) ;

/**
*@brief Fonction principale du thread d'affichage des informations sur le NETWORK
*/
void* run_network_track() ; 

/**
*@brief Fonction qui permet de creer, d'executer et d'attendre la terminaison des threads 
*/
void look_network() ;

void* sniff() ;
void hexdump(const unsigned char *data, int len) ;
const char *protocol_name(int proto) ;



#endif
