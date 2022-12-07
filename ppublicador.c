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

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "./modulo/constantes.h"
#include "./modulo/shm.h"
#include "./modulo/memoria.h"
#include "./modulo/proceso.h"


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


int procesoSuscriptos(void *shmp, sem_t *sem,int filtro, char *buf, int id, int bytesEnviados ){
	
	int aux=1;
	struct memoria mem;

	int contador=0;
	int j=0;

	int puerto;

	int cs=0;
	int r;

	sem_wait(sem);
	//---------operaciones sobre memoria compartida---------
	memcpy(&mem, shmp, sizeof(struct memoria));   //Leemos de memoria
	//------------------------------------------------------
	sem_post(sem);


	for(int i=0;i<20;i++){

		if ( ((mem.process[i].estado) == 1 ) && ((mem.process[i].rol) == 2 ) && ((mem.process[i].filtro) == filtro )) {
			
			contador++;
			puerto=mem.process[i].puerto;
 
    		mem.process[i].mensajesRecibidos=mem.process[i].mensajesEnviados+1;
    		mem.process[i].bytesRecibidos=mem.process[i].bytesRecibidos+bytesEnviados;
			
			cs=crearSocketCliente(puerto);

			if ((r = send(cs, buf, BUFSIZE, 0)) != BUFSIZE) {
		            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
	        }

		}
		if( ((mem.process[i].estado) == 1 ) && ((mem.process[i].id) == id ) ){
			j=i;
		}
		
	}
	mem.process[j].mensajesEnviados=mem.process[j].mensajesEnviados+contador;
	mem.process[j].mensajesRecibidos=mem.process[j].mensajesRecibidos+1;
	mem.process[j].bytesEnviados=mem.process[j].bytesEnviados+bytesEnviados*contador;
	mem.process[j].bytesRecibidos=mem.process[j].bytesRecibidos+bytesEnviados;

	sem_wait(sem);
	//---------operaciones sobre memoria compartida---------
	memcpy(shmp, &mem, sizeof(struct memoria));//Guardamos de memoria
	//------------------------------------------------------
	sem_post(sem);


	return aux;	
}


int sistemaActivo(void *shmp, sem_t *sem){
	int aux=1;
	//Entra a memoria compartida para chequear si el sistema esta activo
	struct memoria mem;
	
	sem_wait(sem);
	//---------operacionprintf("Soy proceso Suscriptor\n");

	memcpy(&mem, shmp, sizeof(struct memoria));
	//------------------------------------------------------
	sem_post(sem);

	aux=mem.estado;

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



int crearSocketEscucha(){
    int ss; // server socket
	int nss; // new server socket

    struct sockaddr_in addr; // listen address
	struct sockaddr_in caddr; // client address

    socklen_t addrlen;

    // crear socket
    if ((ss = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        SYSERROR(EXIT_FAILURE, "Error al crear el socket servidor.");
    }
    // asociar una direccion al socket o ponerle nombre al socket
    addr.sin_family = AF_INET; // esquema de direccionamiento IP
    addr.sin_port = htons(ADMIN_PORT); // puerto de escucha
    addr.sin_addr.s_addr = INADDR_ANY; // todas las interfases

    if (bind(ss, (struct sockaddr *) &addr, sizeof(struct sockaddr_in) ) == -1) {
        SYSERROR(EXIT_FAILURE, "Error al nombrar el socket servidor.");
    }
    // poner el socket en modo de escucha
    if (listen(ss, MAXCONN) == -1) {
        SYSERROR(EXIT_FAILURE, "Error al iniciar escucha.");
    }
    // aceptar conexiones
    if ((nss = accept(ss, (struct sockaddr *) &caddr, &addrlen)) == -1) {
        SYSERROR(EXIT_FAILURE, "Error al establecer conexion.");
    }
    return nss;
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


int main(int argc, char **argv) {

    printf("Soy proceso Suscriptor\n");

	char buf[BUFSIZE];
	char bufRespuesta[BUFSIZE];
	struct proceso prs;

    if ( argc != 2 ){ //Verificamos que no nos pongan mas de dos parametros
        SYSERROR(EXIT_FAILURE, "\nError en la cantidad de parámetros");
    }


	//  SHM Y Semaforo
	void *shmp;
    key_t myKey = getProjId( FTOK_FILE, PROJ_ID );
    
    
    if ((shmp=obtenerSHM(FTOK_FILE, PROJ_ID, SHM_SIZE,myKey, FALSE ))== NULL){
		SYSERROR(EXIT_FAILURE, "Fallo en obtener SHM");
	}

	sem_t *sem=obetenerSemaforo(SEM_NAME, 1 , FALSE);

	//SOCKET
	// Socket con el Satelite
    int nss; // new server socket
	int r; // received bytes from socket


    nss = atoi( argv[1] );
    
    strcpy(buf, mensajeRespuesta(buf, 101));

    if ((r = send(nss, buf, BUFSIZE, 0)) != BUFSIZE) {
            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
    }
	
    int sistActivo=1;

    while ( sistActivo==1) {

		resetSructPrs(&prs);

		if ((r = recv(nss, buf, BUFSIZE, 0)) == -1) {
            SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
        }	

		int tipo=validacion(buf, &prs); // Validamos 
		
		if (tipo == 103){  // 103 Publicacion

			procesoSuscriptos(shmp, sem, prs.filtro, buf, prs.id, prs.bytesEnviados);
			printf("Solicitud\n");

			strcpy(bufRespuesta, mensajeRespuesta(bufRespuesta, 101)); // 101 Confirmacion

			if ((r = send(nss, bufRespuesta, BUFSIZE, 0)) != BUFSIZE) {
            	SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
        	}
				
		}else{

			strcpy(bufRespuesta, mensajeRespuesta(bufRespuesta, 201)); //  201 Error de Solicitud

			if ((r = send(nss, bufRespuesta, BUFSIZE, 0)) != BUFSIZE) {
            	SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
        	}

		}


        sistActivo=sistemaActivo(shmp, sem);
    }


	sem_unlink(SEM_NAME);

    shutdown(nss, SHUT_RDWR);

    return EXIT_SUCCESS;
}