/**
*@file interface.h
*/

#ifndef OPENED_FILE
#define OPENED_FILE

/**
*@brief Structure qui permet de stocker les fichiers ouverts d'un processus
*fic : tableau 2D pour stockere le nom des fichiers
*taille : le nombre de fichiers ouverts par le processus
*/
struct Fichier
{
    char** fic ;
    int taille ;
} ;
typedef struct Fichier Fichier ;

/**
*@brief Fonction qu permet de recuperer tous les fichiers ouverts par un processus par le parcours du dossier du processus :
*\/proc/PID/fd qui  contient des descripteurs de fichiers qui sont des liens symboliques qu'on dechiffre par la fonction readlink 
*@param pid : le PID du processus a tracker
*@return Fichier : la liste des fichiers ouverts par le processus et leur nombre
*/
Fichier list_open_files(int pid) ;

/**
*@brief Fonction qui permet de liberer la memoire allouer par la liste des fichiers ouverts
*@param f : la structure de la liste des fichiers
*/
void free_files(Fichier *f) ;

/**
*@brief Fonction qui permet d'affciher la liste des fichiers ouverts par le parcours du tableau 2D dams la structure Fichier
*@param pid : Le PID du processus a creer
*@param y : La ligne sur laquelle on commence a afficher les fichiers
*/
void print_opened_file(int pid , int y );

#endif