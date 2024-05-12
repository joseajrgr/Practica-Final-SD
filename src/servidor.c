#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../include/almacenamiento.h"
#include "../include/sockets.h"

#define MAX_DATETIME_LENGTH 20
#define MAX_IP_LENGTH 16
#define MAX_USERNAME_LENGTH 256
#define MAX_FILE_LENGTH 256
#define CONNECTIONS_FILE "conexiones.txt"

pthread_mutex_t mutex_almacenamiento;

// Estructura para pasar argumentos al hilo del cliente
struct client_thread_args {
    int socket;
};

void *handle_client(void *args) {
    // Recibir argumentos
    struct client_thread_args *client_args = (struct client_thread_args *)args;
    int client_socket = client_args->socket;
    char buffer[4];
    char datetime[MAX_DATETIME_LENGTH];
    char username[MAX_USERNAME_LENGTH];
    int32_t operacion = -1;
    int32_t result = 0;
    Respuesta respuesta;
    
    
    // Recibir código de operación
    if (recvMessage(client_socket, (char *)buffer, sizeof(int32_t)) == -1) {
        perror("s> Error al recibir el código de operación");
        result = 2;

    } else {
        operacion = ntohl(*(int*)buffer);
        
        // Recibir la fecha y la hora
        if (recvMessage(client_socket, (char *)datetime, MAX_DATETIME_LENGTH) == -1) {
            perror("s> Error al recibir la fecha y la hora");
            result = 2;
        }
        printf("s> Momento en el que se hizo el envío: %s\n", datetime);

        // Recibir nombre de usuario del cliente
        if (readLine(client_socket, (char *)username, MAX_USERNAME_LENGTH) == -1) {
            perror("s> Error al recibir el nombre del cliente");
            result = 2;
        }
        
        
        pthread_mutex_lock(&mutex_almacenamiento);
        
        // Verificar si la operación es REGISTER
        if (operacion == 0) {
            printf("s> REGISTER FROM %s\n", username);
            result = register_user(username);
            // Enviar resultado al cliente
            if (sendMessage(client_socket, (char*)&result, sizeof(int32_t)) == -1) {
                perror("Error al enviar el resultado al cliente");
            }

        // Verificar si la operación es UNREGISTER
        } else if (operacion == 1) {
            printf("s> UNREGISTER FROM %s\n", username);
            result = unregister_user(username);
            // Enviar resultado al cliente
            if (sendMessage(client_socket, (char*)&result, sizeof(int32_t)) == -1) {
                perror("Error al enviar el resultado al cliente");
            }
        // Verificar si la operación es CONNECT
        } else if (operacion == 2) {
            printf("s> CONNECT FROM %s\n", username);
            char ip[MAX_IP_LENGTH];
            int32_t port;
            
            // Recibir IP del cliente
            if (readLine(client_socket, ip, MAX_IP_LENGTH) == -1) {
                perror("s> Error al recibir la IP del cliente");
                result = 2;
            }
            
            // Recibir puerto del cliente
            if (recvMessage(client_socket, (char*)&port, sizeof(int32_t)) == -1) {
                perror("s> Error al recibir el puerto del cliente");
                result = 2;
            } else {
                port = ntohl(port);
            }
           
            // Lógica para CONNECT
            result = connect_user(username, ip, port);
            
        // Verificar si la operación es DISCONNECT
        } else if (operacion == 3) {
            printf("s> DISCONNECT FROM %s\n", username);
            result = disconnect_user(username);
        
            
        // Verificar si la operación es PUBLISH
        } else if (operacion == 4) {
            printf("s> PUBLISH FROM %s\n", username);
            char file_name[MAX_FILE_LENGTH];
            char description[MAX_FILE_LENGTH];

            if (readLine(client_socket, file_name, MAX_FILE_LENGTH) == -1) {
                perror("s> Error al recibir el nombre del cliente");
                result = 4;
            }
            
            // Recibir descripción del fichero del cliente
            if (readLine(client_socket, description, MAX_FILE_LENGTH) == -1) {
                perror("s> Error al recibir la descripción del fichero");
                result = 4;
            }
            
            // Lógica para PUBLISH
            result = publish_file(username, file_name, description);
            
        // Verificar si la operación es DELETE
        } else if (operacion == 5) {
            printf("s> DELETE FROM %s\n", username);
            char file_name[MAX_FILE_LENGTH];

            if (readLine(client_socket, file_name, sizeof(file_name)) == -1) {
                perror("s> Error al recibir el nombre del fichero");
                result = 4;
            } else {
                result = delete_file(username, file_name);
            }
            
        // Verificar si la operación es LIST_USERS
        } else if (operacion == 6) {
            printf("s> LIST_USERS FROM %s\n", username);
            // Recibir nombre de usuario del cliente  
            respuesta = list_connected_users(username);
            result = respuesta.result;
            
        // Verificar si la operación es LIST_CONTENT
        } else if (operacion == 7) {
            printf("s> LIST_CONTENT FROM %s\n", username);
            char remote_user[MAX_USERNAME_LENGTH];

            // Recibir nombre de usuario remoto del cliente
            if (readLine(client_socket, remote_user, sizeof(remote_user)) == -1) {
                perror("s> Error al recibir el nombre de usuario remoto");
                result = 4;
            
            } else {
                printf("s> Nombre de usuario remoto: %s\n", remote_user);
                // Lógica para LIST_CONTENT
                
                respuesta = list_user_content(username, remote_user);
                result = respuesta.result;
                printf("texto: %s\n", respuesta.texto);
                printf("s> Resultado de la operación LIST_CONTENT: %d\n", result);
            }
        } else {
            printf("Operación desconocida: %d\n", operacion);
            result = 3;
        }      
    }
    pthread_mutex_unlock(&mutex_almacenamiento);

    // Enviar resultado al cliente
    if (sendMessage(client_socket, (char*)&result, sizeof(int32_t)) == -1) {
        perror("Error al enviar el resultado al cliente");
    }

    // En caso de LIST_USERS o LIST_CONTENT, enviar la respuesta al cliente
    if (result == 0) {
        if (operacion == 6 || operacion == 7) {
            strcat(respuesta.texto, "\n");
            printf("s> Enviando respuesta al cliente\n");
            sendMessage(client_socket, respuesta.texto, strlen(respuesta.texto)+1);
        }
    }

    // Cerrar conexión con el cliente
    close(client_socket);
    free(args);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Verificar argumentos
    if (argc!= 3 || strcmp(argv[1], "-p")!= 0) {
        printf("Uso: %s -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Comprobar que el puerto sea válido
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        printf("Puerto inválido. Debe estar entre 1 y 65535.\n");
        exit(EXIT_FAILURE);
    }

    printf("s> init server %s:%d\n", "localhost", port);

    // Configurar socket servidor
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

    // Crear socket del servidor
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    // Enlazar el socket del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    memset(&(server_addr.sin_zero), '\0', 8);

    
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error en el enlace del socket del servidor");
        exit(EXIT_FAILURE);
    }

    // Modo de escucha del servidor
    if (listen(server_socket, 5) == -1) {
        perror("Error al poner el socket del servidor en modo de escucha");
        exit(EXIT_FAILURE);
    }

    // Inicializar el mutex de almacenamiento
    pthread_mutex_init(&mutex_almacenamiento, NULL);

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
