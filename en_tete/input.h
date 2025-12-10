
/**
*@file input.h
*/

/**
*@brief Fonction qui regarde les threads actives et les arrete
*/
void check_active_thread();

/**
*@brief Fonction qui gere les entrees claviers de tout l'interface de controle principale du programme de la section 1
* il se charge d'activer les bons threads selon les entrees claviers
*/
void* get_input() ;
