#include<stdio.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<pthread.h>
#include<stdatomic.h>
#include<sys/wait.h>
#include "../en_tete/menu.h"
#include"../en_tete/interface.h"
#include"../en_tete/define.h"
#include"../en_tete/material.h"
#include"../en_tete/cpu.h"
#include"../en_tete/ram.h"
#include"../en_tete/pid_info.h"
#include"../en_tete/input.h"
#include"../en_tete/lim2.h"

pthread_t great_input ;
extern pthread_t th_affiche_mat ;
extern atomic_int run_material ; 


void daemon_menu()
{
	char pattern_cpu[9][70] = {"┌─┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴─┐","┤                    ├","┤                    ├" ,"┤                    ├" ,"┤                    ├" ,"┤                    ├", "┤                    ├" ,"└─┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬─┘"} ;
	int hauteur_ecran , largeur_ecran ;
	get_screen_size(&largeur_ecran , &hauteur_ecran) ;
	int pid , choice , i ;
	char car ;
	char return_name[50] = "NULL";
	begin:
	efface_ecran() ;
	printf(VERT) ;
	for(i=0 ; i<8 ; i++)
	{
		bouge_curseur((hauteur_ecran/2)-4+i  , (largeur_ecran/2)-10) ;
		printf("%s" , pattern_cpu[i]) ;
	}
	printf(NEUTRE) ;
	bouge_curseur((hauteur_ecran/2)-3  , (largeur_ecran/2)-8) ;
	printf(" 1 > LANCEMENT") ;
	bouge_curseur((hauteur_ecran/2)-2  , (largeur_ecran/2)-8) ;
	printf(" 2 > HISTORIQUE") ;
	bouge_curseur((hauteur_ecran/2)-1  , (largeur_ecran/2)-8) ;
	printf(" 3 > NETTOYAGE") ;
	bouge_curseur((hauteur_ecran/2)  , (largeur_ecran/2)-8) ;
	printf(" 4 > CONFIGURATION") ;
	bouge_curseur((hauteur_ecran/2)+1  , (largeur_ecran/2)-8) ;
	printf(" 5 > RETOUR") ;
	bouge_curseur((hauteur_ecran/2)+2  , (largeur_ecran/2)-8) ;
	printf(" 6 > QUITTER") ;
	dessine_boite((largeur_ecran/2)-4 , (hauteur_ecran/2)+5  , 2 , 9 , BLANC) ; 
	bouge_curseur((hauteur_ecran/2)+6  , (largeur_ecran/2)) ;

	fflush(stdout) ;
	if(scanf("%d%c" , &choice , &car )!=2 || car!='\n')
	{
		while(getchar()!='\n') ;
		goto begin ;
	}

	switch (choice) 
	{
		case 1 : 
		{
			pid=search_process("monitor_daemon" , 0 , return_name) ;
			if(pid!=-1)
			{
				back:
				efface_ecran() ;
				printf(ROUGE) ;
				print_right_arrow(1 , 1 , "") ;
				printf(NEUTRE) ;
				bouge_curseur(1 , 13) ;
				printf("Le demon est deja en cours d'execution(Les journals sont /var/log/monitor_track) \n");
				bouge_curseur(6 , 13) ;
				printf("[  Entrer 1 pour le tuer et 0 pour quitter ] : ");
				if(scanf("%d%c" , &choice , &car)!=2 || car != '\n')
				{
					goto back ;
				}
				if(choice==1)
				{
					printf("Terminaison du demon avec le pid : %d\n" , pid) ;
					if(kill((pid_t)pid , SIGKILL)<0)
					{
						printf("Demon non arrete \n");
						perror("kill") ;
						return;
					}
					printf("Demon arrete avec succes\n") ;
					return ;
				}
				else
				{
					return;
				}
			}
			efface_ecran() ;
			pid_t pid_ = fork() ;
			if(pid_==0)
			{
				char* argv[]={"./monitor_daemon" , NULL} ;
				if(execvp(argv[0], argv)<0)
				{
					printf("DEMON non cree cause possible : \n");
					perror("execvp");
					return;
				}
			}
			else 
			{
				int status ;
				int largeur_ecran , hauteur_ecran , i=0 ;
				get_screen_size(&largeur_ecran , &hauteur_ecran) ;
				dessine_boite((largeur_ecran/2)-21 , (hauteur_ecran/2)-1 , 2 , 42 , BLANC )  ;
				bouge_curseur((hauteur_ecran/2)-2 , (largeur_ecran/2)-18) ;
				printf("Lancement du demon...") ;
				while(1)
				{
					pid_t result=waitpid(pid , &status , WNOHANG) ;
					if(i>(largeur_ecran/2)-20)
					{
						goto jump ;
					}
					for(i=(largeur_ecran/2)-20 ; i<=(largeur_ecran/2)+15 && result==0; i++)
					{
						bouge_curseur(hauteur_ecran/2 , i) ;
						printf("%s\u2588%s" , VERT , NEUTRE) ;
						fflush(stdout) ;
						usleep(500000) ;
					}
					jump:
					if(result==pid_)
					{
						for(int j=i ; j<=(largeur_ecran/2)+20 ; j++)
						{
							bouge_curseur(hauteur_ecran/2 , j) ;
							printf("%s\u2588%s" , VERT , NEUTRE) ;
							fflush(stdout) ;
						}
						bouge_curseur(hauteur_ecran-4 , 1) ;
						printf("Demon lance avec succes\n") ;
						break ;
					}					
				}

			}
			wait(NULL) ;
		}
		break ;
		case 2 :
		{
			find_in_history() ;
		}
		break ;
		case 3 :
		{
			clean_history() ;
		}
		break ;
		case 4 :
		{
			pid_t nano = fork() ;
			if(nano < 0 )
			{
				perror("fork") ;
				return ;
			}
			else if(nano == 0 )
			{
				execlp("nano" , "nano" , "/etc/daemon_monitor" , NULL) ;
			}
			else 
			{
				wait(NULL) ;
			}
		}
		break ;
		case 5:
		{
			affiche_menu() ;
		}
		break ;
		case 6:
		{
			return ;
		}
		break ;
		default :
		{
			goto begin ;
		}
		break ;
	}
}

