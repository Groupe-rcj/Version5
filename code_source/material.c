#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>
#include<stdatomic.h>
#include"../en_tete/define.h"
#include"../en_tete/material.h"
#include"../en_tete/interface.h"
#include"../en_tete/menu.h"
#include"../en_tete/cpu.h"
#include"../en_tete/disk.h"
#include"../en_tete/config_daemon.h"

extern atomic_int run_material ;

void print_color(int debut , int fin , char*color , int y , int x )
{
    int i ;
    for(i=debut ; i<fin ; i++)
    {
        printf("\033[%d;%dH%s        %s" , y+i , x , color , NEUTRE);
    }    
}

void get_temperature(int x, int y, int print, float max_temperature)
{
    int i = 0;
    float temp = 0;
    char filename[128], ligne1[64], ligne2[64];
    FILE *fichier_type, *fichier_value;

    while (atomic_load(&run_material))
    {
        print_temperature_pattern(y, x);
        snprintf(filename, sizeof(filename), "/sys/class/thermal/thermal_zone%d/type", i);
        fichier_type = fopen(filename, "r");
        if (fichier_type==NULL) 
        {
            // plus de capteurs -> sortir proprement
            break;
        }
        if (!fgets(ligne1, sizeof(ligne1), fichier_type)) 
        {
            fclose(fichier_type);
            break;
        }
        fclose(fichier_type);

        snprintf(filename, sizeof(filename), "/sys/class/thermal/thermal_zone%d/temp", i);
        fichier_value = fopen(filename, "r");
        if (fichier_value==NULL) 
        {
            break;
        }
        if (fgets(ligne2, sizeof(ligne2), fichier_value)==NULL) 
        {
            fclose(fichier_value);
            break;
        }
        fclose(fichier_value);

        // enlever newline si présent
        if (ligne1[strlen(ligne1)-1] == '\n') 
        {
            ligne1[strlen(ligne1)-1] = '\0';
        }
        if (sscanf(ligne2, "%f", &temp) == 1)
        {
            if (print == 1)
            {
                printf("\033[%d;%dHTemperature venant du CAPTEUR %s: %.3f Deg C\n", y+i, x+10, ligne1, temp/1000.0f);
            }
            else if (temp/1000.0f > max_temperature && print == 0)
            {
                char message[128];
                snprintf(message, sizeof(message), "CAPTEUR %s detecte une temperature de %.3f Deg C\n", ligne1, temp/1000.0f);
                print_mesage(message, " [ ! ]  WARNING ", JAUNE , 0);
            }
        }
        i++;
        print_temperature_pattern(y, x);
        fflush(stdout);
        usleep(500000);
    }
}


char* get_material_info(const char* section, const char* dev_name)
{
    char filename[256];
    char buf[128];
    FILE *f;
    snprintf(filename, sizeof(filename), "/sys/class/power_supply/%s/%s", dev_name, section);
    f = fopen(filename, "r");
    if (f==NULL) 
    {
        return NULL;
    }
    if (fgets(buf, sizeof(buf), f)==NULL) 
    {
        fclose(f); 
        return NULL; 
    }
    fclose(f);
    // enlever le newline final
    size_t len = strlen(buf);
    if (len && buf[len-1] == '\n') 
    {
        buf[len-1] = '\0';
    }
    return strdup(buf); // appelant doit free()
}



void print_battery_level(float capacity , int x , int y )
{
    if(capacity==100)
    {
        print_color(0 , 6 , BACK_VERT , y , x ) ;
    }
    else if(capacity<20)
    {
        print_color(5 , 6 , BACK_VERT , y , x ) ;
    }
    else if(capacity<40)
    {
        print_color(4 , 6 , BACK_VERT , y , x ) ;
    }
    else if(capacity<60)
    {
        print_color(3 , 6 , BACK_VERT , y , x ) ;
    }
    else if(capacity<80)
    {
        print_color(2 , 6 , BACK_VERT , y , x ) ;
    }
    else if(capacity<100)
    {
        print_color(1 , 6 , BACK_VERT , y , x ) ;
    }
    printf("\033[%d;%dH%d" ,y+3 , x+3 , (int)capacity) ;
}

