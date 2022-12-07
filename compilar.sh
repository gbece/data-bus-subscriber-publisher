#!/bin/bash
clear

#Compilar proceso padre
echo "Compilando proceso padre"
gcc -Wall shm.c ppadre.c -lpthread -o ppadre

#Compilar Satelite Suscritor
echo "Compilando Proceso Suscritor"
gcc -Wall psuscriptor.c -o psuscriptor

#Compilar Satelite Suscritor
echo "Compilando Proceso Publicador"
gcc -Wall shm.c ppublicador.c -lpthread -o ppublicador

#Compilar Satelite Suscritor
echo "Compilando Satelite Suscritor"
gcc -Wall satSuscriptor.c -o satSuscriptor


#Compilar Satelite Publicador
echo "Compilando Satelite Publicador"
gcc -Wall satPublicador.c -o satPublicador


#Compilar Consola Administrador
echo "Compilando Consola Adminisitrador"
gcc -Wall consAdmin.c -o consAdmin

#Compilar Proceso Administrador
echo "Compilando Proceso Adminisitrador"
gcc -Wall shm.c padministrador.c -lpthread -o padministrador

#gnome-terminal -e ./ppadre
#gnome-terminal -e ./satPublicador
#gnome-terminal -e ./satPublicador