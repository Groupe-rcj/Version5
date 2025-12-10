/**
*@file main.c
*@mainpage 
* @brief
*               **NOM DU PROJET** : MONITOR TRACK\n\n

*       DESCRIPTION GÉNÉRALE\n\n

* Ce projet est une application écrite en C qui permet de surveiller, en temps réel,\n
*les ressources d’un système Linux telles que le materiel, le processeur, la mémoire, le réseau\n
*et les processus actifs. Il utilise plusieurs threads pour exécuter ces tâches en\n
*parallèle et un démon système pour gérer la supervision et la configuration en\n
*tâche de fond.\n\n

* L’application combine performance, modularité et robustesse pour proposer une\n 
*solution complète de monitoring bas-niveau, adaptée aussi bien aux étudiants\n 
*qu’aux environnements de test avancés.\n\n

*OBJECTIFS DU PROJET\n
*- Fournir une vision temps réel du comportement du système.\n
*- Exploiter le multithreading pour maximiser les performances.\n
*- Proposer un démon autonome qui gère la surveillance continue.\n

*PROBLÈMES RÉSOLUS\n
*1. **Surveillance simultanée** : Les threads indépendants permettent de suivre\n 
*   le CPU, la mémoire, le réseau , le materiel et les processus sans blocage mutuel.\n

*2. **Blocages séquentiels** : La synchronisation par des variables atomiques\n 
*   assure une exécution fluide et évite les goulots d’étranglement.\n

*3. **Mise à jour dynamique** : Le démon redémarre ou recharge la configuration\n 
*   automatiquement dès qu’un changement est détecté.\n

*4. **Supervision des processus** : Intégration avec /proc pour lire directement\n 
*   les informations PID, UID, mémoire et état des tâches.\n

*FONCTIONNALITÉS CLÉS\n

* **Multithreading** : gestion parallèle des ressources système.\n
* **Démon de surveillance** : tourne en tâche de fond et maintient la cohérence\n 
* des données(configuration personelle de l'utilisateur).\n
* **Lecture du /proc** : collecte directe et efficace des données système.\n
* **Interface utilisateur textuelle** : menu interactif simple pour consulter\n 
  les statistiques.\n
* **Logs et relance automatique** : le démon peut se relancer et se mettre a jour automatiquement\n
* selon l'etat du fichier de configuration.\n
* **Structure modulaire** : séparation claire entre les modules cpu.c, ram.c,\n 
  network.c, pid_info.c et config_daemon.c.\n\n


*ORIGINALITÉ DU PROJET\n

*- Association d’un démon autonome et d’une application interactive en C pur.\n
*- Lecture directe des données via /proc sans dépendances externes.\n
*- Mise en œuvre d’un multithreading propre, lisible et portable POSIX.\n

*ASPECTS TECHNIQUES\n\n

*- **Langage** : C (POSIX)\n
*- **Librairies utilisées** : pthread, stdio, unistd, sys/types, sys/stat , stdlib , sys/inotify, signal\n 
*- **Interface** : CLI simple avec menus et affichages rafraîchis\n\n

*EXÉCUTION\n
*1. Compilation :\n
*    ./execution dans le dossier bash_script\n
*2. Lancement de l’application principale :\n
*   ./monitor_track dans le dossier bash_script\n
*3. Lancement du démon :\n
*   ./monitor_daemon dans le dossier bash_script ou menu dans l'application priincipale\n
*4. Arrêt du démon :\n
*    option 3 dans\n 
*   ./monitor_track dans le dossier bash_script (requisition d'autorisation de root)\n


*@author Développé par Jaspy, Robert, Chywwah, (2025)Université d’Antananarivo.

*LICENCE\n
*Projet libre\n

*/


#include"../en_tete/menu.h"
#include<signal.h>
/**
*@brief Fonction principale du projet 
*/

int main()
{
    /**
    *@brief Ignorer le signal d interruption pour une terminaison propre
    *@param SIGINT on capte le signal d'interruption provoque par Ctrl+C
    *@param SIG_IGN on ignore le signal 
    */
    signal(SIGINT , SIG_IGN) ;
    affiche_menu() ; 
    return 0 ; 
}
