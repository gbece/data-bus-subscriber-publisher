#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>


#include <sys/stat.h>      //Bibliotec de semaforos
#include <fcntl.h>         //Bibliotec de semaforos
#include <semaphore.h>     //Bibliotec de semaforos


#include "./modulo/shm.h"
#include "./modulo/proceso.h"
#include "./modulo/constantes.h"
#include "./modulo/memoria.h"

#define myError(status, format, xx...) \
	error_at_line(status, errno, __FILE__, __LINE__, format, xx)

#define myError1(status, format) \
	error_at_line(status, errno, __FILE__, __LINE__, format)



sem_t *obetenerSemaforo(char *nombre, int valor, int crearExcl);
key_t getProjId(char *filename, int projId);
int createShm(key_t myKey, size_t size, int crearExcl);
int desengancharShm(int shmId,void *shmp,int eliminar);
void *obtenerSHM(char *filename,  int  projId, size_t size,key_t myKey, int crearExcl );


sem_t *obetenerSemaforo(char *nombre, int valor, int crearExcl){
	
	sem_t *sem;
	int flags;
	// crearExcl en 0 --> solo solicitar acceso
	// crearExcl en 1 --> exigir creacion

	flags = crearExcl ? O_CREAT | O_EXCL : O_CREAT;
	//flags |= S_IRWXU | S_IRWXG | S_IRWXO;

	

	//crear el sefaforo
	if ( ( sem= sem_open (nombre, flags, 0666, crearExcl)) == SEM_FAILED){
		myError1(EXIT_FAILURE, "Fallo al obtener el semaforo");
	}
	
	//inicializar el semaforo
	if (crearExcl && sem_init(sem, 1, valor)==-1){
		myError1(EXIT_FAILURE, "Error al inicializar el semaforo");
	}

	return sem;
}

void *obtenerSHM(char *filename,  int  projId, size_t size,key_t myKey, int crearExcl ){
	
	//key_t myKey;
	int shmId;
	void *shmp;

	// PASO 1: obtener un ID propio para la shm de mi SISTEMA
	//if (crearExcl== 1){
	//	myKey = getProjId(filename, projId);
	//}
	// PASO 2: solicitar acceso a la SHM
	shmId = createShm(myKey, size, crearExcl);
  
	// PASO 3: mapear (acceder) a las direcciones de memoria de la SHM
	if ((shmp = shmat(shmId, NULL, 0)) == (void *) -1) {
		myError1(EXIT_FAILURE, "Fallo en shmat()");
	}

	return shmp;
}


int desengancharShm(int shmId, void *shmp,int  eliminar){
	// Marcar la memoria compartida para ser eliminada
	if (eliminar && shmctl(shmId, IPC_RMID, NULL)== -1){
		myError1(EXIT_SUCCESS, "Fallo en obtener SHM");
	}
	//Desenganchar la memoria compartida
	return shmdt(shmp);
}



//###################################################################

key_t getProjId(char *filename, int projId) {

	key_t key;
	int mode = S_IRWXU | S_IRWXG | S_IRWXO;

	// 1.1 crear el archivo FTOK_FILE
	if (creat(filename, mode) == -1 ) {
		myError(EXIT_FAILURE, "Fallo de creat() en %s", FTOK_FILE);
	}

	// 1.2 obtener ID propio de mi SISTEMA
	if ((key = ftok(filename, projId)) == -1) {
		myError(EXIT_FAILURE, "Fallo en ftok() en %s", FTOK_FILE);
	}
	return key;

}

//###################################################################

int createShm(key_t myKey, size_t size, int crearExcl) {
	int shmId;
	int flags;
	// crearExcl en 0 --> solo solicitar acceso
	// crearExcl en 1 --> exigir creacion de la shm

	// PASO 2
	flags = crearExcl ? IPC_CREAT | IPC_EXCL : IPC_CREAT;
	flags |= S_IRWXU | S_IRWXG | S_IRWXO;
	
	if ((shmId = shmget(myKey, size, flags)) == -1) {
		myError(EXIT_FAILURE, "Fallo de shmget() con clave %d", myKey);
	}
	return shmId;
}

