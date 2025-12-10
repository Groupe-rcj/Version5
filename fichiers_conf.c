#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdio.h>

void fichier_conf()
    {
     pid_t pid = fork();

     if (pid == 0)
       {
	execlp("nano","nano","/etc/monitor_daemon",NULL);
	exit(1);
       }
     else if (pid > 0)
       {
	 wait(NULL);
       }
    }
int main()
     {
      //execution de l'ouverture du l'editeur nano 
      fichier_conf();
      return(0);
     }
