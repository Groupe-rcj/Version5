#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<dirent.h>
#include<pthread.h>
#include<time.h>
#include<stdatomic.h>
#include"../en_tete/define.h"
#include"../en_tete/cpu.h"
#include"../en_tete/interface.h"
#include"../en_tete/material.h"
#include"../en_tete/pid_info.h"

extern atomic_int run_cpu;

int core_number()
{
    int compteur=0 ; 
    char* ligne=NULL ; 
    FILE* charge_cpu=NULL ;
    charge_cpu=fopen("/proc/stat" , "r")  ;
    if(charge_cpu==NULL)
    {
        return 1 ;
    }
    ligne=malloc(LINE_SIZE) ; 
    do
    {
        fgets(ligne , LINE_SIZE , charge_cpu) ;
        if(strstr( ligne , "cpu" ))
        {
            compteur++ ; 
        }
        else
        {
            break ; 
        }
    }
    while(feof(charge_cpu)==0) ; 
    fclose(charge_cpu) ;
    free(ligne) ;
    return compteur ; 
}

void get_info( char* ligne , cpu_usage *cpu_info , int i )
{
    char cpu[10] ;
    sscanf( ligne , "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu " , 
        cpu ,
        &cpu_info[i].user_consommation , 
        &cpu_info[i].modified_nice , 
        &cpu_info[i].system_consommation , 
        &cpu_info[i].inactivity , 
        &cpu_info[i].iowait , 
        &cpu_info[i].irq , 
        &cpu_info[i].soft_irq , 
        &cpu_info[i].stoled_hyp , 
        &cpu_info[i].vm , 
        &cpu_info[i].nice_vm ) ;
}


void affiche_en_tete(int decalage)
{
    int i ; 
    char user[13][20]={" CPU(%)" , "Apk" , "LowP" ,  "Noyau" , "Rest" , " Disk" , "IRQ" , "SIRQ" , "VM" , "VMUsr" , "VMNice" , "Busy" , " Freq"} ; 
    for(i=0 ; i<13 ; i++)
    {
        bouge_curseur(2 , (i*decalage )+1) ; 
        printf("%s" , user[i]) ; 
    }
}

void print_cpu(int cpu_number , int y , double_cpu_usage cpu , long double consommation , float* frq , int decalage)
{
    number num ;
    bouge_curseur( y ,  2 ) ;
    if(cpu_number==-1)
    {
        printf("CPU*") ; 
    }
    else
    {
        printf(" CPU%d" , cpu_number) ;
    }
    bouge_curseur( y ,  decalage+1 ) ;
    num.lo=cpu.user_consommation ; 
    print_colored(&num , 'l' ,  70 , 40 , ROUGE , JAUNE , VERT) ; 

    bouge_curseur( y , (2*decalage)+1 ) ; 
    num.lo=cpu.modified_nice ; 
    print_colored(&num , 'l' ,  70 , 40 , ROUGE , JAUNE , VERT) ; 

    bouge_curseur( y , (3*decalage)+1 ) ;
    num.lo=cpu.system_consommation ; 
    print_colored(&num , 'l' ,  70 , 40 , ROUGE , JAUNE , VERT) ;

    bouge_curseur( y , (4*decalage)+1 ) ;
    num.lo=cpu.inactivity ; 
    print_colored(&num , 'l' ,  70 , 40 , VERT , JAUNE , ROUGE) ;

    bouge_curseur( y , (5*decalage)+1 ) ;
    num.lo=cpu.iowait ; 
    print_colored(&num , 'l' ,  70 , 40 , ROUGE , JAUNE , VERT) ;

    bouge_curseur( y , (6*decalage)+1 ) ;
    num.lo=cpu.irq ; 
    print_colored(&num , 'l' ,  70 , 40 , ROUGE , JAUNE , VERT) ;

    bouge_curseur( y , (7*decalage)+1) ;
    num.lo=cpu.soft_irq ; 
    print_colored(&num , 'l' ,  70 , 40 , ROUGE , JAUNE , VERT) ;

    bouge_curseur( y , (8*decalage)+1 ) ;
    num.lo=cpu.stoled_hyp ; 
    print_colored(&num , 'l' ,  70 , 40 , ROUGE , JAUNE , VERT) ;

    bouge_curseur( y , (9*decalage)+1  ) ;
    num.lo=cpu.vm ; 
    print_colored(&num , 'l' ,  70 , 40 , ROUGE , JAUNE , VERT) ;

    bouge_curseur( y , (10*decalage)+1  ) ;
    num.lo=cpu.nice_vm ; 
    print_colored(&num , 'l' ,  70 , 40 , ROUGE , JAUNE , VERT) ;

    bouge_curseur( y , (11*decalage)+1  ) ;
    num.lo=consommation ; 
    print_colored(&num , 'l' ,  70 , 40 , ROUGE , JAUNE , VERT) ;

    bouge_curseur( y , (12*decalage)+1  ) ;
    num.lo=frq[cpu_number] ; 
    print_colored(&num , 'l' ,  70 , 40 , ROUGE , JAUNE , VERT) ;
    
}

