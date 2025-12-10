#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<stdatomic.h>
#include<time.h>
#include<sys/select.h>
#include"../en_tete/ram.h"
#include"../en_tete/define.h"
#include"../en_tete/interface.h"
#include"../en_tete/cpu.h"
#include"../en_tete/network.h"

extern atomic_int run_ram ;

void print_number(int x , int y , int num , char*** pattern_number)
{
    int i ;
    for(i=0 ; i<7 ; i++)
    {
        printf("\033[%d;%dH%s" , y+i , x , pattern_number[num][i]) ;
    }
    fflush(stdout); //forcer l'affichage des donnees dans le buffer
}

void char_to_int(char* value , int y , int x , char*** pattern_number)
{
    int i , j , temp=x;
    char number[10]={"0123456789"} ; 
    for(j=0 ; value[j]!='\0' ; j++)
    {
        if(value[j]=='.') //virgule dans les float
        {
            print_point(y+5 , x );
            x-=6; 
        }
        for(i=0 ; i<10 ; i++)
        {
            if(value[j]==number[i])
            {
                print_number(x , y , i , pattern_number) ;
                break ;   
            }
        }
        x+=10 ;
        print_percent(y+2 , temp+54) ;
    }
}

void get_ram_state( ram_info* time1 )
{
    FILE* fichier ; 
    char* ligne=malloc(100) ; 
    fichier=fopen("/proc/meminfo" , "r") ;

    if(fichier==NULL)
    {
        printf("Couldn't open /proc/meminfo") ; 
        return;
    }

    while(fgets(ligne , 100 , fichier)!=NULL)
    {
        if(strstr(ligne , "MemTotal")!=NULL)
        {
            sscanf(ligne , "MemTotal: %f kB" , &time1->memtotal) ; 
        }
        else if(strstr(ligne , "MemFree")!=NULL)
        {
            sscanf(ligne , "MemFree: %f kB" , &time1->memfree) ; 
        }
        else if(strstr(ligne , "MemAvailable")!=NULL)
        {
            sscanf(ligne , "MemAvailable: %f kB" , &time1->memavailible) ; 
        }
        else if(strstr(ligne , "Buffers")!=NULL)
        {
            sscanf(ligne , "Buffers: %f kB" , &time1->buffers) ; 
        }
        else if(strstr(ligne , "Cached")!=NULL)
        {
            sscanf(ligne , "Cached: %f kB" , &time1->cached) ; 
        }
        else if(strstr(ligne , "SwapTotal")!=NULL)
        {
            sscanf(ligne , "SwapTotal: %f kB" , &time1->swaptotal) ; 
        }
        else if(strstr(ligne , "SwapFree")!=NULL)
        {
            sscanf(ligne , "SwapFree: %f kB" , &time1->swapfree) ; 
        }
    }
    fclose(fichier) ; 
    free(ligne) ; 
}

void print_other_information(int y , int x , float cached , float buffer )
{
    printf("\033[%d;%dHCACHED MEMORY : %.3f Giga" , y ,x , cached) ; 
    printf("\033[%d;%dHBUFFER SIZE : %.3f Giga" , y+1 , x , buffer) ; 
}


process* create_process(char* pid , char* name , unsigned int ram_vmrss )
{
    process* p =(process*)malloc(sizeof(process)) ;
    strcpy(p->name , name ) ;
    strcpy(p->pid ,pid) ;
    p->ram_vmrss=ram_vmrss ; 
    p->next=NULL ;  
    return p ;  
}

void insert_process_by_ram(process **tete, char* pid, char* name, unsigned int ram_vmrss) 
{
    process *nv = create_process(pid, name, ram_vmrss);
    process *courant, *precedent = NULL;
    
    // Cas liste vide ou insertion en tete
    if (*tete == NULL || ram_vmrss > (*tete)->ram_vmrss) 
    {
        nv->next = *tete;
        *tete = nv;
        return;
    }
    
    // Parcours pour trouver la position d'insertion
    courant = *tete;
    while (courant != NULL && courant->ram_vmrss > ram_vmrss) 
    {
        precedent = courant;
        courant = courant->next;
    }
    
    // Insertion
    if (precedent != NULL) 
    {
        precedent->next = nv;
    }
    nv->next = courant;
}



void free_process_list(process *tete) 
{
    process *courant = tete;
    process *suivant;
    
    while (courant != NULL) 
    {
        suivant = courant->next;
        free(courant);
        courant = suivant;
    }
}

