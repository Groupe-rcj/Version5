#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/statvfs.h>
#include"../en_tete/disk.h"
#include"../en_tete/interface.h"

void get_disk_stat(disk_info* time , int x , int y , int largeur , int hauteur)
{
    int i=0 , temp=y ; 
    char line[256] ;
    FILE* fichier=NULL ;
    fichier=fopen("/proc/diskstats" , "r") ; 
    if(fichier==NULL)
    {
        printf("Impossible d'ouvrir /proc/diskstats , cause possible \n") ;
        perror("fopen") ;
        return;      

    }
    while (fgets(line , 256 , fichier)!=NULL)
    {
        sscanf(line, "%*d %*d %s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu ",
        time->device_name,
        &time->reads, &time->reads_merged, &time->sectors_read, &time->time_reading,
        &time->writes, &time->writes_merged, &time->sectors_written, &time->time_writing,
        &time->ios_current, &time->time_ios, &time->weighted_time_ios ) ;
        i++ ;
        if(strstr(time->device_name ,"loop")!=NULL)
        {
            continue ;
        }
        printf("\033[%d;%dHPeripherique: %s", y , x , time->device_name);
        printf("\033[%d;%dH  Lecture: %llu (combine: %llu)", y+1 , x , time->reads, time->reads_merged);
        printf("\033[%d;%dH  Ecriture: %llu (combine: %llu)", y+2 , x , time->writes, time->writes_merged);
        printf("\033[%d;%dH  I/O actuel: %llu", y+3 , x , time->ios_current);

        y+=5 ;
        if(y>hauteur-15)
        {
            y=temp ;
            x=largeur/2 ;
        }
    }
    fclose(fichier);
    printf("Nombre de peripheriques blocs (avec les peripheriques loop ) : %d\n\n" , i) ; 
}

void get_file_usage(char *filename , int x , int y )
{
    struct statvfs fs ; 
    unsigned long total , available ; 
    long unsigned int  inode_total , inode_available ;

    if(statvfs(filename, &fs)!=0)
    {
        printf("Information sur / no disponible\n") ; 
        return;
    }
    total=fs.f_blocks*fs.f_bsize ; 
    available=fs.f_bavail*fs.f_bsize ; 
    inode_total=fs.f_files ;
    inode_available=fs.f_favail ;
    printf("\033[%d;%dHStatistique de fichier venant de  : %s \n" , y , x , filename) ;
    printf("\033[%d;%dH LIBRE (utilisateur normal) : %lu sur %lu \n" , y+1 ,x , inode_available , inode_total) ;
    printf("\033[%d;%dH LIBRE (utilisateur normal) : %Lf sur %Lf(Gigaoctets)\n\n" , y+2 , x  , (long double)available/(1024*1024*1024) , (long double)total/(1024*1024*1024)) ;

}

void get_disk_information(int x , int y , int x_home , int y_home )
{
    int hauteur , largeur ;
    disk_info time ; 
    get_screen_size(&largeur , &hauteur) ;
    get_disk_stat(&time , y , x , largeur , hauteur) ;
    get_file_usage("/" , 1, y_home) ;
    get_file_usage("/home" , x_home , y_home ) ;
}
