#include <sys/types.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>

#include <string.h>
#include "./modulo/constantes.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

    if(argc>2){ //Verificamos que no nos pongan mas de dos parametros
        SYSERROR(EXIT_FAILURE, "\nError en la cantidad de parámetros");
    }


    printf("Soy proceso Suscriptor y Publicador\n");

    return EXIT_SUCCESS;
}