void print_process(process* tete, int y, int x, int hauteur) 
{
    process *tmp = tete; 
    int count = 0;
    
    bouge_curseur(y, x);
    printf("PROCESSUS GOURMANDS : "); 
    
    while(tmp != NULL && count < 15 && y < hauteur - 2) // Limiter à 15 processus
    { 
        y++;
        bouge_curseur(y, x);
        printf("PID: %s => %s : %u Mo", tmp->pid, tmp->name, tmp->ram_vmrss); 
        tmp = tmp->next; 
        count++;
        fflush(stdout); 
    }
}


process* get_heavy_proc(int y, int x, int hauteur , int print ) 
{
    DIR *directory; 
    struct dirent *entry;
    directory = opendir("/proc");
    process *first = NULL;
    
    if(directory == NULL) 
    {
        printf("Dossier /proc non ouvert\n");
        return NULL;
    } 
    
    while((entry = readdir(directory)) != NULL) 
    {
        if(is_number(entry->d_name)) 
        {
            char status[270], line[100], name[100] = "";
            unsigned int ram_vmrss = 0;
            int found_vmrss = 0;
            
            snprintf(status, sizeof(status), "/proc/%s/status", entry->d_name); 
            FILE* fichier = fopen(status, "r"); 
            
            if(fichier == NULL) 
            {
                continue; 
            } 
            
            while(fgets(line, sizeof(line), fichier)) 
            {
                if(strstr(line, "Name:")) 
                {
                    sscanf(line, "Name: %99s", name); 
                }
                if(strstr(line, "VmRSS:")) 
                {
                    sscanf(line, "VmRSS: %u", &ram_vmrss); 
                    found_vmrss = 1;
                }
            }
            fclose(fichier);
            
            // N'insérer que si VmRSS a été trouvé et > 0
            if (found_vmrss && ram_vmrss > 0) 
            {
                insert_process_by_ram(&first, entry->d_name, name, ram_vmrss/1024); 
            }
        }
    }
    closedir(directory);
    if(print==1)
    {
        print_process(first, y, x, hauteur);
    }
    return first ;
}

void* print_ram_state() 
{
    int largeur, hauteur ;
    float used; 
    ram_info time1;
    char*** pattern_number = NULL ,  temp[10] ;
    process* first ;
    efface_ecran();
    
    pattern_number = get_pattern();  
    get_screen_size(&largeur, &hauteur);
    configuration() ; 
    time_t last_refresh=time(NULL) ;

    while(atomic_load(&run_ram))
    { 
        time_t now =time(NULL) ;
        if(difftime(now , last_refresh>=2.0))
        {
            efface_ecran();
            get_ram_state(&time1);
            
            // Affichage mémoire principale
            printf("USED MEMORY : %.3f Giga upon %.3f Giga\n", 
                (time1.memtotal - time1.memavailible) / (1024*1024), 
                time1.memtotal / (1024*1024)); 
            
            used = ((time1.memtotal - time1.memavailible) / time1.memtotal) * 100; 
            sprintf(temp, "%.3f", used); 
            
            if(used > 80) 
            {
                printf(ROUGE);
            } 
            else if(used > 50) 
            {
                printf(JAUNE); //niveau intermédiaire
            } 
            else 
            {
                printf(VERT);
            }
            
            char_to_int(temp, 4, 4, pattern_number);
            printf(NEUTRE);
            
            // Affichage swap
            bouge_curseur(1, largeur/2); 
            printf("USED SWAP : %.3f Giga upon %.3f Giga\n",
                (time1.swaptotal - time1.swapfree) / (1024*1024), 
                time1.swaptotal / (1024*1024)); 
            
            used = ((time1.swaptotal - time1.swapfree) / time1.swaptotal) * 100; 
            sprintf(temp, "%.3f", used);
            
            if(used > 10) 
            { // Seuil plus bas pour le swap
                printf(ROUGE);
            } 
            else 
            {
                printf(VERT);
            }
            
            char_to_int(temp, 4, largeur/2, pattern_number);
            printf(NEUTRE);
 
            
            print_other_information(12, largeur/2, time1.cached/(1024*1024), time1.buffers/(1024*1024)); 
            first=get_heavy_proc(12, 1, hauteur , 1 ); 
            free_process_list(first); // Libérer la mémoire après utilisation
            print_left_arrow(hauteur-6 , largeur-20 ,"CPU") ; 
            print_right_arrow(hauteur-6 , largeur-10 ,"NETWORK") ;
            printf("\033[%d;%dH Press q to exit" , hauteur-1 , 1 )  ;
            fflush(stdout) ;
            last_refresh=now ;
        }
       
        usleep(500000);
    }
    return NULL ; 
}


