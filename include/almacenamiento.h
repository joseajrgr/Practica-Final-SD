#ifndef ALMACENAMIENTO_H
#define ALMACENAMIENTO_H

#define MAX_USERNAME_LENGTH 256

int register_user(char username[MAX_USERNAME_LENGTH]);
int unregister_user(char username[MAX_USERNAME_LENGTH]);
int connect_user(char username[MAX_USERNAME_LENGTH], char ip[16], int port);

#endif
