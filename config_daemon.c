#include<stdio.h>
#include<stdlib.h>
#include<sys/inotify.h>
#include<unistd.h>
#include<limits.h>
#include<errno.h>
#include<string.h>
#include<pthread.h>
#include<stdatomic.h>
#include<signal.h>
#include<time.h>
#include <gtk/gtk.h>
#include"../en_tete/config_daemon.h"
#include"../en_tete/cpu.h"
#include"../en_tete/material.h"
#include"../en_tete/define.h"
#include"../en_tete/ram.h"
#include"../en_tete/authentif.h"
#include"../en_tete/input.h"

extern atomic_int run_cpu , run_ram , run_material , run_network ;


seuil limite ={70 , 50 ,60 , 40 , 50 , 1 , 'n','n','n','n','n','n'}; 

void print_mesage(char*message , char*type , char* color_type , int simple)
{
    time_t temps;
    struct tm * date ;
    FILE* log_file=fopen("/var/log/monitor_track" , "a") ;
    if(log_file==NULL)
    {
        log_file=fopen("/var/log/monitor_track" , "w") ;            
    }
    time(&temps); //recuperation du temps actuelN
    date = localtime(&temps); //traduction en heure locale
    if(simple==0)
    {
        fprintf(log_file , "%s %02d/%02d/%02d - %02d:%02d:%02d %s %s %s %s %s\n" , BLEU , date->tm_mday, date->tm_mon + 1, date->tm_year % 100, date->tm_hour, date->tm_min, date->tm_sec, NEUTRE  , color_type , type , NEUTRE , message ) ;
    }
    else
    {
        fprintf(log_file , "%s %02d/%02d/%02d - %02d:%02d:%02d %s %s\n" , BLEU , date->tm_mday, date->tm_mon + 1, date->tm_year % 100, date->tm_hour, date->tm_min, date->tm_sec, NEUTRE  , message ) ;
    }
    fclose(log_file);
}

void message_terminal(const char *m)
{
 printf("%s [!] ALERTE , %s",ROUGE,m);

}
void affichage_ecran(const char *p) 
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(
        NULL,
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_OK,
        "%s",p
    );

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

int d_terminal()
{ 
  char* tty = ttyname(STDIN_FILENO);
  int a; 
  if(tty == NULL)
   {
     //si on est dans l'interface graphique sans terminal
     a = 0;
   }
 else
   {
     //si on est dans l'interface graphique avec terminal ou dans tty
     a = 1;
   }
 return(a);
}

int create_config_file(char* filename)
{
    FILE* config=fopen("../donnees/config_copie" , "r");
    if(config==NULL)
    {
        printf("No file\n");
        exit(-1);
    }
    char line[200];
    FILE* real_config=fopen(filename , "w") ;
    while(fgets(line ,sizeof(line) , config)!=NULL)
    {
        fprintf(real_config , "%s" , line) ;
    }
    fclose(real_config) ;
    fclose(config);
    return 0 ;
}

seuil parse_fichier(char*filename)
{
    seuil limite1 ={70 , 50 ,90 , 40 , 50 , 1 , 'n','n','n','n','n','n'}; 
    FILE* config=fopen(filename , "r");
    if(config==NULL)
    {
        return limite1;
    }
    char line[200] ;

    while(fgets(line ,sizeof(line) , config)!=NULL)
    {
        if(strstr( line , "PLAFOND_COEUR_CPU=" )!=NULL && limite1.found_percent_core_cpu=='n') 
        {
            if(sscanf(line , "PLAFOND_COEUR_CPU=%d " , &limite1.percent_core_cpu)!=1)
            {
                limite1.percent_core_cpu=70 ;
            }
            limite1.found_percent_core_cpu='f' ;
        }
        else if(strstr( line , "PLAFOND_CPU_GENERAL" )!=NULL && limite1.found_percent_general_cpu=='n') 
        {
            if(sscanf(line , "PLAFOND_CPU_GENERAL=%d " , &limite1.percent_general_cpu)!=1)
            {
                limite1.percent_general_cpu=50 ;
            }
            limite1.found_percent_general_cpu='f' ;
        }
        else if(strstr( line , "PLAFOND_RAM=" )!=NULL && limite1.found_percent_ram=='n') 
        {
            if(sscanf(line , "PLAFOND_RAM=%d " , &limite1.percent_ram)!=1)
            {
                limite1.percent_ram=80 ;
            }
            limite1.found_percent_ram='f' ;
        }
        else if(strstr( line , "PLAFOND_SWAP=" )!=NULL && limite1.found_percent_swap=='n') 
        {
            if(sscanf(line , "PLAFOND_SWAP=%d " , &limite1.percent_swap)!=1)
            {
                limite1.percent_swap=40 ;
            }
            limite1.found_percent_swap='f' ;
        }
        else if(strstr( line , "PLAFOND_TEMPERATURE=")!=NULL && limite1.found_temperature=='n') 
        {
            if(sscanf(line , "PLAFOND_TEMPERATURE=%f " , &limite1.temperature)!=1)
            {
                limite1.temperature=50 ;
            }
            limite1.found_temperature='f' ;
        }
        else if(strstr( line , "INTERVALLE_TEMPS=")!=NULL && limite1.found_interval_time=='n') 
        {
            if(sscanf(line , "INTERVALLE_TEMPS=%d " , &limite1.interval_time)!=1)
            {
                limite1.interval_time=1 ;
            }
            limite1.found_interval_time='f' ;
        }
    }
    fclose(config) ; 
    return limite1 ; 
}

