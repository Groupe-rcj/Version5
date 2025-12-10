#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"../en_tete/lim2.h"

/**
*@brief Recherche et affichage d'une ligne du mot saisi comme commende grep 
*/

void grep_like(const char *filename, const char *pattern) 
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) 
    {
        perror("fopen");
        return;
    }

    char line[MAX_LINE];
    int line_num = 0 , found=0 ;

    while (fgets(line, sizeof(line), file)) 
    {
        line_num++;
        if (strstr(line, pattern)) 
        { 
           // strstr() recherche la sous-chaîne
            found=1 ;
            printf("%s:%d: %s", filename, line_num, line);
        }
    }
    if(found==0)
    {
        printf("\nRIEN A AFFICHER\n") ;
    }
    fclose(file);
}

/** 
*@brief Fonction pour convertir "jj/mm/aa" → time_t
*/
time_t convertir_date(const char *date_str) 
{
    struct tm t = {0};
    int jour, mois, annee;
    if (sscanf(date_str, "%d/%d/%d", &jour, &mois, &annee) != 3)
    {
        return (time_t)-1;  // erreur de format
    }

    if (annee < 100) 
    {
        annee += 2000;
    }

    t.tm_mday = jour;
    t.tm_mon = mois - 1;
    t.tm_year = annee - 1900;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;

    return mktime(&t);
}

/**
*@brief suppression des ligne plus de 10 jour
*/
void nettoyer_fichier(const char *nom_fichier, int jours_a_garder) 
{
    FILE  *src = fopen(nom_fichier, "r");
    FILE *tmp = fopen("temp.txt", "w");
    if ( tmp == NULL ) 
    {
        perror("Erreur ouverture fichier1");
        exit(1);
    }
    if  ( src == NULL ) 
    {
        perror("Erreur ouverture fichier2");
        exit(1);
    }

    time_t maintenant = time(NULL);
    time_t limite = maintenant - (jours_a_garder * 24 * 60 * 60);//convertir en seconde "jours...60"

    char ligne[MAX_LINE];
    while (fgets(ligne, sizeof(ligne), src)) 
    {  
        char date_str[16];
        // Extraire la date du début 
        if (sscanf(ligne, "%*s %15[0-9/]", date_str) == 1) 
        {
            time_t date_ligne = convertir_date(date_str);
            if (date_ligne != (time_t)-1 && date_ligne >=limite ) 
            {
                // ligne récente → garder
                fputs(ligne, tmp);
            }
        }
    }

    fclose (src);
    fclose(tmp);

    src = fopen(nom_fichier, "w");
    tmp = fopen("temp.txt", "r");
    while(fgets(ligne , sizeof(ligne) , tmp))
    {
        fprintf(src , "%s" , ligne) ;
    }
    fclose (src);
    fclose(tmp);
    remove("temp.txt");
}

/**
*@brief recuperation du date d'alerte
*/
dates recap_date()
{
    char c ;
    dates n;
    printf("Recherche par date et heure:\n");
    printf("jour:");
    if(scanf("%d%c",&n.jour , &c)!=2 || c!='\n')
    {
        n.jour = 0 ;
        while(getchar()!='\n');
    }
    printf("mois:");
    if(scanf("%d%c",&n.mois, &c)!=2 || c!='\n')
    {
        n.mois = 0 ;
        while(getchar()!='\n');
    }
    printf("annee:");
    if(scanf("%d%c",&n.annee, &c)!=2 || c!='\n')
    {
        n.annee = 0 ;
        while(getchar()!='\n');
    }
    printf("heures:");
    if(scanf("%d%c",&n.heure, &c)!=2 || c!='\n')
    {
        n.heure = 0 ; 
        while(getchar()!='\n');
    }
    printf("minutes:");
    if(scanf("%d%c",&n.minute, &c)!=2 || c!='\n')
    {
        n.minute = 0 ;
        while(getchar()!='\n');
    }
    return(n);
}

void clean_history()
{
    int X ;
    char c ;
    printf("Les historiques plus vieux que X jours seront effaces \n") ;
    printf("Entrer X : ") ;
    if(scanf("%d%c",&X, &c)!=2 || c!='\n')
    {
        while(getchar()!='\n');
    }
    nettoyer_fichier("/var/log/monitor_track", X);
    printf("Nettoyage terminé : anciennes lignes (+%d jours) supprimées.\n" , X );
}

void find_in_history()
{
    dates a;
    char mot[30];
    a=recap_date();

    sprintf(mot,"%d/%d/%d - %d:%d",a.jour,a.mois,a.annee,a.heure,a.minute);
 
    grep_like("/var/log/monitor_track", mot);
}