void affiche_menu()
{
	int choix , largeur_ecran , hauteur_ecran ; 
	char car ; 
	get_screen_size(&largeur_ecran , &hauteur_ecran) ;
	debut :
	efface_ecran() ; 
	affiche_au_centre( (hauteur_ecran/2)-9 , "*** MONITOR TRACK ***" , VERT ) ; 
	dessine_boite((largeur_ecran/2)-(LARGEUR_MENU/2)-1 , (hauteur_ecran/2)-11 , HAUTEUR_MENU+5 , LARGEUR_MENU+2 , BLANC ) ;
	dessine_boite((largeur_ecran/2)-(LARGEUR_MENU/8)-1 , (hauteur_ecran/2)+(HAUTEUR_MENU/2)+2 , 6 , (LARGEUR_MENU/4)+2 , BLANC ) ;

	dessine_boite((largeur_ecran/2)-(LARGEUR_MENU/2)-(LARGEUR_MENU/4)-3 , (hauteur_ecran/2)-(HAUTEUR_MENU/2)+3 , HAUTEUR_MENU+4 , (LARGEUR_MENU/4) , BLANC ) ;

	dessine_boite((largeur_ecran/2)-(LARGEUR_MENU/2) , (hauteur_ecran/2)-10 , 2 , LARGEUR_MENU , VERT ) ;  
	dessine_boite((largeur_ecran/2)-(LARGEUR_MENU/2) , (hauteur_ecran/2)-(HAUTEUR_MENU/2) , HAUTEUR_MENU , LARGEUR_MENU , JAUNE) ;  
	affiche_au_centre( (hauteur_ecran/2)-4 ,"[1] Monitoring systeme temps reel" , BLANC ) ; 
	affiche_au_centre( (hauteur_ecran/2)-2 ,"[2] Suivi d'un processus specifique (PID)" , BLANC ) ; 
	affiche_au_centre( (hauteur_ecran/2) ,"[3] Demon systeme " , BLANC ) ; 
	affiche_au_centre( (hauteur_ecran/2)+2 ,"[4] A propos" , BLANC ) ; 
	affiche_au_centre( (hauteur_ecran/2)+4 ,"[5] Quitter" , BLANC ) ;
	affiche_au_centre( (hauteur_ecran/2)+(HAUTEUR_MENU/2)+4 , "VOTRE CHOIX : " , VERT ) ; 
	reset() ;

	if (scanf("%d%c" , &choix , &car)!=2 || car != '\n')
	{
		efface_ecran() ;
		bouge_curseur(hauteur_ecran/2 ,(largeur_ecran-strlen("Choix invalide\n"))/2 ) ;
		printf("%sChoix invalide\n%s" , ROUGE , NEUTRE) ;
		sleep(1) ;
		while(getchar() != '\n') ;  
		goto debut ;
	}
	switch (choix)
	{
		case 1 : 
		{
    		void *result ;
			int temp ;
			configuration();
			efface_ecran() ;
			atomic_store(&run_material , 1 ) ;
            pthread_create(&great_input , NULL , get_input , NULL )  ;
            pthread_create(&th_affiche_mat , NULL , get_material_overview , NULL )  ;
			pthread_join(great_input , (void**)&result) ;
			reset();
    		temp=*(int*)result ;
    		free(result);
    		if(temp==1)
    		{
        		affiche_menu() ;
    		}
		}
		break ;
		case 2 : 
		{
			efface_ecran() ;
			if(execute_pid_thread()==1)
			{
				efface_ecran() ;
				affiche_menu() ;
			}
		}
		break ; 
		case 3 : 
		{
			if(getuid()!=0)
			{
				efface_ecran() ;
				bouge_curseur(hauteur_ecran/2 ,(largeur_ecran-strlen("Seul l'administrateur systeme peut acceder au demon systeme \n"))/2) ;
				printf("%sSeul l'administrateur systeme peut acceder au demon systeme \n%s" , ROUGE , NEUTRE) ;
				fflush(stdout) ;
				sleep(1) ;
				goto debut;
			}
			daemon_menu();
		}
		break ;
		case 4 : 
		{
			print_readme_file() ; 
		}
		break ;
		case 5 : 
		{
            efface_ecran() ;
			exit(0) ;
		}
		break ;
		default : 
		{
			efface_ecran() ;
			bouge_curseur(hauteur_ecran/2 ,(largeur_ecran-strlen("Choix invalide\n"))/2 ) ;
			printf("%sChoix invalide\n%s" , ROUGE , NEUTRE) ; 
			sleep(1) ;
			goto debut ; 
		}
		break ; 
	} 
	bouge_curseur(hauteur_ecran , largeur_ecran) ; 
}



































