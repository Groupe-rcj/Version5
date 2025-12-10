#!/bin/bash

gcc -Wall -Werror  -I../en_tete/ ../code_source/interface.c  ../code_source/liste_proc.c\
    ../code_source/menu.c ../code_source/material.c  ../code_source/cpu.c \
    ../code_source/ram.c ../code_source/network.c ../code_source/pid_info.c \
    ../code_source/disk.c ../code_source/fichier_ouvert.c ../code_source/input.c ../code_source/lim2.c\
    ../code_source/config_daemon.c ../code_source/main_daemon.c  -pthread -o monitor_daemon -lcrypt -lpthread

if [ $? -eq 0 ]
then
    echo "SUCCES"
fi

