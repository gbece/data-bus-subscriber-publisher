# data-bus-subscriber-publisher

ARCHIVO DE PRUEBA

EJECUTAR EL ARCHIVO COMPILAR QUE ES UN EJECUTABLE PARA COMPILAR (./compilar.sh)

EJECUTAR PRIMERO DESDE UNA TERMINAL EL ARCHIVO: ./ppadre

SE ESTABLECIERON ALGUNOS FILTROS QUE CORRESPONDEN AL TIPO DE ARCHIVO QUE SE ENVIAN. 
1 - .txt
2 - .png
3 - OTROS TIPOS DE ARCHIVOS

YA SE TIENE COMO EJEMPLO EL SIGUIENTE ARCHIVO: "./prueba/hola.txt" QUE SERIA DE FILTRO 1 

PARA EJECUTAR ESTE ARCHIVO DE PRUEBA SE EJECUTA DESDE TERMINALES NUEVAS DE LA SIGUIENTE FORMA: ./satSuscriptor 1
								                               ./satPublicador 1

EL SUSCRIPTOR VA A QUEDAR A LA ESPERA DE UNA PUBLICACION DE TIPO .txt

EL PUBLICADOR VA A QUEDAR A LA ESPERA DE QUE SE SUBA UNA PUBLICACION, PARA PROBAR CON EL ARCHIVO DADO HAY QUE PONER "./prueba/hola.txt"

EL SUSCRIPTOR VA A RECIBIR LA PUBLICACION Y GUARDARLA DE FORMA LOCAL

PARA PROBAR LA CONSOLA HAY QUE EJECUTAR DESDE TERMINALES NUEVAS LOS ARCHIVOS:./padministrador
								             ./consAdmin

EN LA CONSOLA ADMINISTRADOR SE VAN A DESPLEGAR LAS DISTINTAS OPCIONES, Y SE DEBE INTRODUCIR EL NUMERO DE LA OPCION PARA RECIBIR LA SOLICITUD DESEADA