long double get_statistic(int print , int i  , cpu_usage time1 , cpu_usage time2 , int nb_coeur , int cpu_number , int y , float* frq , int decalage )
{
    long double total1 ,total2 , total1_activity1 , total2_activity2 , consommation ; 
    double_cpu_usage diff ; 
    if(i==0)
    {
        time1.user_consommation/=nb_coeur ; 
        time2.user_consommation/=nb_coeur ;  
        time1.system_consommation/=nb_coeur ; 
        time2.system_consommation/=nb_coeur ; 
        time1.inactivity/=nb_coeur ;
        time2.inactivity/=nb_coeur ;
        time1.iowait/=nb_coeur ; 
        time2.iowait/=nb_coeur ; 
        time1.irq/=nb_coeur ; 
        time2.irq/=nb_coeur ; 
        time1.soft_irq/=nb_coeur ; 
        time2.soft_irq/=nb_coeur ; 
        time1.stoled_hyp/=nb_coeur ; 
        time2.stoled_hyp/=nb_coeur ; 
        time1.vm /=nb_coeur ; 
        time2.vm /=nb_coeur ; 
        time1.nice_vm /=nb_coeur ; 
        time2.nice_vm /=nb_coeur ; 
    }
    total1=time1.user_consommation + time1.modified_nice + time1.system_consommation + time1.inactivity + time1.iowait + time1.irq + time1.soft_irq + time1.stoled_hyp + time1.vm + time1.nice_vm ; 
    total2=time2.user_consommation + time2.modified_nice + time2.system_consommation + time2.inactivity + time2.iowait + time2.irq + time2.soft_irq + time2.stoled_hyp + time2.vm + time2.nice_vm ; 
    
    total1_activity1=time1.user_consommation + time1.modified_nice + time1.system_consommation + time1.irq + time1.soft_irq + time1.stoled_hyp + time1.vm + time1.nice_vm ; 
    total2_activity2=time2.user_consommation + time2.modified_nice + time2.system_consommation + time2.irq + time2.soft_irq + time2.stoled_hyp + time2.vm + time2.nice_vm ; 

    if(total2-total1<=0.0001)// pour eviter la division par 0 
    {
        consommation=0 ;
        memset(&diff ,0 , sizeof(diff)) ;
    }
    else
    {
        diff.user_consommation=(long double)(100*(time2.user_consommation-time1.user_consommation)/(total2-total1)) ; 
        diff.modified_nice=(long double)(100*(time2.modified_nice-time1.modified_nice)/(total2-total1)) ; 
        diff.system_consommation=(long double)(100*(time2.system_consommation-time1.system_consommation)/(total2-total1)) ; 
        diff.inactivity=(long double)(100*(time2.inactivity-time1.inactivity)/(total2-total1)) ; 
        diff.iowait=(long double)(100*(time2.iowait-time1.iowait)/(total2-total1)) ; 
        diff.irq=(long double)(100*(time2.irq-time1.irq)/(total2-total1)) ; 
        diff.soft_irq=(long double)(100*(time2.soft_irq-time1.soft_irq)/(total2-total1)) ;
        diff.stoled_hyp=(long double)(100*(time2.stoled_hyp-time1.stoled_hyp)/(total2-total1)) ; 
        diff.vm=(long double)(100*(time2.vm-time1.vm)/(total2-total1)) ; 
        diff.nice_vm=(long double)(100*(time2.nice_vm-time1.nice_vm)/(total2-total1)) ; 
        consommation=(long double)(100*(total2_activity2-total1_activity1)/(total2-total1)) ; 
    }

    if(print==1)
    {
        print_cpu(cpu_number , y , diff , consommation , frq , decalage ) ; 
    }
    return consommation ; 
}


process* create_process_cpu(char* pid , char* name , float cpu_consommation ) //creer une instance de process pour la liste chainee
{
    process* p =(process*)malloc(sizeof(process)) ;
    strncpy(p->name, name, sizeof(p->name)-1);
    p->name[sizeof(p->name)-1] = '\0';
    strcpy(p->pid ,pid) ;
    p->cpu_percent_use=cpu_consommation ; 
    p->next=NULL ;  
    return p ;  
}

