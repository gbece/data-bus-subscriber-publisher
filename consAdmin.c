#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include "./modulo/constantes.h"
#include "./modulo/proceso.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/*#define SYSERROR(status, msg) error_at_line(status, errno, __FILE__, __LINE__, msg)
#define SRVPORT 8000
#define MAXCONN 10
#define BUFSIZE 1024
*/
//char buf[BUFSIZE]="tipo:100\nid:x?x?x?\nrol:4\nfiltro:0\ntamaño:0\ndireccion:0\n\n0\r\n\r\n";

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



char *crearSaludoCons(char *mensajeSaludo){
    char mensajeAux[BUFSIZE];
    strcpy(mensajeSaludo,"tipo:100\nid:");// 100 es saludo
    sprintf(mensajeAux, "%d",getpid());
    strcat(mensajeSaludo,mensajeAux);
    strcat(mensajeSaludo, "\nrol:4\nfiltro:0\ntamaño:0\ndireccion:0\n\n-\r\n\r\n");
    return mensajeSaludo;
}
//CORREGIR FUNCION obtenerCuerpo en consAdmin!!!!
char *obtenerCuerpo(char *cuerpo, char header[]){
    int cont = 0;
    char *aux = (char *) malloc(sizeof(char) * (strlen(header) + 1) );
    //printf("header es: %s\n", header);
    strcpy(aux, header);
    //printf("aux es: %s\n", aux);
    cuerpo = strtok(aux, ":");
    cuerpo = strtok(NULL, "\n");
    cont++;
    //printf("cont=%d es: %s\n", cont, cuerpo);
    while(cont <= 6){
        cuerpo = strtok(NULL, ":");
        cuerpo = strtok(NULL, "\n");
        cont++;
        //printf("cont=%d es: %s\n", cont, cuerpo);
    }
    cuerpo = strtok(NULL, "*");     //Funciona pero me come el titulo del cuerpo
    free(aux);
    return cuerpo;
}


int main(int argc, char **argv){
    int opcion = 0;
    int val;
    struct proceso prs;
    int tipo;

    char mensaje[BUFSIZE];
    char mensajeAux[BUFSIZE];
    //////////////////////////
    int cs; // client socket
	int r; // sent bytes to socket
	
	char buf[BUFSIZE];    //voy a usar msj!!!
    char cuerpo[BUFSIZE];
///////////////////////////////////////////////////////
   if(argc>1){ //Verificamos que no nos pongan mas de 1 parametro
        SYSERROR(EXIT_FAILURE, "\nError en la cantidad de parámetros");
    }
    
    cs=crearSocketCliente();
	
    strcpy(buf, crearSaludoCons(buf));
    
    
	if ((r = send(cs, buf, BUFSIZE, 0)) != BUFSIZE) {
		SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
	}

	// operar con el socket
	if ((r = recv(cs, buf, BUFSIZE, 0)) == -1) {
		SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
	}

    ///

    printf("BIENVENIDO A LA CONSOLA ADMINSITRADOR\n");

    while(opcion != 5){
        strcpy(mensaje,"tipo:102\nid:");// 102 es para realizar solicitud
        sprintf(mensajeAux, "%d",getpid());
        strcat(mensaje,mensajeAux);
        strcat(mensaje, "\nrol:4\nfiltro:0\ntamaño:0\ndireccion:0\n\n");    //Rol: 4 (consola)
        
        printf("Ingrese la opción deseada:\n1- Detener servidor central\n2- Procesos conectados y rol de cada uno\n3- Cantidad de mensajes que recibe y/o envía cada proceso\n4- Cantidad de bytes enviados y recibidos por cada proceso\n5- Salir\n");
        val = scanf("%d", &opcion);
        if(val <= 0 || opcion < 1 || opcion >5){
            printf("ERROR. Ingrese una opción válida\n");
            while(getchar() != '\n');
            
        }else{
            switch(opcion){
            case 1:
                //printf("Ingreso a opc %d\n\n", opcion);
                strcat(mensaje,"1");
                strcat(mensaje, "\r\n\r\n");
                strcpy(buf, mensaje);
                
                if ((r = send(cs, buf, BUFSIZE, 0)) != BUFSIZE) {
		            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
	            }
                
                if ((r = recv(cs, buf, BUFSIZE, 0)) == -1) {
		            SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
	            }

                tipo=validacion(buf, &prs); // Validamos 

                if (tipo == 104){
                    printf("Sistema detenido\n");
                    opcion=5;
                }
                //Falta detener el sistema propiamente.. que hay que hacer??
                break;

            case 2:
                strcat(mensaje,"2");
                strcat(mensaje, "\r\n\r\n");

                strcpy(buf, mensaje);
                
                if ((r = send(cs, buf, BUFSIZE, 0)) != BUFSIZE) {
		            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
	            }
                
                if ((r = recv(cs, buf, BUFSIZE, 0)) == -1) {
		            SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
	            }
                strcpy(cuerpo, obtenerCuerpo(cuerpo, buf)); 
                printf("\n\n\n\n\n\n\n\n\n\nProcesos activos conectados al sistema:\n"); 
	            printf("%s\n\n", cuerpo);
                break;

            case 3:
                
                strcat(mensaje,"3");
                strcat(mensaje, "\r\n\r\n");
                strcpy(buf, mensaje);
                
                if ((r = send(cs, buf, BUFSIZE, 0)) != BUFSIZE) {
		            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
	            }
                if ((r = recv(cs, buf, BUFSIZE, 0)) == -1) {
		            SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
	            }
                strcpy(cuerpo, obtenerCuerpo(cuerpo, buf));
                printf("\n\n\n\n\n\n\n\n\n\nCantidad de Mensajes enviados y Recibidos por proceso:\n"); 
	            printf("%s\n\n", cuerpo);
                //printf("Mensaje Enviado: \n%s\n", mensaje);
                break;

            case 4:
                
                strcat(mensaje,"4");
                strcat(mensaje, "\r\n\r\n");
                strcpy(buf, mensaje);
                
                if ((r = send(cs, buf, BUFSIZE, 0)) != BUFSIZE) {
		            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
	            }
                
                if ((r = recv(cs, buf, BUFSIZE, 0)) == -1) {
		            SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
	            }
                strcpy(cuerpo, obtenerCuerpo(cuerpo, buf)); 
                printf("\n\n\n\n\n\n\n\n\n\nCantidad de Bytes enviados y Recibidos por proceso:\n");
	            printf("%s\n\n", cuerpo);
                //printf("Mensaje Enviado: \n%s\n", mensaje);
                break;

            case 5:
                strcat(mensaje,"5");
                strcat(mensaje, "\r\n\r\n");
                strcpy(buf, mensaje);
                if ((r = send(cs, buf, BUFSIZE, 0)) != BUFSIZE) {
		            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
	            }
                break;
            }

        }     
    }
 
//////////////////////////////////////////////////////
// cerrar conexion
	shutdown(cs, SHUT_RDWR);

//////////////////////////
     
    return(EXIT_SUCCESS);
}