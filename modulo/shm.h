#ifndef _SHM_H_
#define _SHM_H_

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <sys/stat.h>      //Bibliotec de semaforos
#include <fcntl.h>         //Bibliotec de semaforos
#include <semaphore.h>     //Bibliotec de semaforos


#define FTOK_FILE "/tmp/obl.ftok"
#define PROJ_ID 17
#define SHM_SIZE  684 //1024 * 10   //684
#define SHM_FLAGS IPC_CREAT || IPC_EXCL
#define SEM_NAME "/semaforoBus"


sem_t *obetenerSemaforo(char *nombre, int valor, int crearExcl);

key_t getProjId(char *filename, int projId);

int createShm(key_t myKey, size_t size, int crearExcl);

int desengancharShm(int shmId,void *shmp,int eliminar);

void *obtenerSHM(char *filename,  int  projId, size_t size,key_t myKey, int crearExcl );

void agregarArchivo (char *nombreArchivo, void *memoriaCompartida, int estado, int size, int indiceArchivoShm);

int obtenerIndiceShmArchivo(void *memoriaCompartida);

#endif
