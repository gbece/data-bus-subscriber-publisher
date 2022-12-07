#include <sys/types.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>

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



int crearSocketCliente(){
    int cs; // client socket

    struct sockaddr_in srvaddr; // server address
    
    // crear socket
	if ((cs = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		SYSERROR(EXIT_FAILURE, "Error al crear el socket servidor.");
	}

	srvaddr.sin_family = AF_INET;
	//srvaddr.sin_port = htons(SRVPORT);
    srvaddr.sin_port = htons(ADMIN_PORT);
	srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	while (connect(cs, (struct sockaddr *) &srvaddr,(socklen_t) sizeof(struct sockaddr_in) ) == -1) {
		//SYSERROR(EXIT_FAILURE, "Error al conectar.");
        //Esperamos a que conecte
	}

    return cs;
}

int crearSocketEscucha(int puerto){
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
    if ((nss = accept(ss, (struct sockaddr *) &caddr, &addrlen)) == -1) {
        SYSERROR(EXIT_FAILURE, "Error al establecer conexion.");
    }
    return nss;
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


int main(int argc, char **argv) {

    int sistemaActivo=1;
    printf("Soy proceso Suscriptor\n");

    if(argc!=3){ //Verificamos que no nos pongan mas de dos parametros
        SYSERROR(EXIT_FAILURE, "\nError en la cantidad de parámetros");
    }

    // Socket con el Satelite
    int nss; // new server socket
	int r; // received bytes from socket

    int puerto;

    nss = atoi( argv[1] );
    puerto = atoi( argv[2] );
    char buf[BUFSIZE];

    strcpy(buf, mensajeRespuesta(buf, 101));

    if ((r = send(nss, buf, BUFSIZE, 0)) != BUFSIZE) {
            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
    }
    int cs; // client socket
	int r2; // sent bytes to socket
    
    
    cs= crearSocketEscucha(puerto);

    while (sistemaActivo==1){
        // Recibimos de Proceso Publicador
	    if ((r2 = recv(cs, buf, BUFSIZE, 0)) == -1) {
		    SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
	    }

        //Enviamos a proceso Suscriptor
        if ((r = send(nss, buf, BUFSIZE, 0)) != BUFSIZE) {
            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
        }

    }
    
    shutdown(nss, SHUT_RDWR);
    

    return EXIT_SUCCESS;
}