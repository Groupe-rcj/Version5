/**
*@file disk.h
*/

#ifndef DISK
#define DISK
/**
*@brief Structure qui contient les infos ur le disque
*/
struct disk_info
{
    char device_name[32];
    unsigned long long reads, reads_merged, sectors_read, time_reading;
    unsigned long long writes, writes_merged, sectors_written, time_writing;
    unsigned long long ios_current, time_ios, weighted_time_ios;
} ;
typedef struct disk_info disk_info ; 

/**
*@brief Fonction qui donne les stat sur un fichier donne en argument
*@param filename le nom du fichier a examiner
*@param x la position en largeur ou afficher le stat
*@param y la position en hauteur ou afficher le stat
*/
void get_file_usage(char *filename , int x , int y ) ;

/**
*@brief Fonction 
*/
void get_disk_stat(disk_info* time , int x , int y , int largeur , int hauteur) ;
void get_disk_information(int x , int y , int x_home , int y_home );

#endif