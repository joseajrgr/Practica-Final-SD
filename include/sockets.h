#ifndef SOCKETS_H
#define SOCKETS_H

int sendMessage(int socket, char * buffer, int len);
int recvMessage(int socket, char *buffer, int len);

#endif
