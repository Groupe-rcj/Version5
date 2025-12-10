#include<pthread.h>
#include<stdatomic.h>
#include<time.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include"../en_tete/ram.h"
#include"../en_tete/define.h"
#include"../en_tete/interface.h"
#include"../en_tete/cpu.h"
#include"../en_tete/network.h"
#include"../en_tete/material.h"
#include"../en_tete/menu.h"
#include"../en_tete/disk.h"
#include"../en_tete/input.h"

atomic_int run_cpu=0 , run_ram=0 , run_material=0 , run_network=0 , run_connexion=0, run_sniff=0 ;
pthread_t th_affiche_cpu , th_affiche_ram , th_affiche_mat , th_affiche_net , th_ut , th_sniff;


void* look_connex(void*arg)
{
    int mode = *(int*)arg ;
    char filename[20] ;
    if(mode == 1)
    {
        strcpy(filename , "/proc/net/tcp") ;
    }
    else    
    {
        strcpy(filename , "/proc/net/udp") ;
    }
    time_t now , last_refresh = time(NULL) ;
    while(atomic_load(&run_connexion))
    {
        now = time(NULL) ;
        if(difftime(now , last_refresh)>=1.0 )
        {
            get_connexion_state(filename , mode) ;
            last_refresh=now ;
        }
    }
    return NULL ;
}


void check_active_thread() //arret de tous les threads actifs
{
    if(run_cpu==1)
    {
        atomic_store(&run_cpu , 0) ;
        pthread_join(th_affiche_cpu , NULL) ;
    }
    if(run_ram==1)
    {
        atomic_store(&run_ram , 0) ;
        pthread_join(th_affiche_ram , NULL) ;
    }
    if(run_material==1)
    {
        atomic_store(&run_material , 0) ;
        pthread_join(th_affiche_mat , NULL) ;
    }
    if(run_network==1)
    {
        atomic_store(&run_network , 0) ;
        pthread_join(th_affiche_net , NULL) ;
    }
    if(run_connexion==1)
    {
        atomic_store(&run_connexion , 0) ;
        pthread_join(th_ut , NULL) ;
    }
    if(run_sniff==1)
    {
        atomic_store(&run_sniff , 0) ;
        pthread_join(th_sniff , NULL) ;
    }
    efface_ecran();
}

void* get_input()
{
    char move;
    int  *back=NULL;
    back=malloc(sizeof(int)) ;
    int largeur , hauteur ;
    get_screen_size(&largeur , &hauteur) ;
    while(1)
    {
        if(kbhit()) 
        {
            move = get_char();
            switch(move)
            {
                case 'L':
                {
                    if(run_cpu==1)
                    {
                        atomic_store(&run_cpu , 0) ;
                        pthread_join(th_affiche_cpu , NULL) ;
                        efface_ecran();
                        atomic_store(&run_material ,1 );
                        pthread_create(&th_affiche_mat , NULL , get_material_overview , NULL )  ;
                    }
                    else if(run_ram==1)
                    {
                        atomic_store(&run_ram , 0) ;
                        pthread_join(th_affiche_ram , NULL) ;
                        efface_ecran();
                        atomic_store(&run_cpu ,1 );
                        pthread_create(&th_affiche_cpu , NULL , get_cpu_state , NULL ) ; 
                    }
                    else if(run_material==1)
                    {
                        atomic_store(&run_material , 0) ;
                        pthread_join(th_affiche_mat , NULL) ;
                        efface_ecran();
                        *back=1 ;
                        return (void*)back ;
                    }
                    else if(run_network==1)
                    {
                        atomic_store(&run_network , 0) ;
                        pthread_join(th_affiche_net , NULL) ;
                        efface_ecran();
                        atomic_store(&run_ram ,1 );
                        pthread_create(&th_affiche_ram , NULL , print_ram_state ,NULL) ;
                    }
                    else if(atomic_load(&run_connexion))
                    {
                        check_active_thread() ;
                        atomic_store(&run_network ,1 );
                        pthread_create(&th_affiche_net , NULL , run_network_track , NULL ) ; 
                    }
                    else if(atomic_load(&run_sniff))
                    {
                        check_active_thread() ;
                        atomic_store(&run_network ,1 );
                        pthread_create(&th_affiche_net , NULL , run_network_track , NULL ) ; 
                    }
                }
                break ;
                case 'R':
                {
                    if(run_material==1)
                    {
                        atomic_store(&run_material , 0) ;
                        pthread_join(th_affiche_mat , NULL) ;
                        efface_ecran();
                        atomic_store(&run_cpu ,1 );
                        pthread_create(&th_affiche_cpu , NULL , get_cpu_state , NULL ) ;  
                    }
                    else if(run_cpu==1)
                    {
                        atomic_store(&run_cpu , 0) ;
                        pthread_join(th_affiche_cpu , NULL) ;
                        efface_ecran();
                        atomic_store(&run_ram ,1 );
                        pthread_create(&th_affiche_ram , NULL , print_ram_state ,NULL) ;
                    }
                    else if(run_ram==1)
                    {
                        atomic_store(&run_ram , 0) ;
                        pthread_join(th_affiche_ram , NULL) ;
                        efface_ecran();
                        atomic_store(&run_network ,1 );
                        pthread_create(&th_affiche_net , NULL , run_network_track , NULL ) ; 
                    }
                }
                break ;
                case 't' :
                {
                    if(getuid()!=0)
                    {
                        bouge_curseur(hauteur-5 , 1) ;
                        printf("Permision non accorde") ;
                        sleep(2) ;
                        efface_ecran() ;
                        continue ;
                    }
                    int *mode = malloc(sizeof(int)) ;
                    *mode = 1 ;
                    if(run_network==1 || atomic_load(&run_connexion))
                    {
                        check_active_thread() ;
                        atomic_store(&run_connexion , 1 );
                        pthread_create(&th_ut , NULL , look_connex , mode ) ;
                    }
                }
                break;
                case 'u' :
                {
                    if(getuid()!=0)
                    {
                        bouge_curseur(hauteur-5 , 1) ;
                        printf("Permision non accorde") ;
                        sleep(2) ;
                        efface_ecran() ;
                        continue ;
                    }
                    int *mode = malloc(sizeof(int)) ;
                    *mode = 2 ;
                    if(run_network==1 || atomic_load(&run_connexion))
                    {
                        check_active_thread() ;
                        atomic_store(&run_connexion , 1 );
                        pthread_create(&th_ut , NULL , look_connex , mode) ;
                    }
                }
                break ;
                case 's' :
                {
                    if(getuid()!=0)
                    {
                        bouge_curseur(hauteur-5 , 1) ;
                        printf("Permision non accorde") ;
                        sleep(2) ;
                        efface_ecran() ;
                        continue ;
                    }
                    check_active_thread() ;
                    atomic_store(&run_sniff , 1 );
                    pthread_create(&th_sniff , NULL , sniff , NULL) ;
                }
                break ;
                case 'q' :
                {
                    check_active_thread() ;
                    *back=0 ;
                    return (void*)back ;
                }
                break ;
            }
        }
        fflush(stdout) ;
        usleep(10000); // 100ms de pause
    }
    return NULL;
}
