#ifndef _MEMORIA_H_
#define _MEMORIA_H_

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "./proceso.h"

typedef struct memoria {
    int estado;
    int procesosTotales;
    int procesosActivos;
    int publicadoresActivos;
    int suscriptoresActivos;
    int publicadoresYSuscriptoresActivos;
    struct proceso process[20];   //33*20
} memoria; //684
 


#endif
