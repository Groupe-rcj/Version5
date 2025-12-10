#define MAX_LIGNES 100
#define MAX_TAILLE_LIGNE 1024
#define MAX_LINE 1024
#include <time.h>

struct dates
{
      int jour;
      int mois;
      int annee;
      int heure;
      int minute;
      int seconde;
};

typedef struct dates dates;
void grep_like(const char *filename, const char *pattern);
void supprime_codes_couleur(const char *str);
time_t convertir_date(const char *date_str);
void nettoyer_fichier(const char *nom_fichier, int jours_a_garder);
dates recap_date();
void clean_history() ;
void find_in_history() ;