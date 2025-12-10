#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdatomic.h>
#include<pthread.h>
#include<time.h>
#include<dirent.h>
#include <signal.h>
#include <pwd.h>
#include"../en_tete/pid_info.h"
#include"../en_tete/menu.h"
#include"../en_tete/cpu.h"
#include"../en_tete/define.h"
#include"../en_tete/interface.h"
#include"../en_tete/fichier_ouvert.h"
#include"../en_tete/liste_proc.h"

atomic_int  running=1 ;
pthread_t th_pid_monitor , th_input_pid ;

char* get_process_owner(int pid)
{
    int uid ;
    char filename[50] , line[100] ;
    sprintf(filename , "/proc/%d/status" , pid) ;
    FILE* stat = fopen( filename , "r" ) ;
    if(stat==NULL)
    {
        printf("Le pid n'existe pas\n") ;
        return "UNKNOW" ;
    }
    while(fgets(line , sizeof(line) , stat)!=NULL)
    {
        if(strstr(line , "Uid")!=NULL)
        {
            sscanf(line , "Uid: %d %*d %*d %*d" , &uid) ;
            struct passwd *person = getpwuid(uid) ;
            return person->pw_name ; 
        }
    }
    return "Unknow" ;
}

int search_process(char*name , int pid , char *return_name)
{
    DIR* directory ;
    struct dirent *entry ;
    directory=opendir("/proc") ;
    int back=-1 ;
    if(directory==NULL)
    {
        printf("Repertoire /proc directory non ouvert \n");
        return back ;
    }
    entry=readdir(directory) ;
    while(entry!=NULL)
    {
        if(is_number(entry->d_name))
        {
            char file[270] ;
            sprintf(file , "/proc/%s/status" , entry->d_name) ;
            FILE* fichier=fopen(file , "r") ;
            if(fichier==NULL)
            {
                entry=readdir(directory) ;
                continue ;
            }
            char line[50] , original_name[50] ;
            fgets(line , sizeof(line) , fichier) ;
            sscanf(line , "Name: %s" , original_name) ;
            fclose(fichier) ;
            if(atoi(entry->d_name) == pid)
            {
                strcpy( return_name , original_name) ;
            }
            if(strcmp(name , original_name)==0)
            {
                back = atoi(entry->d_name) ;
            }
        }
        entry=readdir(directory) ;
    }
    closedir(directory);
    return back ;
}


unsigned long long int  get_total_cpu()
{
    char ligne[300] ;
    cpu_usage *cpu_info ;
    cpu_info=malloc(2*sizeof(cpu_usage)) ; 

    FILE *fichier =fopen("/proc/stat" , "r") ; 
    if(fichier==NULL)
    {
        return 0 ;
    }
    fgets(ligne , sizeof(ligne) , fichier) ;
    fclose(fichier) ;

    get_info(ligne , cpu_info , 1 ) ;

    return cpu_info[1].user_consommation + 
        cpu_info[1].modified_nice + 
        cpu_info[1].system_consommation + 
        cpu_info[1].irq +
        cpu_info[1].soft_irq + 
        cpu_info[1].stoled_hyp + 
        cpu_info[1].vm + 
        cpu_info[1].nice_vm +
        cpu_info[1].iowait +
        cpu_info[1].inactivity ; 
}

unsigned long int  get_cpu_of_pid(int pid , process_cpu *info , int go_out)
{
    char filename[100] , line[256] ;
    sprintf(filename , "/proc/%d/stat" , pid) ;
    FILE* fichier=fopen(filename , "r") ; 
    if(fichier==NULL)
    {
        if(go_out==1)
        {
            efface_ecran();
            printf("Ce processus n'est plus disponible \n\n\n") ;
            reset() ;
            usleep(10000) ;
            exit(0) ;
        }
        return (0); 
    }
    fgets(line , sizeof(line) , fichier) ;
    fclose(fichier) ; 

    sscanf(line , "%*d %*s %s %d %d %d %*d %*d %*d %*d %*d %*d %*d %lu %lu %*d %*d %d %d %d %*d %llu" , 
        info->state , 
        &info->ppid , 
        &info->pgrp , 
        &info->psid , 
        &info->utime , 
        &info->stime ,
        &info->priority ,
        &info->nice ,
        &info->num_thread ,
        &info->time 
    ) ;
    return info->utime+info->utime ;
}

long double get_percentage_cpu(int pid , process_cpu *info2)
{
    long long unsigned int cpu_time_total1 , cpu_time_total2 ;
    unsigned long time1 , time2 ;
    long double percent ;
    process_cpu info1 ;

    cpu_time_total1=get_total_cpu() ; 
    time1=get_cpu_of_pid(pid , &info1 ,1 ) ;

    sleep(1) ; 
    cpu_time_total2=get_total_cpu() ; 
    time2=get_cpu_of_pid(pid , info2 ,1 ) ;

    if(cpu_time_total2-cpu_time_total1<=0.0001)
    {
        return 0 ;
    }
    if(time2 < time1 )
    {
        return 0 ;
    }
    percent=((long double)(time2-time1)/(long double)(cpu_time_total2-cpu_time_total1))*100 ;

    return percent ;
}

void print_title()
{
    bouge_curseur(3,1);
    printf("   ETAT : \n") ;
    printf("   PPID : \n") ;
    printf("   GROUP PROCESS ID : \n") ;
    printf("   SESSION PROCESS ID : \n") ;
    printf("   PRIORITY : \n") ;
    printf("   THREAD NUMBER :\n") ;
    printf("   DURATION (s) :\n") ;
    printf("   CPU USAGE : \n");
    printf("   MEMORY USAGE : \n\n");
    printf("   OPENED FILE : \n");
}

