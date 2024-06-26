/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _IMPRESION_H_RPCGEN
#define _IMPRESION_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif


struct entrada {
	char *user;
	int operacion;
	char *file;
	char *datetime;
};
typedef struct entrada entrada;

#define RPC_PROG 99
#define RPC_VERS 1

#if defined(__STDC__) || defined(__cplusplus)
#define IMPRIMIR 0
extern  void * imprimir_1(entrada , CLIENT *);
extern  void * imprimir_1_svc(entrada , struct svc_req *);
extern int rpc_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define IMPRIMIR 0
extern  void * imprimir_1();
extern  void * imprimir_1_svc();
extern int rpc_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_entrada (XDR *, entrada*);

#else /* K&R C */
extern bool_t xdr_entrada ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_IMPRESION_H_RPCGEN */
