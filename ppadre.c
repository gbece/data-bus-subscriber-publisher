#include <sys/types.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <stdlib.h>

#include <semaphore.h>     

#include <sys/stat.h>      
#include <fcntl.h> 

#include <string.h>

#include <signal.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "./modulo/constantes.h"
#include "./modulo/shm.h"
#include "./modulo/memoria.h"
#include "./modulo/proceso.h"

#define SYSERROR(status, msg) error_at_line(status, errno, __FILE__, __LINE__, msg)


int validacion(char header[], struct proceso *prs){  //Validamos el header para saber el rol que tiene
	int var=-1;
	int tipo=-1;
	
	char *aux = (char *) malloc(sizeof(char) * (strlen(header) + 1) );
    strcpy(aux, header);

	size_t largoMensaje=strlen(header);
	char* token;

	int flag=0;
	int bandera=1;
	int contador=0;

	
	token=strtok(aux, "\n:");
		while(token!=NULL && flag != -1){
			
			
			if(strstr(token, "tipo") != NULL ){
				token =	strtok( NULL, "\n:");
				tipo=atoi(token);
				
				contador++;
				bandera=0;
			}else if(strstr(token, "id") != NULL ){
				token =	strtok( NULL, "\n:");
				var=atoi(token);
				
				prs->id=var;
				contador++;
				bandera=0;
			}else if(strstr(token, "rol") != NULL ){
				token =	strtok( NULL, "\n:");
				var=atoi(token);
				
				prs->rol=var;
				if (prs->rol < 1 || prs->rol >4){  // Validamos el rango del rol
					prs->rol=-1;
				}
				contador++;
				bandera=0;
			}else if(strstr(token, "filtro") != NULL ){
				token =	strtok( NULL, "\n:");
				var=atoi(token);
				
				prs->filtro=var;
				contador++;
				bandera=0;
			}else if(strstr(token, "tamaño") != NULL ){
				token =	strtok( NULL, "\n:");
				var=atoi(token);
				
				prs->bytesEnviados=largoMensaje; //Guardamos el largo del mensaje entero no del cuerpo
				contador++;
				bandera=0;
			}else if(strstr(token, "direccion") != NULL ){
				token =	strtok( NULL, "\n:");
				
				contador++;
				bandera=0;
				flag=-1;
			}
			if (bandera){
				token =	strtok( NULL, "\n:");
			}
			bandera=1;
		}
		if (contador!=6){
			tipo=-2;  //Error de Cabecera
		}
		
	free (aux);
	return tipo;
}


int crearProcesoHijo(int rol, int nss, int puerto){

	pid_t newPid;

	char *aux= (char *) malloc(sizeof(char) * (MAX_SIZE + 1) );
	char *aux2= (char *) malloc(sizeof(char) * (MAX_SIZE + 1) );


		// Hasta aca solo existe el proceso Padre
	newPid = fork();

		// Luego del fork() existen padre e hijo con exactamente el mismo codigo.
		// ESTE ES EL PUNTO EN EL QUE EJECUTAN AMBOS PROCESOS, PADRE E HIJO.

	if (newPid > 0) {

		//Guardar en SHM el satelite nuevo (Publicador, Suscriptor o ambos) como activo

		//Se establece conexion con satelite hijo

		// Chequear si el sistema sigue activo



	} else if (newPid == 0) {
			
		//Proceso Hijo

		if(rol==1){
			//Publicador

			sprintf(aux, "%d", nss);

			if( (execl("./ppublicador", "ppublicador", aux , NULL)) == -1 ) {
				//Error a ejecutar el archivo	
				SYSERROR(EXIT_FAILURE, "Error al ejecutar el archivo de Proceso Publicador");			
			}
		}else if(rol==2){
			//Suscriptor

			sprintf(aux, "%d", nss);
			sprintf(aux2, "%d", puerto);

			if( (execl( "./psuscriptor", "psuscriptor", aux, aux2 , NULL)) == -1 ) {
				//Error a ejecutar el archivo	
				SYSERROR(EXIT_FAILURE, "Error al ejecutar el archivo de Proceso Suscriptor");			
			}
		}else{
				//Publicador y Suscriptor

				sprintf(aux, "%d", nss);
			if( (execl("./psusypub", "psusypub" ,aux, NULL)) == -1 ) {
					//Error a ejecutar el archivo	
				SYSERROR(EXIT_FAILURE, "Error al ejecutar el archivo de Proceso Suscriptor");			
			}
		}
			
	} else {
			SYSERROR(EXIT_FAILURE, "El PADRE no pudo crear el proceso HIJO\n");
	}

	free(aux);

	return newPid;
}

