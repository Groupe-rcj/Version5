/**
*@file config_daemon.h
*/ 
#ifndef DAEMON 
#define DAEMON

/**
*@brief Structure qui stocke les seuils a respecter dans le fichier de configuration
*/
struct seuil
{
    int percent_core_cpu ;
    int percent_general_cpu ; 
    int percent_ram ;
    int percent_swap ; 
    float temperature ;
    int interval_time ;
    char found_percent_core_cpu , found_percent_general_cpu , found_percent_ram , found_percent_swap , found_temperature , found_interval_time ;
};
typedef struct seuil seuil ;

/**
*@brief Fonction qui cree le fichier de configuration si il n'existe pas 
*@param filename : le nom du fichier de configuration
*/
int create_config_file(char* filename);

/**
*@brief Fonction qui parse le fichier de configuration et prend les donnees necessaires cad les plafonds a repecter
*@param filename nom du fichier de configuration 
*@return retourne la structure qui contient les plafinds
*/
seuil parse_fichier(char*filename);

/**
*@brief Fonction qui surveille  le fichier de configuration
*/
void* watch_config_file(void* arg);

/**
*@brief Fonction qui cree le demon 
*/
void create_daemon(char** argv);

/**
*@brief Fonction qui prend les informations
*/
void* get_save_info();

/**
*@brief Fonction qui affiche un message dans le fichier de configuration
*/
void print_mesage(char*message , char*type , char* color_type , int simple);

#endif
