#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../en_tete/liste_proc.h"
#include "../en_tete/menu.h"
#include "../en_tete/interface.h"
#include "../en_tete/define.h"
#include <errno.h>
#include <sys/resource.h>
#include <signal.h>

// Fonction pour obtenir l'UID d'un processus
int get_process_uid(int pid) 
{
    char path[256];
    char buffer[1024];
    FILE *file;
    int uid = -1;
    
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    file = fopen(path, "r");
    if (file != NULL) 
    {
        while (fgets(buffer, sizeof(buffer), file) != NULL) 
        {
            if (strncmp(buffer, "Uid:", 4) == 0) 
            {
                sscanf(buffer, "Uid: %d", &uid);
                break;
            }
        }
        fclose(file);
    }
    return uid;
}


int affichage_proc(int ligne_max, char *file)
{
    FILE *f = fopen(file, "r");
    char *buffer = malloc(256);  
    int count = 1, choix = 0;
    int max_lines = ligne_max - 7;
    
    printf("=== Affichage des processus ===\n");
    
    while(fgets(buffer, 256, f) != NULL)
    {
        printf("%s", buffer);
        
        if (count >= max_lines)
        {
            printf("\n--- Fin de la page ---\n");
            printf("%s[ 0 ]%s - Page suivante\n" , BLEU , NEUTRE);
            printf("%s[ 1 ]%s - Suivre un processus\n" , VERT , NEUTRE);
            printf("%s[ 2 ]%s - Retour\n" , BLEU , NEUTRE) ;
            printf("Choix: ");
            
            if (scanf("%d", &choix) != 1) 
            {
                printf("Entrée invalide\n");
                clearerr(stdin);
                break;
            }
            
            if (choix == 1) 
            {
                free(buffer);
                fclose(f);
                return 1 ;
            }
            else if (choix == 0) 
            {
                count = 0;  // Reset pour nouvelle page
                printf("\n=== Page suivante ===\n");
            }
            else if (choix == 2) 
            {
                free(buffer) ;
                fclose(f) ;
                return 0 ;
            }
        }
        count++;
    }
    
    // Si on arrive ici, soit fin du fichier, soit choix = 1
    if (feof(f)) 
    {
        printf("\n--- Fin de la liste ---\n");
        printf("%s[ 1 ]%s - Suivre un processus\n" , VERT , NEUTRE);
        printf("%s[ 0 ]%s - Retour\n" , BLEU , NEUTRE);
        printf("Choix: ");
        if (scanf("%d", &choix) != 1) 
        {
            printf("Entrée invalide\n");
            clearerr(stdin);
            exit(0);
        }
        if (choix == 1) 
        {
            free(buffer);
            fclose(f) ; 
            return 1 ;
        }
        else 
        {
            efface_ecran() ;
            affiche_menu() ;
        }
    }
    
    free(buffer);
    fclose(f);
    return 0 ;
}

// Fonction pour obtenir le nom d'utilisateur depuis l'UID
char* uid_to_username(int uid) 
{
    struct passwd *pw = getpwuid(uid);
    char *username;
    
    if (pw != NULL) 
    {
        username = malloc(strlen(pw->pw_name) + 1);
        if (username != NULL) 
        {
            strcpy(username, pw->pw_name);
        }
    } 
    else 
    {
        username = malloc(16);
        if (username != NULL) 
        {
            snprintf(username, 16, "%d", uid);
        }
    }
    return username;
}

char* is_background(pid_t pid_c)
{
	char path[50] , ligne[256] ;
	sprintf(path, "/proc/%d/stat", pid_c) ;
	FILE *f = fopen(path, "r") ;
	fgets(ligne, sizeof(ligne), f) ;
	fclose(f) ;
	int tty = 0 , pgrp = 0 , tpgid = 0 ;
	sscanf(ligne, "%*d %*s %*s %*d %d %*d %d %d", &pgrp, &tty, &tpgid) ;
	if (tty == 0)
	{
		return ("NA") ;
	}
	else if (pgrp == tpgid)
	{
		return ("FG") ;
	}
	else
	{
		return ("BG") ;
	}
}

int change_priority(int pid, int n)
{
	if (setpriority(PRIO_PROCESS, pid, n) == 0)
	{
		printf("Priorité changer\n") ;
		return (1) ;
	}
	else
	{
		if (errno == EACCES)
		{
			printf("%s\n", strerror(errno)) ;
			return (0) ;
		}
		else
		{
			printf("Erreur de changement de priorité\n") ;
			return (0) ;
		}
	}
}

