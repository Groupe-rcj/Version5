/**
*@file liste_proc.h
*/

/**
*@brief Structure qui contient les donnees sur les signaux
*@param num le numero du signal defini par le macros dans signal.h
*@param name le nom du signal
*/
typedef struct
{
    int num ;
    char* name ;
} liste_sig ;

/**
*@brief Fonction qui prend l'UID reel
*/
int get_process_uid(int pid) ;
int affichage_proc(int ligne_max, char *file) ;
char* uid_to_username(int uid) ;
void list_processes_proc() ;
char* is_background(pid_t pid_c) ;
int change_priority(int pid, int n) ;
int symbole_to_int(char* signal) ;
int signal_pid(char* spid, int pid) ;
char* authentification() ;
