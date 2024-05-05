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
#define PORT 5500

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
    
    // Recibir código de operación
    if (recv(client_socket, buffer, sizeof(int32_t), 0) == -1) {
        perror("Error al recibir el código de operación");
        result = 2;
    } else {
        operacion = ntohl(*(int*)buffer);

        // Recibir nombre de usuario del cliente
        if (recv(client_socket, username, sizeof(char[MAX_USERNAME_LENGTH]), 0) == -1) {
            perror("Error al recibir el nombre del cliente");
            result = 2;
        }

        // Verificar si la operación es REGISTER
        if (operacion == 0) {
            result = register_user(username);

        // Verificar si la operación es UNREGISTER
        } else if (operacion == 1) {
            result = unregister_user(username);
                      
        // Verificar si la operación es CONNECT
        } else if (operacion == 2) {
            char ip[16];
            int port;
            
            // Recibir IP del cliente
            if (recv(client_socket, ip, sizeof(ip), 0) == -1) {
                perror("Error al recibir la IP del cliente");
                result = 2;
            }
            
            // Recibir puerto del cliente
            if (recv(client_socket, buffer, sizeof(buffer), 0) == -1) {
                perror("Error al recibir el puerto del cliente");
                result = 2;
            } else {
                port = atoi(buffer);
            }

            // Lógica para CONNECT
            result = connect_user(username, ip, port);

        // Verificar si la operación es DISCONNECT
        } else if (operacion == 3) {

        // Verificar si la operación es PUBLISH
        } else if (operacion == 4) {

        // Verificar si la operación es DELETE
        } else if (operacion == 5) {

        // Verificar si la operación es LISTUSERS
        } else if (operacion == 6) {

        // Verificar si la operación es LISTCONTENT
        } else if (operacion == 7) {

        // Verificar si la operación es GETFILE
        } else if (operacion == 8) {

        // Verificar si la operación no se conoce
        } else {
            printf("Operación desconocida: %d\n", operacion);
            result = 3;
        }      
    }

    // Enviar resultado al cliente
    if (send(client_socket, &result, sizeof(result), 0) == -1) {
        perror("Error al enviar el resultado al cliente");
    }

    // Cerrar conexión con el cliente
    close(client_socket);
    free(args);
    pthread_exit(NULL);
}

int main() {
    // Configurar servidor
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);  // Puerto del servidor
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
