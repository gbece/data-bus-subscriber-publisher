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

#include "./modulo/constantes.h"
#include "./modulo/shm.h"
#include "./modulo/memoria.h"
#include "./modulo/proceso.h"

////
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/ip.h>

////


int detenerSistema(void *shmp, sem_t *sem){
	int aux=1;
    
	//Entra a memoria compartida para chequear si el sistema esta activo
	struct memoria mem;
	
	sem_wait(sem);
	//---------operaciones sobre memoria compartida---------
	memcpy(&mem, shmp, sizeof(struct memoria));   //Leemos de memoria

    mem.estado=0; //   Damos de Baja Sistema

	memcpy(shmp, &mem, sizeof(struct memoria));   //Guardamos en Memoria
	//------------------------------------------------------
	sem_post(sem);

	aux=mem.estado;

	return aux;	
}


char *procesosActivos(void *shmp, sem_t *sem, char *aux){
    
    
	char aux2[BUFSIZE];
    //Entra a memoria compartida para chequear si el sistema esta activo
	struct memoria mem;

    
    ////-----////
	
	sem_wait(sem);
	//---------operaciones sobre memoria compartida---------
	memcpy(&mem, shmp, sizeof(struct memoria));   //Leemos de memoria
    //------------------------------------------------------
	sem_post(sem);

    strcpy(aux, "\n\n*Total de procesos conectados: \n");
    for(int i=0;i<=20;i++){
        if (mem.process[i].estado==1){
            strcat(aux, "- Proceso: ");
            sprintf(aux2, "%d", mem.process[i].id);
            strcat(aux, aux2);

            strcat(aux, " con el rol ");
            if ( mem.process[i].rol == 1){
                strcat(aux, "Publicador");
            }else if( mem.process[i].rol == 2){
                strcat(aux, "Suscriptor");
            }else{
                sprintf(aux2, "%d", mem.process[i].rol);
                strcat(aux, aux2);
            }
            
            strcat(aux, "\n");
            
        }
    }
    strcat(aux,"\r\n\r\n");
	
	return aux;	
}

char *mensajesEnviadosYrecibidos(void *shmp, sem_t *sem, char *aux){
    
	char aux2[BUFSIZE];
    //Entra a memoria compartida para chequear si el sistema esta activo
	struct memoria mem;

	sem_wait(sem);
	//---------operaciones sobre memoria compartida---------
	memcpy(&mem, shmp, sizeof(struct memoria));   //Leemos de memoria
    //------------------------------------------------------
    sem_post(sem);
    strcpy(aux, "\n\n*Bytes Enviados y recibidos: \n");
    for(int i=0;i<=20;i++){
        if (mem.process[i].estado==1){
            strcat(aux, "- Proceso: ");
            sprintf(aux2, "%d", mem.process[i].id);
            strcat(aux, aux2);
            strcat(aux, " envió ");
            sprintf(aux2, "%d", mem.process[i].mensajesEnviados);
            strcat(aux, aux2);
            strcat(aux, " bytes y recibió ");
            sprintf(aux2, "%d", mem.process[i].mensajesRecibidos);
            strcat(aux, aux2);
            strcat(aux, " mensajes\n");
        }
    }
    strcat(aux,"\r\n\r\n");
	
	
	return aux;	
}

//Función de prueba por shm
//RETORNA UN CUERPO DE MSJ
char *bytesEnviadosYrecibidos(void *shmp, sem_t *sem, char *aux){

    char aux2[BUFSIZE];
	struct memoria mem; 

	sem_wait(sem);
	//---------operaciones sobre memoria compartida---------
	memcpy(&mem, shmp, sizeof(struct memoria));   //Leemos de memoria
    //------------------------------------------------------
	sem_post(sem);
    strcpy(aux, "\n\n*Bytes Enviados y recibidos: \n");
    for(int i=0;i<20;i++){
        if (mem.process[i].estado==1){
            strcat(aux, "- Proceso: ");
            sprintf(aux2, "%d", mem.process[i].id);
            strcat(aux, aux2);

            strcat(aux, " envió ");
            sprintf(aux2, "%d", mem.process[i].bytesEnviados);
            strcat(aux, aux2);
            strcat(aux, " bytes y recibió ");
            sprintf(aux2, "%d", mem.process[i].bytesRecibidos);
            strcat(aux, aux2);
            strcat(aux, " bytes\n");
        }    
    }
	
    strcat(aux,"\r\n\r\n");
	return aux;	
}

