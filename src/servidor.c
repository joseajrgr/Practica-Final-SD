#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../include/almacenamiento.h"

#define MAX_USERNAME_LENGTH 256
#define MAX_FILE_LENGTH 256
#define CONNECTIONS_FILE "conexiones.txt"


// Estructura para pasar argumentos al hilo del cliente
struct client_thread_args {
    int socket;
};

void *handle_client(void *args) {
    struct client_thread_args *client_args = (struct client_thread_args *)args;
    int client_socket = client_args->socket;
    int32_t operacion = -1;
    char username[MAX_USERNAME_LENGTH];
    int result = 0;
    char buffer[4];
    char datetime[20];
    
    // Recibir código de operación
    if (recv(client_socket, buffer, sizeof(int32_t), 0) == -1) {
        perror("s> Error al recibir el código de operación");
        result = 2;

    } else {
        operacion = ntohl(*(int*)buffer);
        
        printf("s> Operación: %d\n", operacion);
        
        // Recibir la fecha y la hora
        if (recv(client_socket, datetime, 20, 0) == -1) {
            perror("s> Error al recibir la fecha y la hora");
            result = 2;
        }
        printf("s> Momento en el que se hizo el envío: %s\n", datetime);

        // Recibir nombre de usuario del cliente
        if (recv(client_socket, username, sizeof(char[MAX_USERNAME_LENGTH]), 0) == -1) {
            perror("s> Error al recibir el nombre del cliente");
            result = 2;
        }
        printf("s> %d FROM %s\n", operacion, username);
        
        // Verificar si la operación es REGISTER
        if (operacion == 0) {
            result = register_user(username);

        // Verificar si la operación es UNREGISTER
        } else if (operacion == 1) {
            result = unregister_user(username);
                      
        // Verificar si la operación es CONNECT
        } else if (operacion == 2) {
            char ip[16];
            int32_t port;
            
            // Recibir IP del cliente
            if (recv(client_socket, ip, sizeof(ip), 0) == -1) {
                perror("s> Error al recibir la IP del cliente");
                result = 2;
            }
            printf("s> IP: %s\n", ip);
            // Recibir puerto del cliente
            if (recv(client_socket, &port, sizeof(port), 0) == -1) {
                perror("s> Error al recibir el puerto del cliente");
                result = 2;
            } else {
                port = ntohl(port);
            }
            printf("s> Puerto: %d\n", port);
            // Lógica para CONNECT
            result = connect_user(username, ip, port);

        // Verificar si la operación es DISCONNECT
        } else if (operacion == 3) {
            result = disconnect_user(username);
        // Verificar si la operación es PUBLISH
        } else if (operacion == 4) {
            printf("s> Operación PUBLISH\n");
            char file_name[MAX_FILE_LENGTH];
            char description[MAX_FILE_LENGTH];

            if (recv(client_socket, file_name, sizeof(file_name), 0) == -1) {
                perror("s> Error al recibir el nombre del cliente");
                result = 4;
            }
            printf("s> Nombre del fichero recibido: %s\n", file_name);

            // Recibir descripción del fichero del cliente
            if (recv(client_socket, description, sizeof(description), 0) == -1) {
                perror("s> Error al recibir la descripción del fichero");
                result = 4;
            }
            printf("s> Descripción del fichero recibida: %s\n", description);
            
            printf("s> Descripción: %s\n", description);
            // Lógica para PUBLISH
            result = publish_file(username, file_name, description);

        // Verificar si la operación es DELETE
        } else if (operacion == 5) {
            char file_name[MAX_FILE_LENGTH];

            if (recv(client_socket, file_name, sizeof(file_name), 0) == -1) {
                perror("s> Error al recibir el nombre del fichero");
                result = 4;
            } else {
                result = delete_file(username, file_name);
            }
        // Verificar si la operación es LISTUSERS
        } else if (operacion == 6) {
            // Recibir nombre de usuario del cliente
            
            result = list_connected_users(username, client_socket);
            
        } else if (operacion == 7) {
            char remote_user[MAX_USERNAME_LENGTH];

            // Recibir nombre de usuario remoto del cliente
            if (recv(client_socket, remote_user, sizeof(remote_user), 0) == -1) {
                perror("s> Error al recibir el nombre de usuario remoto");
                result = 4;
            
            } else {
                printf("s> Nombre de usuario remoto: %s\n", remote_user);
                // Lógica para LIST_CONTENT
                result = list_user_content(username, remote_user, client_socket);
            }
        // Verificar si la operación es GETFILE
        } else if (operacion == 8) {
            char remote_user[MAX_USERNAME_LENGTH];
            char file_name[MAX_FILE_LENGTH];
            char remote_ip[16];
            int remote_port;

            if (recv(client_socket, remote_user, sizeof(remote_user), 0) == -1) {
                perror("s> Error al recibir el nombre de usuario remoto");
                result = 2;
            } else if (recv(client_socket, file_name, sizeof(file_name), 0) == -1) {
                perror("s> Error al recibir el nombre del fichero");
                result = 2;
            } else {
                result = get_file_info(remote_user, file_name, remote_ip, &remote_port);
                if (result == 0) {
                    char success_code = 0;
                    printf("s> IP remota: %s\n", remote_ip);
                    printf("s> Puerto remoto: %d\n", remote_port);
                    remote_ip[15] = '\0';
                    char remote_port_str[6];
                    sprintf(remote_port_str, "%d", remote_port);
                    char delimiter = ':';
                    send(client_socket, &success_code, sizeof(success_code), 0);
                    send(client_socket, remote_ip, strlen(remote_ip) + 1, 0);
                    send(client_socket, &delimiter, sizeof(delimiter), 0);
                    send(client_socket, remote_port_str, strlen(remote_port_str) + 1, 0);


                } else {
                    char error_code = result;
                    send(client_socket, &error_code, sizeof(error_code), 0);
                }
            }
        // Verificar si la operación no se conoce
        } else {
            printf("Operación desconocida: %d\n", operacion);
            result = 3;
        }      
    }
    printf("s>\n");
    // Enviar resultado al cliente
    if (send(client_socket, &result, sizeof(result), 0) == -1) {
        perror("Error al enviar el resultado al cliente");
    }

    // Cerrar conexión con el cliente
    close(client_socket);
    free(args);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc!= 3 || strcmp(argv[1], "-p")!= 0) {
        printf("Uso: %s -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        printf("Puerto inválido. Debe estar entre 1 y 65535.\n");
        exit(EXIT_FAILURE);
    }

    printf("s> init server %s:%d\ns>\n", "localhost", port);

    // Configurar servidor
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Crear el archivo "conexiones.txt" si no existe
    FILE *file = fopen(CONNECTIONS_FILE, "w");
    if (file == NULL) {
        perror("Error al crear el archivo de conexiones");
        exit(EXIT_FAILURE);
    }
    fclose(file);

    // Crear el archivo "publicaciones.txt" si no existe
    FILE *publi = fopen("publicaciones.txt", "w");
    if (publi == NULL) {
        perror("Error al crear el archivo de conexiones");
        exit(EXIT_FAILURE);
    }
    fclose(publi);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);  // Puerto del servidor
    memset(&(server_addr.sin_zero), '\0', 8);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error en el enlace del socket del servidor");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Error al poner el socket del servidor en modo de escucha");
        exit(EXIT_FAILURE);
    }

    // Esperar conexiones de clientes y manejarlas
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1) {
            perror("Error al aceptar la conexión del cliente");
            continue;
        }

        // Manejar las operaciones en hilos separados
        pthread_t thread;
        struct client_thread_args *args = malloc(sizeof(struct client_thread_args));
        args->socket = client_socket;

        if (pthread_create(&thread, NULL, handle_client, args) != 0) {
            perror("Error al crear el hilo del cliente");
            close(client_socket);
            free(args);
        }
    }

    // Cerrar el socket del servidor
    close(server_socket);

    return 0;
}
