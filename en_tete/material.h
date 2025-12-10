/**
*@file material.h
*/

#ifndef MATERIAL
#define MATERIAL
/** 
*@brief Fonction qui permet d'obtenir les temperatures actuelles de la machine\n
* "Les" temperatures car il peut y avoir plusieurs capteurs sur le PC
*Sous linux ils sont dans le dossier "/sys/class/thermal/thermal_zone?/temp" 
* Les temperatures sont en millidegre C 
*/
void get_temperature(int x , int y , int print , float max_temperature) ;

/** 
*@brief Fonction qui permet de faciliter l'obtention du niveau de la batterie selon certains criteres
*Comme la temperature il peut y avoir plusieurs batteries sur le PC
*Ils sont dan le dossier /sys/class/power_supply/
*@param char* section : indique quelle information sur la batterie doit on donner (en return)
*@param char* dev_name : le nom du peripherique batterie
*@return char* : l'information requise selon la section et le peripherique demande
*/ 
char* get_material_info( const char* section , const char* dev_name) ;

/** 
*@brief Fonction qui permet d'afficher la quantite de couleur a afficher selon l'etat de la batterie
*@param debut : intervalle de debut ou afficher les barres 
*@param fin : ou arreter d afficher les barres de la batterie (ligne)
*@param color : couleur des barres dans la le pattern de batterie selon le niveau de la batterie
*@param y : position initiale suivant la ligne ou commencer a afficher les barres 
*@param x : poistion suivant la colonne
*/
void print_color(int debut , int fin , char*color , int y , int x );


/** 
*@brief Fonction qui affiche les barres selon l etat de la batterie 
*elle utilise la fonction de dessus
*/
void print_battery_level(float capacity , int x , int y );

/**
*@brief Fonction qui permet d'afficher les details sur l'etat des batteries dans le PC
*Elle fait un parcours du repertoire "/sys/class/power_supply" et detaille seulement les batteries presentes
*C'est dans cette fonction que l'on affiche l'etat de la batterie
*/
void get_power(int x , int y) ;

/**
*@brief Fonction principale du thread d'affichage , elle reunie le fonctions qui fait le monitoring le materiel
*incluant la temperature, la batterie, l'etat de disques sur la machine
*@return void* pour etre compatible a une fonction dans un thread
*/
void* get_material_overview() ;

/**
*@brief Fonction qui cree et execute les threads d'affichage de l'etat niveau materiel de la machine 
*et du thread qui attend les entrees utilisateurs 
*Cette fonction attend que le threads se terminent avant de se teminnr elle meme
*/
void execute_material_thread();

#endif