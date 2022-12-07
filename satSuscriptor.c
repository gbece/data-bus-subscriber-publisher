#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "./modulo/constantes.h"
#include "./modulo/proceso.h"

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

	//char buf[BUFSIZE]="tipo:103\nid:7522\nrol:1\nfiltro:3\ntamaño:16\ndireccion:hola.txt\n\nHOLA COMO ESTAS\r\n\r\n";
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


void crearDir(char directorio[MAX_SIZE]){
	
	char* token;
	struct stat sb;
	char cDir[MAX_SIZE];
	getcwd(cDir, MAX_SIZE);

	
	char *aux = (char *) malloc(sizeof(char) * (strlen(directorio) + 1 + 8 ) );
	*aux = '\0';
    char ruta[MAX_SIZE];
    strcpy(ruta,directorio);
    strcat(aux,"/directorio/");
    strcat(aux, dirname(ruta));

	token=strtok(aux, "/");	
		
	while ( (token != NULL)  ) { 
		while (stat( token , &sb ) == 0  )  {  //Nos fijamos si existe la carpeta
			if (S_ISDIR(sb.st_mode) ) {
			
				if (chdir( token ) == -1){			//Entramos a la carpeta
					SYSERROR(EXIT_FAILURE, "Error al cambiar de directorio.\n");
				}
				token = strtok(NULL, "/"); 
			} else {
				SYSERROR(EXIT_FAILURE, "ERROR, ya existe 'algo' con nombre \n");
			}
		}
		
		if ((token != NULL)){
			if ( ( mkdirat( AT_FDCWD, token , 0755 ) )== -1 || chdir( token ) == -1) {  //Creamos y entramos a la carpeta
				SYSERROR(EXIT_FAILURE, "Error al crear la carpeta de origen.\n");
			}
		} 
		token = strtok(NULL, "/"); 
	}
	
	chdir (cDir);
	getcwd(cDir, MAX_SIZE);
	
	free(aux);
}

void crearArc( char directorio[MAX_SIZE], char cuerpo[MAX_ARCHIVO]) {
	int destino;

	char *aux = (char *) malloc(sizeof(char) * (strlen(directorio) + 1 + 8 ) );
	*aux = '\0';
	char cDir[MAX_SIZE];
	char * nomArchivo= basename(directorio);

    strcpy(directorio, dirname(directorio));
    strcpy(aux,"/directorio/");
    strcat(aux,directorio);
    strcat(aux,"/");
    strcat(aux,nomArchivo);

	getcwd(cDir, MAX_SIZE);
	strcat(cDir,aux);
    long size=strlen(cuerpo);

    if ((destino = open(cDir, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
		SYSERROR(EXIT_FAILURE,  "Error al abrir archivo de destino.\n");
	}
    write(destino, cuerpo, size);

    close(destino);

	free(aux);
}

char *crearSaludo(char *mensajeSaludo, int filtro){
    char mensajeAux[BUFSIZE];
    strcpy(mensajeSaludo,"tipo:100\nid:");// 100 es para realizar solicitud
    sprintf(mensajeAux, "%d",getpid());
    strcat(mensajeSaludo,mensajeAux);
    strcat(mensajeSaludo, "\nrol:2\nfiltro:");
    sprintf(mensajeAux, "%d",filtro);
    strcat(mensajeSaludo,mensajeAux);
    strcat(mensajeSaludo, "\ntamaño:0\ndireccion:0\n\n-\r\n\r\n");
    return mensajeSaludo;
}


char *obtenerDireccion(char *direccion, char header[]){
    int cont = 0;
    char *aux = (char *) malloc(sizeof(char) * (strlen(header) + 1) );
    strcpy(aux, header);
    direccion = strtok(aux, ":");
    direccion = strtok(NULL, "\n");
    cont++;
    while(cont <= 5){
        direccion = strtok(NULL, ":");
        direccion = strtok(NULL, "\n");
        cont++;
    }
    free(aux);
    return direccion;
}

char *obtenerCuerpo(char *cuerpo, char header[]){
    int cont = 0;
    char *aux = (char *) malloc(sizeof(char) * (strlen(header) + 1) );
    strcpy(aux, header);
    cuerpo = strtok(aux, ":");
    cuerpo = strtok(NULL, "\n");
    cont++;
    while(cont <= 5){
        cuerpo = strtok(NULL, ":");
        cuerpo = strtok(NULL, "\n");
        cont++;
    }
    cuerpo = strtok(NULL, "\n\n");
    free(aux);
    return cuerpo;
}
    


int crearSocketCliente(){
    int cs; // client socket

    int conectado=-1;
    int puerto=PPADRE_PORT;

    struct sockaddr_in srvaddr; // server address
    
    // crear socket
	if ((cs = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		SYSERROR(EXIT_FAILURE, "Error al crear el socket servidor.");
	}

	srvaddr.sin_family = AF_INET;
    
	srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while(conectado==-1 && puerto < (PPADRE_PORT+20)){
        
        srvaddr.sin_port = htons(puerto);
	    conectado= connect(cs, (struct sockaddr *) &srvaddr,(socklen_t) sizeof(struct sockaddr_in));
        //Esperamos a que conecte

        if (conectado==-1){
            puerto=puerto+1;
        }
        
        if (puerto == PPADRE_PORT+20){
            printf("En este momento no contamos con puertos disponibles\nIntente mas tarde.\n");
        }
	}

    return cs;
}


int main(int argc, char **argv){

    int filtroSaludo = atoi(argv[1]);
    char buf[BUFSIZE];
    int tipo;
    struct proceso prs;
    char cDir[MAX_SIZE];  
    char cuerpo[MAX_ARCHIVO];   


    if(argc != 2 ){ //Verificamos que no nos pongan mas de dos parametros
        SYSERROR(EXIT_FAILURE, "\nError en la cantidad de parámetros. Debe especificar el tipo de suscripción");
    }
    
    printf("Bienvenido a Consola Suscriptor\n");
    //ESTABLECEMOS CONEXION CON PROCESO PADRE (SOCKETS, ETC)
    int cs; // client socket
	int r; // sent bytes to socket
    cs=crearSocketCliente();

    printf("Conexion Establecida\n");

    // ENVIAR MENSAJE DE HOLA
    
    strcpy(buf, crearSaludo(buf, filtroSaludo));

    if ((r = send(cs, buf, BUFSIZE, 0)) != BUFSIZE) {
		SYSERROR(EXIT_FAILURE, "Error al escribir en el socket.");
	}  

    // Espero el OK
	if ((r = recv(cs, buf, BUFSIZE, 0)) == -1) {
		SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
	}

    tipo=validacion(buf, &prs); // Validamos 

    if (tipo ==101){
        printf("Conexion establecida exitosamente\n");
    }else{
        SYSERROR(EXIT_FAILURE, "Error para establecer conexion.\n");
    }

    

    //ENVIAMOS PUBLICACION
    

    while ( tipo != 301 ){  // 301 Desconexion

        if ((r = recv(cs, buf, BUFSIZE, 0)) == -1) {
		    SYSERROR(EXIT_FAILURE, "Error al leer del socket.");
	    }

        tipo=validacion(buf, &prs); // Validamos 

        if ( tipo == 103){
            strcpy(cDir, obtenerDireccion(cDir, buf));
            strcpy(cuerpo, obtenerCuerpo(cuerpo, buf));
            crearDir( cDir );
            crearArc(cDir ,  cuerpo);
        }
        
        
    }
    



    return(EXIT_SUCCESS);
}