/**
*@file ram.h
*/

#ifndef RAM 
#define RAM
/**
*@brief Structure qui permet de stocker les informations sur la RAM  
*/
struct ram_info
{
    float memtotal ; 
    float memfree ; 
    float memavailible ; 
    float buffers ; 
    float cached ; 
    float swaptotal ; 
    float swapfree ;
} ;
typedef struct ram_info ram_info ;  

/**
*@brief Structure qui permet de stocker les informtions sur un processus pour faciliter le reperage des processus gourmanda
*en RAM dans une liste chainee de processus
*/
struct process
{
    char pid[10] ;
    char name[100] ;
    unsigned int ram_vmrss ; 
    unsigned int ram_vmsize ; 
    float cpu_percent_use ;

    struct process *next ; 
};
typedef struct process process ;

/** 
*@brief Obtention de l'etat de la RAM a un instant par la lecture du fichier /proc/meminfo ou les informations sont en kilobytes
*@param time1 : structure ou stocker les informations sur la RAM instantannement 
*/
void get_ram_state( ram_info* time1 ) ;

/**
*@brief Fonction pour afficher les pattern des nombres
*@param x : position en colonne ou placer le pattern 
*@param y : poisition en ligne ou placer le pattern 
*@param pattern_number : tableau a 3 dimenions ou stocker tous les patterns  
*/
void print_number(int x , int y , int num , char*** pattern_number) ;

/**
*@brief Fonction permettant de convertir les donnees chiffres en pattern e chiffre correspondant pour 
*l'affichage du resultat
*@param value : le nombre a traduire en pattern
*@param x : position en colonne ou placer le pattern 
*@param y : poisition en ligne ou placer le pattern 
*@param pattern_number : tableau a 3 dimenions ou stocker tous les patterns  
*/
void char_to_int(char* value , int y , int x , char*** pattern_number) ;

/**
*@brief Fonction pour afficher d'autres informations sur la RAM 
*/
void print_other_information(int y , int x , float cached , float buffer ) ;

/**
*@brief Fonction pour la creation d'une structure processus pour etre classer dans la liste chainee des processus pour  etre ensuite trie et
*pour reperer les processus gourmands en RAM
*@param pid : le PID du processus a creer 
*@param name : le nom du processus
*@param ram_vmrss : La quanite de RAM reellement occupe par le processus non pas ce qu'elle a demande au kernel
*@return process* : l'adresse de la structure processus que la fontion vient de creer 
*/
process* create_process(char* pid , char* name , unsigned int ram_vmrss ) ;

/**
*@brief Fonction permettant de trier les processus selon l'usage de RAM par 
*On ne trie pas la liste chaine mais des l'insertion d'un nouveau processus on le place au bon endroit cad selon sa consommation en RAM
*@param tete : La tete de la liste chainee des processus deja en ordre selon la consommation en RAM
*@param pid : le PID du processus que l'on va inserer dans la liste chainee
*@param name : Le nom du processus que l'on va inserer dans la liste chainee
*@param ram_vmrss : La consommation en RAM du processus que l'on va inserer dans la liste chainee
*/
void insert_process_by_ram(process **tete, char* pid, char* name, unsigned int ram_vmrss) ;

/**
*@brief Fonction permettant de liberer les espaces occupes par lune liste chainee 
*@param tete : La tete de la liste chainee a liberer
*/
void free_process_list(process *tete) ;

/**
*@brief Fonction qui permet d'afficher la liste chainee de processus
*@param tete : Tete de la liste chainee dont on veut afficher
*@param y : Position en ligne ou afficher la liste des processus
*@param x : Position en colonne ou afficher la liste des processs 
*@param hauteur : Hauteur de l'ecran pour eviter les debordements d'affichage de la liste de procesus 
*/
void print_process(process* tete, int y, int x, int hauteur) ;

/**
*@brief Fonction aui permet de tracker les processus gourmands en faisant un parcours du repertoire /proc et d'entrer dans les repertoires dont le nom  est un chiffre
*qui represente un PID et de parser le fichier status de  chacun de ces repertoires pour obtenir la VmRSS 
*@param y : Position en ligne ou afficher la liste des processus
*@param x : Position en colonne ou afficher la liste des processs 
*@param hauteur : Hauteur de l'ecran pour eviter les debordements d'affichage de la liste de procesus 
*/
process* get_heavy_proc(int y, int x, int hauteur , int print) ;

/**
*@brief Fonction principale du thread d'affichage et calcule et recherche des iformations sur la RAM
*/
void* print_ram_state() ; 

/**
*@brief Fonction qui permet de creer et executer les threads d'attente d'entrees claviers et d'affichage et calcul concernant la RAM
*elle attend la fin de ces derniers avec phtread_join()
*/
void execute_ram_thread() ;

#endif

