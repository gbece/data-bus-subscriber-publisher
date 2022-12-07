#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "./modulo/constantes.h"
#include "./modulo/proceso.h"

char * publicacion(FILE *archivo, char* mensaje, char *cDir,  int filtro ){

    char *mensajeAux = (char *) malloc(sizeof(char) * (BUFSIZE + 1) );

    char *buf = malloc(BUFSIZE);
            
    fseek(archivo, 0, SEEK_END); 
    long fsize = ftell(archivo);
    fseek(archivo, 0, SEEK_SET); 
            
    fread(buf, BUFSIZE, 1, archivo);
    fclose(archivo);

    strcpy(mensaje,"tipo:103\nid:");// 103 es para enviar publicacion
    sprintf(mensajeAux, "%d",getpid() );
    strcat(mensaje,mensajeAux);
    strcat(mensaje, "\nrol:1\nfiltro:");
    sprintf(mensajeAux, "%d",filtro );
    strcat(mensaje,mensajeAux);
    strcat(mensaje, "\ntamaño:");
    sprintf(mensajeAux, "%ld", fsize);
    strcat(mensaje,mensajeAux);
    strcat(mensaje, "\ndireccion:");
    strcat(mensaje,cDir);
    strcat(mensaje, "\n\n");
    strcat(mensaje, buf);
    strcat(mensaje, "\r\n\r\n");

    
    free (mensajeAux);
    free (buf);

    return mensaje;
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


char *crearSaludo(char *mensajeSaludo, int filtro){
    char mensajeAux[BUFSIZE];
    strcpy(mensajeSaludo,"tipo:100\nid:");// 100 es para realizar solicitud
    sprintf(mensajeAux, "%d",getpid());
    strcat(mensajeSaludo,mensajeAux);
    strcat(mensajeSaludo, "\nrol:1\nfiltro:");
    sprintf(mensajeAux, "%d",filtro);
    strcat(mensajeSaludo,mensajeAux);
    strcat(mensajeSaludo, "\ntamaño:0\ndireccion:0\n\n-\r\n\r\n");

    return mensajeSaludo;
}

int crearSocketCliente(){
    int cs; // client socket

    int puerto=PPADRE_PORT+1;

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

int main(int argc, char **argv){


    char mensaje[BUFSIZE];
    char cDir[MAX_SIZE];
    struct proceso prs;
    int tipo;
    int bandera;
    int filtro;

    if(argc!=2 ){ //Verificamos que no nos pongan mas de dos parametros
        SYSERROR(EXIT_FAILURE, "\nError en la cantidad de parámetros");
    }

    //ESTABLECEMOS CONEXION CON PROCESO PADRE (SOCKETS, ETC)
    int cs; // client socket
	int r; // sent bytes to socket
    cs=crearSocketCliente();



    // ENVIAR MENSAJE DE HOLA, SERIA DEL TIPO: "tipo:100\nid:PID+IP\nrol:1\nfiltro:argv[1]\ntamaño:0\ndireccion:\n\n\r\n\r\n"
    strcpy(mensaje, crearSaludo(mensaje, atoi(argv[1]) ) );

    if ((r = send(cs, mensaje, BUFSIZE, 0)) != BUFSIZE) {
		SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
	}  

    // Espero el OK
	if ((r = recv(cs, mensaje, BUFSIZE, 0)) == -1) {
		SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
	}

    tipo = validacion(mensaje, &prs);

    if ( tipo != 101 ){
        printf("Error en el mensaje\n");
    }else{
        printf("Conexion establecida correctamente\n");
    }

    //ENVIAMOS PUBLICACION
    printf("Ingrese la direccion del archivo que desea Publicar sino escriba: Salir\n");
	scanf("%s", cDir);

    while ( strcmp(cDir, "Salir") ){
        
        bandera=0;
        
        
        FILE *archivo ;
        if ((archivo = fopen(cDir, "rb")) == NULL) {
            
            printf("Error al abrir archivo especificado: %s\n", cDir );
        }else{
            filtro=atoi(argv[1]);
            strcpy(mensaje, publicacion(archivo,  mensaje, cDir, filtro ) );

            if ( (strstr(mensaje, ".txt") != NULL )  && filtro ==1 ){
                //Es texto
                bandera=1;
            }else if( (strstr(mensaje, ".png") != NULL )  && filtro ==2 ){
                bandera=1;
            }else if ( filtro ==3 ){
                bandera=1;
            }else{
                bandera=0;
            }
            if ( bandera ) {
                if ((r = send(cs, mensaje, BUFSIZE, 0)) != BUFSIZE) {
		            SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
	            } 
            }
            
            
        }
        if ( bandera ) {
            if ((r = recv(cs, mensaje, BUFSIZE, 0)) == -1) {
                SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
            }
            tipo=validacion(mensaje, &prs); // Validamos 
            
            if (tipo == 101){
                printf("Publicacion enviada Exitosamente\n");
            }else if (tipo == 200){
                printf("Error de Conexion\n");
            }else if (tipo == 201){
                printf("Error de Solicitud\n");
            }else if (tipo == 202){
                printf("Error de Rol\n");
            }else{
                printf("Error en el protocolo\n");
            }
        }else{
            printf("El filtro no corresponde con lo subido\n");
        }
        printf("Ingrese la direccion del archivo que desea Publicar sino escriba Salir\n");
	    scanf("%s", cDir);
    }
    

    return(EXIT_SUCCESS);
}