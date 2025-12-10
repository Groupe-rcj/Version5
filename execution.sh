#!/bin/bash

gcc -g -pedantic -Wall -Werror  -I ../en_tete/\
	../code_source/interface.c  ../code_source/menu.c ../code_source/lim2.c\
	 ../code_source/material.c ../code_source/cpu.c ../code_source/ram.c\
	 ../code_source/network.c ../code_source/pid_info.c ../code_source/liste_proc.c  \
	 ../code_source/disk.c ../code_source/fichier_ouvert.c ../code_source/input.c  ../code_source/config_daemon.c\
	 ../code_source/main.c -o monitor_track  -pthread -lcrypt `pkg-config --cflags --libs gtk+-3.0`

if [ $? -eq 0 ]
then
	echo "COMPILE AVEC SUCCES (entrer ./monitor_track pour lancer l'executable)"
fi