unsigned int get_process_ram(int pid)
{
    unsigned int ram ; 
    char filename[256] ;
    sprintf(filename , "/proc/%d/status" , pid ) ;
    FILE* fichier=fopen(filename ,"r") ; 
    if(fichier==NULL)
    {
        efface_ecran() ;
        printf("Ce processus n'est plus disponible \n\n\n") ;
        usleep(10000) ;
        reset() ;
        exit(0) ;
    }
    while(fgets(filename , sizeof(filename) , fichier)!=NULL)
    {
        if(strstr(filename , "VmRSS")!=NULL)
        {
            sscanf(filename , "VmRSS: %u" , &ram);
            return ram ;
        }
    }
    return 0 ;
}

void* trace_pid(void* pid)
{
    process_cpu info2 ; 
    int largeur , hauteur , *copy=(int*)pid , y=2 , len=strlen("   SESSION PROCESS ID : \n")+5 ; 
    get_screen_size(&largeur , &hauteur);
    time_t last_refresh =time(NULL) ;
    while(atomic_load(&running))
    {
        time_t now = time(NULL) ;
        if(difftime(now , last_refresh>=2.0))
        {
            efface_ecran();
            print_title() ;
            dessine_boite(1,1, 11 , len-2 , BLEU) ;
            dessine_boite(len , 1 , 11 , largeur-len , BLEU) ;
            print_opened_file(*copy , y+12);
            fflush(stdout);
            char return_name[50] ;
            search_process("NULL" , *copy , return_name) ;

            printf("\033[2;%dHPID : %d (%s)" , (largeur/2)-15 , *copy , return_name) ;
            printf("\033[%d;%dH    %s" , y+1 ,len+4 , info2.state) ;
            printf("\033[%d;%dH    %d" , y+2 ,len+4 , info2.ppid) ;
            printf("\033[%d;%dH    %d" , y+3 ,len+4 , info2.pgrp) ;
            printf("\033[%d;%dH    %d" , y+4 ,len+4 , info2.psid) ;
            printf("\033[%d;%dH    %d" , y+5 ,len+4 , info2.priority) ;
            printf("\033[%d;%dH    %d" , y+6 ,len+4 , info2.num_thread) ;
            printf("\033[%d;%dH    %lu sec",y+7 , len+4 ,  info2.utime/100) ;
            printf("\033[%d;%dH    %u octes",y+9 , len+4 , get_process_ram(*copy) ) ;
            printf("\033[%d;%dH q (quitter) / k(envoyer un signal) / n(moidfier la priorite)" , hauteur-1 , 1 )  ;
            fflush(stdout);
            printf("\033[10;%dH   %LF %%\n" , len+4 , get_percentage_cpu(*copy , &info2)) ;
            fflush(stdout);
            last_refresh=now ;
        }

        usleep(500000);
    }
    return NULL ;
}

void* wait_pid_input(void*pid)
{
    int *copy=pid , choix;
    char move, car;
    int *result=malloc(sizeof(int));
    while(atomic_load(&running))
    {
        if(kbhit()) {
            move = get_char();
            switch(move)
            {
                case 'n' : 
                {
                    atomic_store(&running,0) ;
                    pthread_join(th_pid_monitor , NULL) ;
                    reset() ;
                    debut:
                    efface_ecran() ;
                    printf("Entrer la priorite a donner au processus:") ;
                    if (scanf("%d%c" , &choix , &car)!=2 || car != '\n')
                    {
                        while(getchar() != '\n') ;  
                        goto debut ;
                    }
                    change_priority(*copy, choix) ;
                }
                break ;
                case 'q' : 
                {
                    atomic_store(&running,0) ;
                    pthread_join(th_pid_monitor , NULL) ;
                    reset() ;
                    efface_ecran() ;
                    *result=0 ;
                    return  result ;
                }
                break ; 
                case 'L' : 
                {
                    atomic_store(&running,0) ;
                    pthread_join(th_pid_monitor , NULL) ;
                    reset() ;
                    efface_ecran() ;
                    *result=1 ;
                    return  result ;
                }
                break ; 
                case 'k' :
                {
                    char sign[20] ;
                    atomic_store(&running , 0 ) ;
                    pthread_join(th_pid_monitor , NULL) ;
                    reset() ;
                    efface_ecran() ;
                    efface_ecran() ;
                    printf("Entrer le signal a envoyer a ce processus: ");
                    scanf("%s" , sign) ;
                    signal_pid(sign, *copy) ;
                    sleep(5) ;
                    *result=1 ;
                    return  result ;

                }
                break ;
            }
        }
        usleep(100000); // 100ms de pause
    }
    return result ;
}


int execute_pid_thread()
{
    int pid , largeur , hauteur , temp ; 
    void *result ;
    char car ;
    get_screen_size(&largeur , &hauteur) ;
    list_processes_proc();
    char file[]= "./liste_proc.txt" ;
    if(affichage_proc(hauteur , file)==0)
    {
        remove(file) ;
        return 1 ;
    }
    else
    {
        remove(file) ;
        debut:
        efface_ecran() ;
        printf("Entrer alors le PID : ");
        if (scanf("%d%c" , &pid , &car)!=2 || car != '\n')
        {
            while(getchar() != '\n') ;  
            goto debut ;
        }
        configuration() ;

        pthread_create(&th_pid_monitor , NULL , trace_pid , &pid) ;
        pthread_create(&th_input_pid  , NULL , wait_pid_input , &pid) ;

        pthread_join(th_input_pid  , (void**)&result) ;
        temp=*(int*)result ;
        free(result);
        if(temp==1)
        {
            affiche_menu() ;
        }        
    }
    return 0 ;
}