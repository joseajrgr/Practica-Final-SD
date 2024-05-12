#ifndef SOCKETS_H
#define SOCKETS_H

#include <stdio.h>
#include <arpa/inet.h>

#define MAX_DATETIME_LENGTH 20
#define MAX_USERNAME_LENGTH 256
#define MAX_FILE_LENGTH 256

int sendMessage(int socket, char * buffer, int len);
int recvMessage(int socket, char *buffer, int len);
ssize_t readLine(int fd, void *buffer, size_t n);
void envio_rpc(int32_t operacion, char username[MAX_USERNAME_LENGTH], char datetime[MAX_DATETIME_LENGTH]);
void envio_rpc_con_fichero(int32_t operacion, char username[MAX_USERNAME_LENGTH], char datetime[MAX_DATETIME_LENGTH], char file[MAX_FILE_LENGTH]);

#endif