int sistemaActivo(void *shmp, sem_t *sem){
	int aux=1;
	//Entra a memoria compartida para chequear si el sistema esta activo
	struct memoria mem;
	
	sem_wait(sem);
	//---------operaciones sobre memoria compartida---------
	
	memcpy(&mem, shmp, sizeof(struct memoria)); 
	
	//------------------------------------------------------
	sem_post(sem);

	aux=mem.estado;

	return aux;	
}

int bajaProceso(void *shmp, sem_t *sem, int idPros){
	
	int aux=1;
	struct memoria mem;

	sem_wait(sem);
	//---------operaciones sobre memoria compartida---------
	memcpy(&mem, shmp, sizeof(struct memoria));   //Leemos de memoria

	int i=0;
	while ( (mem.process[i].id) != idPros ){  //Buscamos el proceso que se quiere dar de baja
		i++;
	}

	mem.process[i].estado=0; //Se pone como baja
	mem.procesosActivos=mem.procesosActivos-1;

	memcpy(shmp, &mem, sizeof(struct memoria));   //Guardamos en Memoria
	//------------------------------------------------------
	sem_post(sem);

	//Entra a memoria compartida para chequear si el sistema esta activo
	return aux;	
}

void iniciaSistema(void *shmp, sem_t *sem){

	struct memoria mem;
	int puerto=PRSPORT;

	mem.estado=1;
    mem.procesosTotales=0;
    mem.procesosActivos=0;
    mem.publicadoresActivos=0;
    mem.suscriptoresActivos=0;
    mem.publicadoresYSuscriptoresActivos=0;
	
	sem_wait(sem);
	//---------operaciones sobre memoria compartida---------

	
	for(int i=0;i<20;i++){

		mem.process[i].estado=0; //Inicializamos todos los procesos como inactivos
		mem.process[i].rol=0;   
    	mem.process[i].id=0;    
    	mem.process[i].filtro=0; 
    	mem.process[i].mensajesEnviados=0; 
    	mem.process[i].mensajesRecibidos=0;
    	mem.process[i].bytesEnviados=0;
    	mem.process[i].bytesRecibidos=0;


		mem.process[i].puerto=puerto;
		puerto++;
	}
	memcpy(shmp, &mem, sizeof(struct memoria));
	//------------------------------------------------------
	sem_post(sem);
 	
}

int guardarProceso(void *shmp, sem_t *sem, int rol, int idPros, int tipo, int largoMensaje){
	int aux=1;
	struct memoria mem;

	sem_wait(sem);
	//---------operaciones sobre memoria compartida---------
	memcpy(&mem, shmp, sizeof(struct memoria));   //Leemos de memoria

	int i=0;

	while ( (mem.procesosActivos<20) && ((mem.process[i].estado) == 1) ){  //Buscamos un proceso que ese vacio
		i++;
	}
	
	if( i < 20){ //Capacidad maxima de procesos activos
		mem.procesosTotales=mem.procesosTotales+1;
		mem.procesosActivos=mem.procesosActivos+1;

		mem.process[i].estado=1;
    	mem.process[i].rol=rol;
    	mem.process[i].id=idPros;
    	mem.process[i].filtro=tipo;
    	mem.process[i].mensajesEnviados=1;
    	mem.process[i].mensajesRecibidos=1;
    	mem.process[i].bytesEnviados=largoMensaje;
    	mem.process[i].bytesRecibidos=1;

		aux=mem.process[i].puerto;  //Devolvemos el puerto

	}else{
		aux=-1;
	}

	memcpy(shmp, &mem, sizeof(struct memoria));   //Guardamos en Memoria
	//------------------------------------------------------
	sem_post(sem);

	
	//Entra a memoria compartida para chequear si el sistema esta activo
	return aux;	
}