int symbole_to_int(char* signal)
{
	int num = 1 ;
	//Vérifie si signal est entièrement numérique
	for (int i = 0 ; signal[i] ; i++)
	{
		if (!isdigit((unsigned char)signal[i]))
		{
			num = 0 ;
			break ;
		}
	}
	if (num == 1)
	{
		int res = atoi(signal) ;
		if (res > 0 && res < NSIG)
		{
			return (res) ;
		}
		return (0) ;
	}
    
	char name[32] ;
	strncpy(name, signal, sizeof(name)-1) ;
	name[sizeof(name)-1] = '\0' ;

	//enlever SIG au début
	if (strncasecmp(name, "SIG", 3) == 0)
	{
		memmove(name, name + 3, strlen(name + 3) + 1) ;
	}
	
    liste_sig signals[] = {
        { SIGHUP, "HUP"  },
        { SIGINT,  "INT"  },
        { SIGQUIT, "QUIT" },
        { SIGILL,  "ILL"  },
        { SIGTRAP, "TRAP" },
        { SIGABRT, "ABRT" },
        { SIGBUS,  "BUS"  },
        { SIGFPE,  "FPE"  },
        { SIGKILL, "KILL" },
        { SIGUSR1, "USR1" },
        { SIGSEGV, "SEGV" },
        { SIGUSR2, "USR2" },
        { SIGPIPE, "PIPE" },
        { SIGALRM, "ALRM" },
        { SIGTERM, "TERM" },
        { SIGSTKFLT, "STKFLT" },
        { SIGCHLD, "CHLD" },
        { SIGCONT, "CONT" },
        { SIGSTOP, "STOP" },
        { SIGTSTP, "TSTP" },
        { SIGTTIN, "TTIN" },
        { SIGTTOU, "TTOU" },
        { SIGURG, "URG" },
        { SIGXCPU, "XCPU" },
        { SIGXFSZ, "XFSZ" },
        { SIGVTALRM, "VTALARM" },
        { SIGPROF, "PROF" },
        { SIGWINCH, "WINCH" },
        { SIGIO, "IO" },
        {  SIGPWR, "PWR" },
        { SIGSYS, "SYS" },
    } ;
    for (size_t i = 0; i < sizeof(signals)/sizeof(signals[0]); i++)
    {
        if (strncasecmp(name, signals[i].name, strlen(name)) == 0)
        {
            return signals[i].num;
        }
    }
    //essayer de comparer avec strsignal()
	for (int i = 1 ; i < NSIG ; i++)
	{
		char* desc = strsignal(i) ;
		if (desc && strncasecmp(signal, desc, strlen(signal)) == 0)
		{
			return (i) ;
		}
	}
	return (0) ;
}

int signal_pid(char* spid, int pid)
{
    int largeur , hauteur ;
    get_screen_size(&largeur , &hauteur) ;
	int n = symbole_to_int(spid) ;
    printf(ROUGE) ;
    if (n == 0)
    {
        bouge_curseur(hauteur/2 , (largeur-strlen("Signal invalide\n"))/2) ;
        printf("Signal invalide\n%s" , NEUTRE) ;
        return (-1) ;
    }
	if (kill(pid, n) == 0)
	{
        printf(VERT) ;
        bouge_curseur(hauteur/2 , (largeur-strlen("Signal envoye avec succès\n"))/2) ;
		printf("Signal envoye avec succès\n%s", NEUTRE) ;
		return (1) ;
	}
	else
	{
        bouge_curseur(hauteur/2 , (largeur-strlen(strerror(errno)))/2) ;
		if (errno == EINVAL)
		{
			printf("%s%s\n", strerror(errno) ,NEUTRE) ;
			return (-1) ;
		}
		else if (errno == EPERM)
		{
			printf("%s%s\n", strerror(errno) ,NEUTRE) ;
			return (-1) ;
		}
		else
		{
			printf("%s%s\n", strerror(errno) , NEUTRE) ;
			return (-1) ;
		}
	}
}

char* authentification()
{
		struct passwd *person ;
		long uid = getuid() ;
		person = getpwuid(uid) ;
		return person->pw_name ;
}
void list_processes_proc() 
{
    char* user_name ;
    user_name=malloc(50) ;
    user_name = authentification() ;
    struct passwd *good_user ; 
    good_user=getpwnam(user_name) ;
    int user_uid = good_user->pw_uid ;
    freopen("./liste_proc.txt", "w", stdout) ;
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir("/proc");
    if (dir == NULL) 
    {
        perror("opendir");
        return;
    }
    printf("%sPID\tUtilisateur\tF/B\tNom du processus%s\n" , BLEU , NEUTRE);
    
    printf("%s----------------------------------------%s\n" , BLEU , NEUTRE);
    
    while ((entry = readdir(dir)) != NULL) 
    {
        // Vérifier si le nom est un nombre (PID)
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) 
        {
            char path[268];
            char comm[268];
            FILE *file;
            int pid = atoi(entry->d_name);
            
            // Lire le nom du processus depuis /proc/pid/comm
            snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
            file = fopen(path, "r");
            if (file != NULL) {
                if (fgets(comm, sizeof(comm), file) != NULL) 
                {
                    // Supprimer le saut de ligne
                    comm[strcspn(comm, "\n")] = 0;
                    
                  // Obtenir l'UID du processus
                    int uid = get_process_uid(pid);

                    //verifier si le processus appartient a l'utilisateur courant
                    if (uid == user_uid)
                    {
                        char *username = uid_to_username(uid);
                        printf("%s\t%s\t\t%s\t%s\n", entry->d_name, username, is_background(pid), comm);
                        // Libérer la mémoire allouée
                        free(username);
                    }
                }
                fclose(file);
            }
        }
    }
    
    closedir(dir);
    //Restaurer stdout après freopen
    freopen("/dev/tty", "w", stdout);
}