void add_element(process** tete , process* nv ) //ajout d'element a la liste chainee
{
    process* temp ;
    if(*tete == NULL)
    {
        *tete=nv ;
        return ;
    }
    temp = *tete ;
    while(temp->next!=NULL)
    {
        temp=temp->next ;
    }
    temp->next = nv ;
}

void print_process_cpu(process* tete, int y, int x, int hauteur) 
{
    process *tmp = tete; 
    int count = 0;
    char name[50] , filename[100] ;
    bouge_curseur(y, x);
    printf("PROCESSUS GOURMANDS : "); 
    
    while(tmp != NULL && count < 15 && y < hauteur - 2) 
    {
        y++;
        bouge_curseur(y, x);
        sprintf(filename , "/proc/%s/status" , tmp->pid) ;
        FILE* fichier=fopen(filename ,"r");
        if(fichier==NULL)
        {
            tmp = tmp->next;
            count++;
            continue;
        }
        fscanf(fichier , "Name: %s" , name);
        fclose(fichier);
        printf("PID: %s => %s : %f %%                  ", tmp->pid, name , tmp->cpu_percent_use); 
        tmp = tmp->next; 
        count++;
    }
    fflush(stdout); 
}

void insert_process_by_cpu(process* tete1, process* tete2, process** tete , unsigned int total) 
{
    process *tmp1 = tete1;
    
    // Vérifications de sécurité
    if (tete1 == NULL || tete2 == NULL || tete == NULL) 
    {
        return;
    }
    
    while(tmp1 != NULL) 
    {
        // Chercher le même PID dans la deuxième liste
        process *tmp2 = tete2;
        while(tmp2 != NULL && strcmp(tmp1->pid, tmp2->pid) != 0) 
        {
            tmp2 = tmp2->next;
        }
        
        // Si trouvé et consommation positive
        if(tmp2 != NULL && total !=0 ) 
        {
            float cpu_consommation = (float)(tmp2->cpu_percent_use - tmp1->cpu_percent_use)/(float)total;
            cpu_consommation*=100 ;
            // Ne garder que les processus avec consommation positive
            if(cpu_consommation > 0) 
            {
                process *nv = create_process_cpu(tmp1->pid, tmp1->name, cpu_consommation);
                
                if (nv != NULL) 
                {
                    // Insertion en tête si liste vide ou plus grande consommation
                    if (*tete == NULL || cpu_consommation > (*tete)->cpu_percent_use) 
                    {
                        nv->next = *tete;
                        *tete = nv;
                    }
                    else 
                    {
                        // Insertion triée
                        process *courant = *tete;
                        process *precedent = NULL;
                        
                        while (courant != NULL && courant->cpu_percent_use > cpu_consommation) 
                        {
                            precedent = courant;
                            courant = courant->next;
                        }
                        
                        if (precedent != NULL) 
                        {
                            precedent->next = nv;
                        }
                        nv->next = courant;
                    }
                }
            }
        }
        tmp1 = tmp1->next;
    }
}

process* get_heavy_proc_cpu(int hauteur , int x , int y , int print)
{
    process* tete1=NULL , *tete2=NULL , *aligned=NULL ;
    unsigned long long int total1 = get_total_cpu() , total2 ;
    struct dirent *entry ;
    process_cpu info1 ;
    DIR* directory ;
    
    // Première lecture
    directory=opendir("/proc") ;
    if(directory==NULL) 
    {
        return NULL ;
    }
    total1=get_total_cpu() ;
    while((entry=readdir(directory))!=NULL)
    {
        if(is_number(entry->d_name))
        {
            unsigned long int time1=get_cpu_of_pid(atoi(entry->d_name) , &info1 , 0) ;
            process* new=create_process_cpu(entry->d_name , entry->d_name , (unsigned int)time1 );
            if(new != NULL) 
            {
                add_element(&tete1 , new) ;
            }
        }
    }
    closedir(directory);
    
    sleep(1) ;
    total2=get_total_cpu() ;
    // Deuxième lecture
    directory=opendir("/proc") ;
    while((entry=readdir(directory))!=NULL)
    {
        if(is_number(entry->d_name))
        {
            unsigned long int time1=get_cpu_of_pid(atoi(entry->d_name) , &info1 , 0 ) ;
            process* new=create_process_cpu(entry->d_name , entry->d_name , (unsigned int)time1 );
            if(new != NULL) 
            {
                add_element(&tete2 , new) ;
            }
        }
    }
    closedir(directory);
    
    // Trier par consommation CPU
    insert_process_by_cpu(tete1 , tete2 , &aligned , (unsigned int)(total2-total1)) ;
    
    if(print==1)
    {
        // Afficher la liste triée (aligned), pas tete1!
        print_process_cpu(aligned , y , x , hauteur) ;
    }
    
    free_process_list(tete1);
    free_process_list(tete2);
    return aligned ;
}