char *crearCabecera(char *cabecera, int tipo, int rol){
    char mensajeAux[BUFSIZE];

    strcpy(cabecera,"tipo:");
    sprintf(mensajeAux, "%d", tipo);
    strcat(cabecera, mensajeAux);

    strcat(cabecera, "\nid:");
    sprintf(mensajeAux, "%d",getpid());
    strcat(cabecera,mensajeAux);
    
    strcpy(mensajeAux, "\nrol:");
    strcat(cabecera,mensajeAux);
    
    sprintf(mensajeAux, "%d", rol);
    strcat(cabecera,mensajeAux);
    strcat(cabecera, "\nfiltro:0\ntamaño:0\ndireccion:0");
    return cabecera;
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


int validarConsulta(char header[]){  //Validamos el header para saber el rol que tiene
    char *token;
    int tipoAux; 
    int rolAux;
    int opc = -4;
    int cont = 0;

    //Primer token para determinar tipo de msj...
    token = strtok(header, ":");
    token = strtok(NULL, "\n");
    tipoAux=atoi(token);
    if(tipoAux != 100 || tipoAux != 102){
        opc = -1;
    }
    cont++;
    while(cont <= 5){
        token = strtok(NULL, ":");
        token = strtok(NULL, "\n");
        cont++;
        if(cont == 3){          //Posicionado en Rol
            rolAux=atoi(token);
            if(rolAux != 4){
                //printf("ERROR EN ROL DE MSJ!!!\n");
                cont = 6;       //salgo del while
                opc = -2;
                //tipoAux = -1;
            }
        }
    }
    if(tipoAux == 100 && rolAux == 4){
        opc = 0;
    }else if(tipoAux == 102 && rolAux == 4){
        token = strtok(NULL, "\n\n");
        opc=atoi(token);
        cont++;
        if (opc < 1 || opc > 5){
            opc = -3;
            //printf("ERROR EN OPCIÓN (cuerpo) DE MENSAJE!!!\n");
        }
    }
    if(opc == -1){
        printf("ERROR EN TIPO DE MENSAJE!!!\n");
    }else if(opc == -2){
        printf("ERROR EN ROL DE MENSAJE!!!\n");
    }else if(opc == -3){
        printf("ERROR EN OPCIÓN (cuerpo) DE MENSAJE!!!\n");
    }
    return opc;
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


int main(int argc, char **argv) {   

    if(argc>1){ //Verificamos que no nos pongan mas de 1 parametro
        SYSERROR(EXIT_FAILURE, "\nError en la cantidad de parámetros");
    }
    int varAux = 0;     //para controlar bucle ping pong

    ////  SHM Y SEMAFORO
    void *shmp;
    key_t myKey = getProjId( FTOK_FILE, PROJ_ID );
        
    if ((shmp=obtenerSHM(FTOK_FILE, PROJ_ID, SHM_SIZE,myKey, FALSE ))== NULL){
		SYSERROR(EXIT_FAILURE, "Fallo en obtener SHM");
	}

    sem_t *sem=obetenerSemaforo(SEM_NAME, 1 , FALSE);


    ///SOCKET
	int nss; // new server socket
	int r; // received bytes from socket
	nss = crearSocketEscucha();


	char buf[BUFSIZE];
	char cabecera[BUFSIZE];
	char cuerpo[BUFSIZE];
    //--
    int estadoSistema;
    //--

    // operar con el socket
    if ((r = recv(nss, buf, BUFSIZE, 0)) == -1) {
        SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
    }

    int opcion = validarConsulta(buf); //Validamos
    
    if(opcion == 0){
        

        // operar con el socket
        strcpy(buf, "Conexión con ADMINISTRADOR establecida\n");
        if ((r = send(nss, buf, BUFSIZE, 0)) != BUFSIZE) {
            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
        }
    }
    sleep(1);

    while(varAux != -1){
        // operar con el socket
        if ((r = recv(nss, buf, BUFSIZE, 0)) == -1) {
            SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
        }

        opcion = validarConsulta(buf);
        
        if(opcion == 1){
            

            strcpy(buf, mensajeRespuesta(buf, 104));
            
            estadoSistema = detenerSistema(shmp, sem);
            printf("%d", estadoSistema);
            
            
            if ((r = send(nss, buf, BUFSIZE, 0)) != BUFSIZE) {
                SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
            }
            opcion=5;
            
        }else if(opcion == 2){
           
            strcpy(cabecera, crearCabecera(cabecera, 104, 5)); 
            strcpy(cuerpo, procesosActivos(shmp, sem, cuerpo));

            strcat(cabecera, cuerpo);
            strcpy(buf, cabecera);
            if ((r = send(nss, buf, BUFSIZE, 0)) != BUFSIZE) {
            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
            }
            
        }else if(opcion == 3){
            
            strcpy(cabecera, crearCabecera(cabecera, 104, 5)); 
            strcpy(cuerpo, mensajesEnviadosYrecibidos(shmp, sem, cuerpo));

            strcat(cabecera, cuerpo);
            strcpy(buf, cabecera);
            if ((r = send(nss, buf, BUFSIZE, 0)) != BUFSIZE) {
            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
            }
            
        }else if(opcion == 4){
            
            strcpy(cabecera, crearCabecera(cabecera, 104, 5)); 
            strcpy(cuerpo, bytesEnviadosYrecibidos(shmp, sem, cuerpo));

            strcat(cabecera, cuerpo);
            strcpy(buf, cabecera);
            if ((r = send(nss, buf, BUFSIZE, 0)) != BUFSIZE) {
            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
            }
            
        }
        if(opcion == 5){
            
            varAux = -1;
            // cerrar conexion
	        shutdown(nss, SHUT_RDWR);
        }

    }
    return EXIT_SUCCESS;
}