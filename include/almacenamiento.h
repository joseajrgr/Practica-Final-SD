#ifndef ALMACENAMIENTO_H
#define ALMACENAMIENTO_H

#define MAX_USERNAME_LENGTH 256
#define MAX_FILE_LENGTH 256

int register_user(char username[MAX_USERNAME_LENGTH]);
int unregister_user(char username[MAX_USERNAME_LENGTH]);
int connect_user(char username[MAX_USERNAME_LENGTH], char ip[16], int port);
int publish_file(char username[MAX_USERNAME_LENGTH], char file_name[MAX_FILE_LENGTH], char description[MAX_FILE_LENGTH]);
#endif
