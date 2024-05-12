/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "impresion.h"

void *
imprimir_1_svc(entrada arg1,  struct svc_req *rqstp)
{
	static char * result;
	int operacion = arg1.operacion;

	// Comprobar la operación
	if (operacion == 0) {
		result = "REGISTER";
	} else if (operacion == 1) {
		result = "UNREGISTER";
	} else {
		result = "Operación no válida";
	}

	return (void *) &result;
}