void* watch_config_file(void* arg)
{
    char message[100] ;
    char **argv = (char**) arg ;

    char filename[]="/etc/monitor_daemon" ;
    FILE* verifie=fopen(filename , "r") ;

    if(verifie==NULL)
    {
        create_config_file(filename) ;
    }

    if(verifie!=NULL)
    {
        fclose(verifie) ;
    }
    // Initialisation
    int fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) 
    {
        perror("inotify_init1\n");
        exit(EXIT_FAILURE);
    }

    // Ajouter un "watch" sur le fichier qui surveille si le fichier est modifier , supprimer ou deplacer
    int wd = inotify_add_watch(fd, filename , IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF);
    if (wd == -1) 
    {
        perror("inotify_add_watch");
        close(fd);
        exit(EXIT_FAILURE);
    }


    // Buffer pour les événements
    char buf[4096]
    __attribute__ ((aligned(__alignof__(struct inotify_event)))); //alignement
    const struct inotify_event *event;
    ssize_t len;
    char *ptr;
    // Boucle infinie
    while(1) 
    {
        len = read(fd, buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN) 
        {
            perror("read");
            break;
        }
        if (len<= 0) 
        {
            usleep(200000); // attendre un peu
            continue ;
        }

        for (ptr = buf; ptr< buf + len;
             ptr += sizeof(struct inotify_event) + event->len) 
        {

            event = (const struct inotify_event *) ptr;

            if (event->mask & IN_MODIFY  ) 
            {
                sprintf(message , "Fichier de configuration %s modifié!\n" , filename ) ;
                print_mesage(message , "NOTHING" , NEUTRE , 1) ; 
                limite=parse_fichier(filename) ;
            }
            else if (event->mask & IN_DELETE_SELF) 
            {
                sprintf(message , "Fichier de configuration %s supprime !\n" ,filename ) ;
                print_mesage(message , "NOTHING" , NEUTRE , 1) ;
                print_mesage("Relancement du demon ... (Cette operation peut duree plusieurs secondes)\n" , "NOTHING" , NEUTRE , 1) ; 
                execvp(argv[0] , argv) ; // On relance totalement le programme
                perror("execvp") ;
            }
        }
        sleep(limite.interval_time) ;
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    return NULL ;
}

                
void* get_save_info()
{
    long double consommation ;
    int i=0 , nb_coeur=core_number(),n; 
    char *ligne = calloc(LINE_SIZE , sizeof(char)) ;
    cpu_usage *cpu_info1=NULL , *cpu_info2=NULL ;
    char message[200] ;

    cpu_info1=calloc(nb_coeur+1,sizeof(cpu_usage)) ;
    cpu_info2=calloc(nb_coeur+1,sizeof(cpu_usage)) ;
    ram_info time1 ;

    while(1)
    {
        FILE* charge_cpu=fopen("/proc/stat" , "r")  ;
        fgets(ligne , LINE_SIZE , charge_cpu) ;
        get_info( ligne , &cpu_info1[0] , 0 ) ; 
        i=0;
        do            
        {
            i++ ; 
            fgets(ligne , LINE_SIZE , charge_cpu) ; 
            get_info( ligne , &cpu_info1[i] , 0 ) ; 
        }
        while(i<nb_coeur-1) ; 
        fclose(charge_cpu) ;

        sleep(1) ;

        charge_cpu=fopen("/proc/stat" , "r")  ;
        fgets(ligne , LINE_SIZE , charge_cpu) ;
        get_info( ligne , &cpu_info2[0] , 0 ) ; 

        i=0 ;
        do
        {
            i++ ; 
            fgets(ligne , LINE_SIZE , charge_cpu) ;
            get_info( ligne , &cpu_info2[i] , 0 ) ; 
        }
        while(i<nb_coeur-1) ; 
                    
        fclose(charge_cpu) ;
        for(i=0 ; i<nb_coeur+1 ; i++)
        {
            consommation=get_statistic( 0 , i , cpu_info1[i] , cpu_info2[i] , nb_coeur , i-1 , 0 , 0 , 0 ) ;
            if(consommation>90 && i !=0 )
              {
                n=d_terminal();
                if( n == 1 )
                 {
                  sprintf(message , "Coeur numero %d trop utilise : %.3Lf %% utilise\n" , i , consommation);
                  message_terminal(message);
                 }
                else
                  {
                   sprintf(message , "Coeur n° %d trop utilise : %.3Lf %% utilise\n" , i , consommation);
                   affichage_ecran(message);
                  }
              } 
            else if(consommation>limite.percent_core_cpu && i !=0 )
            {
                sprintf(message , "Coeur numero %d trop utilise : %.3Lf %% utilise\n" , i , consommation);
                print_mesage( message , " [ ! ] WARNING " , JAUNE , 0 ) ;
            }
            else if(consommation>limite.percent_general_cpu && i==0 )
            {
                sprintf(message , "CPU surrutilise : %.3Lf %% utilise\n" , consommation);
                print_mesage( message , " [ ! ] ALERT " , ROUGE , 0) ;

                n=d_terminal();
                if( n == 1 )
                 {
                  message_terminal(message);
                 }
                else
                  {
                   affichage_ecran(message);
                  }

                process* tete=get_heavy_proc_cpu(0 , 0 , 0 , 0) ;
                process *tmp = tete; 
                int count = 0;
                char name[50] , filename[100] ;
                print_mesage("PROCESSUS qui occupe trop le CPU :\n" , "NOTHING" , NEUTRE , 1) ; 
    
                while(tmp != NULL && count< 4) 
                {
                    // Afficher le pourcentage correctement
                    sprintf(filename , "/proc/%s/status" , tmp->pid) ;
                    FILE* fichier=fopen(filename ,"r");
                    if(fichier==NULL)
                    {
                        continue;
                    }
                    fscanf(fichier ,"Name: %s" , name);
                    fclose(fichier);
                    sprintf(message , "PID: %s => %s : %f %%", tmp->pid, name , tmp->cpu_percent_use);
                    print_mesage(message , "NOTHING" , NEUTRE , 1 ) ; 
                    tmp = tmp->next; 
                    count++;
                }
                free_process_list(tete) ;
            }
        }
        get_ram_state(&time1 ) ;
        float used = ((time1.memtotal - time1.memavailible) / time1.memtotal) * 100;
        if(used>90 && i !=0 )
        {
                n=d_terminal();
                if( n == 1 )
                 {
                  sprintf(message , "RAM saturee %.3f %% utilise\n" , used);
                  message_terminal(message);
                 }
                else
                  {
                   sprintf(message , "RAM saturee %.3f %% utilise\n" , used) ;
                   affichage_ecran(message);
                  }
        } 
        else if(used > limite.percent_ram)
        {
            sprintf(message , "RAM saturee %.3f %% utilise\n" , used) ;
            print_mesage( message , "[ ! ] WARNING " , JAUNE , 0 ) ;
            process* first = get_heavy_proc(0, 0 , 0 , 0 ) ;
            process *tmp = first; 
            int count = 0;
            print_mesage("PROCESSUS gourmands en  RAM :\n" , "NOTHING" , NEUTRE , 1) ; 
    
            while(tmp != NULL && count< 3) 
            { // Limiter à 10 processus
                sprintf(message , "PID: %s => %s : %u Mo", tmp->pid, tmp->name, tmp->ram_vmrss); 
                print_mesage(message , "NOTHING" , NEUTRE , 1);
                tmp = tmp->next; 
                count++;
            }
            free_process_list(first); // Libérer la mémoire après utilisation
        }
        used = ((time1.swaptotal - time1.swapfree) / time1.swaptotal) * 100; 
        if(used > limite.percent_swap)
        {
            char message[100] ;
            sprintf(message , "SWAP saturee %.3f %% utilise\n" , used) ;
            print_mesage( message , " [ ! ] WARNING " , JAUNE , 0 ) ;
        }
        get_temperature(0 , 0 , 0 ,  limite.temperature) ;
        sleep(limite.interval_time) ;
    }

 
    free(ligne) ;
    free(cpu_info1) ; 
    free(cpu_info2) ;
    return NULL ; 
}

void create_daemon(char**argv)
{
    long int pid ;
    pthread_t watch , analyse ; 
    sleep(10);
    pid=fork() ;//Creation d'un processus fils
    if(pid<0)
    {
        printf("Processus fils non cree cause possible :\n") ;
        perror("fork") ;
        return;
    }
    if(pid>0)
    {
        exit(0) ; // On  tue le processus pere
    }
    if(pid==0) 
    {
        setsid() ; //Creation  d'une nouvelle sesion pour detacher le processus du terminal
        freopen("/dev/null" , "w" , stdout) ; //redirection des sorties vers /dev/null
        pthread_create(&watch , NULL , watch_config_file , (void*)argv) ;
        pthread_create(&analyse , NULL , get_save_info , NULL) ;
        pthread_join(watch , NULL) ;
        pthread_join(analyse , NULL) ;
    }
}

