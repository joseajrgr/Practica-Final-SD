#include <unistd.h>
#include <errno.h>
#include "../include/transferencias.h"
#include "../rpc/impresion.h"

int sendMessage(int socket, char * buffer, int len)
{
	int r;
	int l = len;
		
	do {	
		r = write(socket, buffer, l);
		l = l -r;
		buffer = buffer + r;
	} while ((l>0) && (r>=0));
	
	if (r < 0)
		return (-1);   /* fail */
	else
		return(0);	/* full length has been sent */
}

int recvMessage(int socket, char *buffer, int len)
{
	int r;
	int l = len;
		
	do {	
		r = read(socket, buffer, l);
		l = l -r ;
		buffer = buffer + r;
	} while ((l>0) && (r>=0));
	
	if (r < 0)
		return (-1);   /* fallo */
	else
		return(0);	/* full length has been receive */
}

ssize_t readLine(int fd, void *buffer, size_t n)
{
	ssize_t numRead;  /* num of bytes fetched by last read() */
	size_t totRead;	  /* total bytes read so far */
	char *buf;
	char ch;

	if (n <= 0 || buffer == NULL) { 
		errno = EINVAL;
		return -1; 
	}

	buf = buffer;
	totRead = 0;
	
	for (;;) {
        numRead = read(fd, &ch, 1);	/* read a byte */

        if (numRead == -1) {	
            if (errno == EINTR)	/* interrupted -> restart read() */
                continue;
            else
				return -1;		/* some other error */
        } else if (numRead == 0) {	/* EOF */
            if (totRead == 0)	/* no byres read; return 0 */
                return 0;
			else
                break;
        } else {			/* numRead must be 1 if we get here*/
            if (ch == '\n')
                break;
            if (ch == '\0')
                break;
            if (totRead < n - 1) {		/* discard > (n-1) bytes */
				totRead++;
				*buf++ = ch; 
			}
		} 
	}
	
	*buf = '\0';
    	return totRead;
}

void envio_rpc(int32_t operacion, char username[MAX_USERNAME_LENGTH], char datetime[MAX_DATETIME_LENGTH]) {
	// Variables correspondientes a RPC
    CLIENT *clnt;
	void *resultado_rpc;
	entrada mensaje_rpc;
    char *host = "127.0.0.1";

	// Genera el cliente (TCP)
    clnt = clnt_create (host, RPC_PROG, RPC_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror (host);
        exit (1);
    }

	// Almacena en la estructura "mensaje" los valores que se van a mandar
    mensaje_rpc.operacion = operacion;
    mensaje_rpc.user = malloc(strlen(username));
    strcpy(mensaje_rpc.user, username);
    mensaje_rpc.file = "";
    mensaje_rpc.datetime = malloc(strlen(datetime));
    strcpy(mensaje_rpc.datetime, datetime);

	// Llama a la función y destruye al cliente después
    resultado_rpc = imprimir_1(mensaje_rpc, clnt);
    if (resultado_rpc == (void *) NULL) {
        clnt_perror (clnt, "call failed");
    }
    clnt_destroy (clnt);
}

void envio_rpc_con_fichero(int32_t operacion, char username[MAX_USERNAME_LENGTH], 
                            char datetime[MAX_DATETIME_LENGTH], char file[MAX_FILE_LENGTH]) {
	// Variables correspondientes al RPC
    CLIENT *clnt;
	void *resultado_rpc;
	entrada mensaje_rpc;
    char *host = "127.0.0.1";

    // Genera un cliente rpc (TCP)
    clnt = clnt_create (host, RPC_PROG, RPC_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror (host);
        exit (1);
    }

	// Almacena en la estructura "mensaje" los valores que se van a mandar
    mensaje_rpc.operacion = operacion;
    mensaje_rpc.user = malloc(strlen(username));
    strcpy(mensaje_rpc.user, username);
    mensaje_rpc.file = malloc(strlen(file));
    strcpy(mensaje_rpc.file, file);
    mensaje_rpc.datetime = malloc(strlen(datetime));
    strcpy(mensaje_rpc.datetime, datetime);

	// Llama a la función y después destruye al cliente
    resultado_rpc = imprimir_1(mensaje_rpc, clnt);
    if (resultado_rpc == (void *) NULL) {
        clnt_perror (clnt, "call failed");
    }
    clnt_destroy (clnt);
}