void get_frq(int nb_coeur , float* frq)
{
    int i ; 
    FILE* fichier ; 
    char* filename=NULL , *ligne=NULL; 
    filename=malloc(100) ; 
    ligne=malloc(25) ; 

    for(i=0 ; i<nb_coeur ; i++)
    {
        sprintf(filename , "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq" , i ) ;
        fichier=fopen(filename , "r") ;
        if(fichier==NULL)
        {
            frq[i]=0.0f ;
            continue; 
        }
        if(fgets(ligne , 25 , fichier))
        {
            sscanf(ligne , "%f" , &frq[i]) ;
            frq[i]/=(1000*1000) ; 
        } 
        else
        {
            frq[i]=0.0f ;
        }
        fclose(fichier) ;       
    }
    free(filename) ;
    free(ligne) ; 
}

void print_load_avg()
{
    FILE* fichier ; 
    float minute1 , minute5 , minute15 ;
    int last_pid , running_process , all_proceses ; 
    char* filename=NULL , *ligne=NULL; 
    filename=malloc(100) ; 
    ligne=malloc(50) ; 
    sprintf(filename , "/proc/loadavg") ;
    fichier=fopen(filename , "r") ;
    if(fichier==NULL)
    {
        return; 
    }
    fgets(ligne , 50 , fichier) ; 
    fclose(fichier) ;  
    sscanf(ligne , "%f %f %f %d/%d %d" , &minute1 , &minute5 , &minute15 , &running_process , &all_proceses , &last_pid) ;
    free(filename) ;
    free(ligne) ;  
    printf("Charge moyenne du CPU (1min) : %.2f\n" , minute1) ;
    printf("Charge moyenne du CPU (5min) : %.2f\n" , minute5) ;  
    printf("Charge moyenne du CPU (15min) : %.2f\n" , minute15) ;  
    printf("Processus en execution : %d sur %d\n" , running_process , all_proceses ) ; 
    printf("Dernier processus cree : %d\n" , last_pid) ; 
}


void* get_cpu_state()
{
    int i , nb_coeur , largeur_ecran , hauteur_ecran , largeur , decalage ;
    float *frq=NULL ; 
    char* ligne = NULL ; 
    FILE* charge_cpu=NULL ;
    cpu_usage *cpu_info1=NULL , *cpu_info2=NULL ;
    process* tete ;

    nb_coeur=core_number() ; 
    frq=malloc(nb_coeur*sizeof(float)) ; 

    ligne=calloc(LINE_SIZE , sizeof(char)) ; 

    cpu_info1=calloc(nb_coeur+1,sizeof(cpu_usage)) ;
    cpu_info2=calloc(nb_coeur+1,sizeof(cpu_usage)) ; 

    get_screen_size(&largeur_ecran, &hauteur_ecran) ; 
    efface_ecran() ; 
    get_array_size(&largeur , &decalage) ; 
    print_array( (2*nb_coeur )+3, largeur , decalage ) ; 
    affiche_en_tete(decalage) ; 

    configuration() ;
    time_t last_refresh=time(NULL) ;
    print_left_arrow(hauteur_ecran-6 , largeur_ecran-20 , "MATERIAL") ; 
    print_right_arrow(hauteur_ecran-6 , largeur_ecran-10 ,"RAM") ;

    while(atomic_load(&run_cpu))
    {
        time_t now = time(NULL) ;
        if(difftime(now,last_refresh)>=1.0)
        {
            charge_cpu=fopen("/proc/stat" , "r")  ;
            fgets(ligne , LINE_SIZE , charge_cpu) ;
            get_info( ligne , &cpu_info1[0] , 0  ) ; 
            tete=get_heavy_proc_cpu(hauteur_ecran , largeur/2 , (2*nb_coeur )+7 , 1 ) ;
            free_process_list(tete) ;
            i=0 ; 
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
            get_frq(nb_coeur , frq) ; 
            for(i=0 ; i<nb_coeur ; i++)
            {
                get_statistic( 1 , i , cpu_info1[i] , cpu_info2[i] , nb_coeur , i-1 , (2*i)+4 , frq , decalage ) ; 
            }

            bouge_curseur((2*nb_coeur )+5 , 1) ;
            bouge_curseur((2*nb_coeur )+7 , 1) ;
            print_load_avg() ; 
            bouge_curseur(hauteur_ecran , largeur_ecran ) ;
            last_refresh=now ;
            printf("\033[%d;%dH Taper q pour quitter" , hauteur_ecran-1 , 1 )  ;
        }
        fflush(stdout);
        usleep(500000) ;
    }
    free(ligne) ; 
    free(cpu_info1) ; 
    free(cpu_info2) ; 
    return NULL ;
}

