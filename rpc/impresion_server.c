#include "impresion.h"

void *
imprimir_1_svc(entrada arg1,  struct svc_req *rqstp)
{
    static char * result;

    // Comprobar la operación
    if (arg1.operacion == 0) {
        printf("%s    REGISTER    %s\n", arg1.user, arg1.datetime);
        fflush(stdout);
    } else if (arg1.operacion == 1) {
        printf("%s    UNREGISTER    %s\n", arg1.user, arg1.datetime);
        fflush(stdout);
	} else if (arg1.operacion == 2) {
        printf("%s    CONNECT    %s\n", arg1.user, arg1.datetime);
        fflush(stdout);
	} else if (arg1.operacion == 3) {
        printf("%s    DISCONNECT    %s\n", arg1.user, arg1.datetime);
        fflush(stdout);
	} else if (arg1.operacion == 4) {
        printf("%s    PUBLISH %s    %s\n", arg1.user, arg1.file, arg1.datetime);
        fflush(stdout);
	} else if (arg1.operacion == 5) {
        printf("%s    DELETE %s    %s\n", arg1.user, arg1.file, arg1.datetime);
        fflush(stdout);
	} else if (arg1.operacion == 6) {
        printf("%s    LIST_USERS    %s\n", arg1.user, arg1.datetime);
        fflush(stdout);
	} else if (arg1.operacion == 7) {
        printf("%s    LIST_CONTENT    %s\n", arg1.user, arg1.datetime);
        fflush(stdout);
    } else {
        printf("Los datos no están en el formato esperado\n");
        fflush(stdout);
    }

    return (void *) &result;
}
