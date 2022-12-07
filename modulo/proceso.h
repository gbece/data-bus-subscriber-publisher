#ifndef _PROCESO_H_
#define _PROCESO_H_

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct proceso {
    int estado;    
    int rol;   
    int id;    
    int filtro;
    int puerto;  
    int mensajesEnviados; 
    int mensajesRecibidos;
    int bytesEnviados;
    int bytesRecibidos;
} proceso ;  //  33 Bytes
  


#endif