char *mensajeRespuesta(char * buf, int tipoMensaje){
	char *aux = (char *) malloc(sizeof(char) * (strlen(buf) + 1) );

	strcpy(buf,"tipo:");    
	sprintf(aux, "%d", tipoMensaje );
	strcat(buf,aux);
	strcat(buf,"\nid:");
	sprintf(aux, "%d",getpid());
    strcat(buf,aux);
    strcat(buf, "\nrol:5\nfiltro:0\ntamaño:0\ndireccion:0\n\n-\r\n\r\n");

	free(aux);
	return buf;
}

void resetSructPrs(struct proceso *prs){
	prs->estado=0; //Inicializamos todos los procesos como inactivos
	prs->rol=0;   
    prs->id=0;    
    prs->filtro=0; 
	prs->puerto=0; 
    prs->mensajesEnviados=0; 
    prs->mensajesRecibidos=0;
    prs->bytesEnviados=0;
    prs->bytesRecibidos=0;
}

int crearSocketCliente(int puerto){
    int cs; // client socket

    struct sockaddr_in srvaddr; // server address
    
    // crear socket
	if ((cs = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		SYSERROR(EXIT_FAILURE, "Error al crear el socket servidor.");
	}

	srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(puerto);
	srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	while (connect(cs, (struct sockaddr *) &srvaddr,(socklen_t) sizeof(struct sockaddr_in) ) == -1) {
		//SYSERROR(EXIT_FAILURE, "Error al conectar.");
        //Esperamos a que conecte
	}

    return cs;
}

int crearSocketEscucha(int puerto){
    int ss=0; // server socket
	int nss=0; // new server socket

    struct sockaddr_in addr; // listen address
	struct sockaddr_in caddr; // client address

    socklen_t addrlen;

    // crear socket
    if ((ss = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        SYSERROR(EXIT_FAILURE, "Error al crear el socket servidor.");
    }
    // asociar una direccion al socket o ponerle nombre al socket
    addr.sin_family = AF_INET; // esquema de direccionamiento IP
    addr.sin_port = htons(puerto); // puerto de escucha
    addr.sin_addr.s_addr = INADDR_ANY; // todas las interfases

    if (bind(ss, (struct sockaddr *) &addr, sizeof(struct sockaddr_in) ) == -1) {
        SYSERROR(EXIT_FAILURE, "Error al nombrar el socket servidor.");
    }
    // poner el socket en modo de escucha
    if (listen(ss, MAXCONN) == -1) {
        SYSERROR(EXIT_FAILURE, "Error al iniciar escucha.");
    }

    // aceptar conexiones
    if  ((nss = accept(ss, (struct sockaddr *) &caddr, &addrlen)) == -1) {
        SYSERROR(EXIT_FAILURE, "Error al establecer conexion.");
		
    }
    return nss;
}

int puertoVacio(void *shmp, sem_t *sem){

	int aux=1;
	struct memoria mem;

	sem_wait(sem);
	//---------operaciones sobre memoria compartida---------
	memcpy(&mem, shmp, sizeof(struct memoria));   //Leemos de memoria

	int i=0;

	while ( (mem.procesosActivos<20) && ((mem.process[i].estado) == 1) ){  //Buscamos un proceso que ese vacio
		i++;
	}
	
	if( i < 20){ //Capacidad maxima de procesos activos
		
		aux=mem.process[i].puerto;  //Devolvemos el puerto
		aux=aux-40;
	}else{
		aux=-1;
	}
	//------------------------------------------------------
	sem_post(sem);

	
	
	return aux;	

}




int main(int argc, char **argv) {

	int sistActivo=1;
	int tipo;
	
	void *shmp;
    key_t myKey = getProjId( FTOK_FILE, PROJ_ID );
    struct proceso prs;

    if ((shmp=obtenerSHM(FTOK_FILE, PROJ_ID, SHM_SIZE, myKey, TRUE ))== NULL){
		SYSERROR(EXIT_FAILURE, "Fallo en obtener SHM");
	}
    int shmId= createShm(myKey, SHM_SIZE, FALSE) ;

	sem_t *sem=obetenerSemaforo(SEM_NAME, 1 , TRUE);
	

	// sockets
	// socketpeername 
	// getpeername



	// Bus de prueba de mensaje
	char buf[BUFSIZE];

	

    if(argc!=1){ //Verificamos que no nos pongan mas de dos parametros
        SYSERROR(EXIT_FAILURE, "\nError en la cantidad de parámetros");
    }

	printf("Bienvenido al Sistema de Bus\n");
	iniciaSistema(shmp, sem); 

	///SOCKET

	int nss; // new server socket
	int r; // received bytes from socket
	
	int puerto;

	while ( sistActivo==1 ) {

		resetSructPrs(&prs);
		
		puerto = puertoVacio(shmp, sem);

		nss = 0;
		nss = crearSocketEscucha(puerto);

		printf("Conexion Establecida\n");

		if ((r = recv(nss, buf, BUFSIZE, 0)) == -1) {
        	SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
    	}


		tipo=validacion(buf, &prs); // Validamos 

		if ( prs.rol > 0  && prs.rol<4 ){
			
			if(tipo ==100){
				
				puerto = guardarProceso(shmp, sem, prs.rol, prs.id, prs.filtro, prs.bytesEnviados);

				crearProcesoHijo(prs.rol, nss, puerto);
				

			}else if (tipo == 102){  // 102 Solicitud

				strcpy(buf, mensajeRespuesta(buf, 101));  //   101 Confirmacion

			}else if (tipo == 103){  // 103 Publicacion

				printf("Solicitud\n");
				
			}else if (tipo == 300){  //   300 Desconexion

				bajaProceso(shmp,sem, prs.id);

				strcpy(buf, mensajeRespuesta(buf, 101));  //   101 Confirmacion

				if ((r = send(nss, buf, BUFSIZE, 0)) != BUFSIZE) {
					SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
				}

			}else{
				strcpy(buf, mensajeRespuesta(buf, 201));   // 201 Error de solicitud

				if ((r = send(nss, buf, BUFSIZE, 0)) != BUFSIZE) {
					SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
				}

			}

		}else if ( prs.rol ==4){

			strcpy(buf, mensajeRespuesta(buf, 202));   // 201 Error de rol

			if ((r = send(nss, buf, BUFSIZE, 0)) != BUFSIZE) {
					SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
			}

		}else {

			strcpy(buf, mensajeRespuesta(buf, 201)); // 201 Error de solicitud

			if ((r = send(nss, buf, BUFSIZE, 0)) != BUFSIZE) {
				SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
			}
		}


		sistActivo=sistemaActivo(shmp, sem);
	}

	kill ( 0 , SIGKILL);  // Matar Hijos

	shutdown(nss, SHUT_RDWR);
	
	if (desengancharShm(shmId, shmp, TRUE)==-1){
		SYSERROR(EXIT_SUCCESS, "Fallo al eliminar SHM");
	}

    sem_close(sem);
    sem_unlink(SEM_NAME);

	return EXIT_SUCCESS;
}