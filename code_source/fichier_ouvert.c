#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include"../en_tete/fichier_ouvert.h"
#include"../en_tete/interface.h"


Fichier list_open_files(int pid)
{
    Fichier resultat = {NULL, 0};
    char path[255];
    
    snprintf(path, sizeof(path), "/proc/%d/fd", pid); //regarder tous les descripteurs de fichier que le procesu ouvre
    DIR *dir = opendir(path);
    
    if (dir==NULL)
    {
        printf("Impossible d'accéder aux fichiers ouverts pour le PID %d.\n", pid);
        return resultat;
    }

    // Compter d'abord le nombre de fichiers
    int count = 0 ;
    struct dirent *ent;
    
    while ((ent = readdir(dir)) != NULL)
    {
        if (ent->d_name[0] == '.') //fichier cache ou reference vers repertoire d'au dessus ou courant
        {
            continue;
        }
        count++;
    }
    
    rewinddir(dir); // Retour au début du répertoire
    
    // Allouer la mémoire
    if (count > 0)
    {
        resultat.fic = malloc(count * sizeof(char*));
        if (resultat.fic==NULL)
        {
            printf("Erreur d'allocation mémoire.\n");
            closedir(dir);
            return resultat ;
        }
        
        int i = 0;
        while ((ent = readdir(dir)) != NULL && i < count)
        {
            if (ent->d_name[0] == '.')
            {
                continue;
            } 
            
            char fdpath[512], link[256];
            snprintf(fdpath, sizeof(fdpath), "%s/%s", path, ent->d_name);
            
            ssize_t len = readlink(fdpath, link, sizeof(link)-1); // tous les descripteurs de fichiers sont des liens symboliques
            if (len != -1 && i<count)
            {
                link[len] = '\0';
                // Allouer chaque chaîne
                resultat.fic[i] = malloc(518 * sizeof(char));
                if (resultat.fic[i]!=NULL)
                {
                    snprintf(resultat.fic[i], 518 , "%s", link);
                    i++;
                }
            }
        }
        resultat.taille = i;
    }
    
    closedir(dir);
    return resultat;
}

// Fonction pour libérer la mémoire
void free_files(Fichier *f)
{
    if (f!=NULL && f->fic!=NULL)
    {
        for (int i = 0; i < f->taille; i++)
        {
            free(f->fic[i]);
        }
        free(f->fic);
        f->fic = NULL;
        f->taille = 0;
    }
}

void print_opened_file(int pid , int y )
{
    int largeur , hauteur ;
    get_screen_size(&largeur , &hauteur) ;
    Fichier fichiers = list_open_files(pid);
    bouge_curseur(y,largeur/2) ;
    printf("   Total: %d fichiers             ", fichiers.taille);
    bouge_curseur(y+1,1) ;
    for (int i = 0; i < fichiers.taille; i++)
    {
        printf(" %s , ", fichiers.fic[i]) ;
    }
    bouge_curseur(hauteur-2,largeur);
    free_files(&fichiers);    
}