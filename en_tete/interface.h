/**
*@file interface.h
*/

#ifndef INTERFACE
#define INTERFACE

/**
*@brief Union pour stocker divers types de variables
*/
union number
{
    unsigned long long int ulli ; 
    long double lo ; 
    float f ; 
};
typedef union number number ; 

/**
*@brief Fontion qui prend la taille du terminal sur lequel est affiche le programme pour un affichage dynamique
*et conforme
*/
void get_screen_size(int* largeur , int* hauteur) ; 
void efface_ecran() ; 
void bouge_curseur( int ligne , int colonne ) ;
void affiche_au_centre( int hauteur , char* texte , const char* couleur ) ;
void print_colored(number* num , char type ,  int max , int middle , const char* color_max , const char* color_middle , const char* color_min);
void dessine_boite(int x , int y , int hauteur , int largeur , char* color ) ;
char*** malloc_3D() ;
void initiate_pattern(char**pattern ,  char* ligne1 , char* ligne2 , char* ligne3 , char* ligne4 , char* ligne5 , char* ligne6 , char* ligne7 ) ;
char*** get_pattern();
void print_point(int y , int x ) ;
void print_percent(int y , int x );
void print_left_arrow(int y , int x ,char* en_tete) ; 
void print_right_arrow(int y , int x , char* en_tete) ;
/**
*@brief Fonction qui restaure les parametres du terminal
*/
void reset() ;
/**
*@brief Fonction qui met le terminal en mode canonique et pas d'affichage
*/
void configuration() ;
int get_char() ;
void print_array(int ligne , int colonne , int espacement);

/**
*@brief Fonction qui attend les evenements claviers non bloquants par l'usage de select
*/
int kbhit(void);
int is_number(char* name);
void get_array_size(int *largeur , int *decalage) ;
void print_readme_file();
void print_battery_pattern(int x, int y);
void print_temperature_pattern(int y , int x );

#endif







 