void get_power(int x ,int y )
{
    DIR* power_directory ;
    struct dirent *entry ;
    power_directory = opendir("/sys/class/power_supply") ;
    if(power_directory==NULL)
    {
        printf("No directory\n") ;
        return;
    }
    entry=readdir(power_directory) ;
    while(entry!=NULL)
    {
        if(strcmp(entry->d_name , ".")==0 || strcmp(entry->d_name , "..")==0)
        {
            entry=readdir(power_directory) ;
            continue ;
        }
        if(strstr(entry->d_name , "AC")==NULL)
        {
            char *present = get_material_info("present", entry->d_name);
            if (present==NULL) 
            {
                printf("\033[%d;%dHPeripherique de batterie %s : information 'present' INDISPONIBLE\n", y, x, entry->d_name);
                entry = readdir(power_directory);
                continue;
            }
            if (strcmp(present, "1") != 0) 
            {
                printf("\033[%d;%dHPeripherique de batterie %s  non disponible \n", y, x, entry->d_name);
                free(present);
                entry = readdir(power_directory);
                continue;
            }
            free(present);

            char *status = get_material_info("status", entry->d_name);
            char *voltage = get_material_info("voltage_now", entry->d_name);
            char *charge_now = get_material_info("charge_now", entry->d_name);
            char *charge_full = get_material_info("charge_full", entry->d_name);
            char *capacity = get_material_info("capacity", entry->d_name);

            if (status) 
            {
                printf("\033[%d;%dH     ETAT : %s\n", y+1, x+10, status);
                free(status);
            } 
            else 
            {
                printf("\033[%d;%dH     ETAT : INDISPONIBLE\n", y+1, x+10);
            }

            if (voltage) 
            {
                long v = strtol(voltage, NULL, 10);
                printf("\033[%d;%dH     TENSION : %f V\n", y+2, x+10, (float)v/1000000.0f);
                free(voltage);
            } 
            else 
            {
                printf("\033[%d;%dH     TENSION : INDISPONIBLE\n", y+2, x+10);
            }

            if (charge_now) 
            {
                long cn = strtol(charge_now, NULL, 10);
                printf("\033[%d;%dH     CHARGE ACTUELLE : %f Ah\n", y+3, x+10, (float)cn/1000000.0f);
                free(charge_now);
            } 
            else 
            {
                printf("\033[%d;%dH     CHARGE ACTUELLE : INDISPONIBLE\n", y+3, x+10);
            }

            if (charge_full) 
            {
                long cf = strtol(charge_full, NULL, 10);
                printf("\033[%d;%dH     CHARGE MAXIMALE : %f Ah\n", y+4, x+10, (float)cf/1000000.0f);
                free(charge_full);
            } 
            else 
            {
                printf("\033[%d;%dH     CHARGE MAXIMALE : INDISPONIBLE\n", y+4, x+10);
            }

            if (capacity) 
            {
                int cap = atoi(capacity);
                print_battery_level((float)cap, x, y);
                free(capacity);
            } 
            else 
            {
                printf("\033[%d;%dH     CAPACITE : INDISPONIBLE\n", y+5, x+10);
            }
        }
        entry=readdir(power_directory) ;
    }
    closedir(power_directory);
}


void* get_material_overview()
{
    int largeur , hauteur ;
    get_screen_size(&largeur , &hauteur) ;
    efface_ecran() ;
    time_t last_refresh = time(NULL) ;
    while(atomic_load(&run_material))
    {
        time_t now = time(NULL) ;
        if(difftime(now , last_refresh)>=2.0)
        {
            print_battery_pattern(1, 1) ;
            get_power(2, 3) ;
            get_temperature(largeur/2 ,1 , 1 , 0) ;
            get_disk_information(14,1 , largeur/2 , hauteur-4) ;
            printf("\033[%d;%dH Taper q pour quitter " , hauteur-1 , 1 )  ;
            fflush(stdout) ;
            last_refresh=now ;
        }
        usleep(500000) ;
    }
    return NULL ;
}
