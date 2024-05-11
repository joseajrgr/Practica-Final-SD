#ifndef ALMACENAMIENTO_H
#define ALMACENAMIENTO_H

#define MAX_USERNAME_LENGTH 256
#define MAX_FILE_LENGTH 256

typedef struct{
    int result;
    char* texto;
} Respuesta;

int register_user(char username[MAX_USERNAME_LENGTH]);
int unregister_user(char username[MAX_USERNAME_LENGTH]);
int connect_user(char username[MAX_USERNAME_LENGTH], char ip[16], int port);
int publish_file(char username[MAX_USERNAME_LENGTH], char file_name[MAX_FILE_LENGTH], char description[MAX_FILE_LENGTH]);
Respuesta list_connected_users(char username[MAX_USERNAME_LENGTH]);
int disconnect_user(char username[MAX_USERNAME_LENGTH]);
void remove_user_from_connections_file(char username[MAX_USERNAME_LENGTH]);
Respuesta list_user_content(char username[MAX_USERNAME_LENGTH], char remote_user[MAX_USERNAME_LENGTH]);
int delete_file(char username[MAX_USERNAME_LENGTH], char file_name[MAX_FILE_LENGTH]);
int get_file_info(char username[MAX_USERNAME_LENGTH], char file_name[MAX_FILE_LENGTH], char remote_ip[16], int *remote_port);

#endif
