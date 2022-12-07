#ifndef _CONSTANTES_H_
#define _CONSTANTES_H_

#define TRUE 1
#define FALSE 0

#define BUFSIZE 1124
#define MAX_ARCHIVO 1024
#define MAX_SIZE 100

#define VAL_EN_SHM 12


#define ADMIN_PORT 9999
#define PPADRE_PORT 8000
#define PRSPORT 8040  


#define MAXCONN 10


#define SYSERROR(status, msg) error_at_line(status, errno, __FILE__, __LINE__, msg)

#endif

