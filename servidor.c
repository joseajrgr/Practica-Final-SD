#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_USERNAME_LENGTH 256
#define USERS_FILE "usuarios.txt"
#define PORT 5500

// Estructura para pasar argumentos al hilo del cliente
struct client_thread_args {
    int socket;
};

void *handle_client(void *args) {
    struct client_thread_args *client_args = (struct client_thread_args *)args;
    int client_socket = client_args->socket;
    char operacion[MAX_USERNAME_LENGTH];
    char username[MAX_USERNAME_LENGTH];
    int result = 0;

    // Recibir nombre de usuario del cliente
    if (recv(client_socket, username, MAX_USERNAME_LENGTH, 0) == -1) {
        perror("Error al recibir el código de operación");
        result = 2;
    } else {
        // Aquí la lógica para verificar si el usuario ya está registrado
        // y actualizar el resultado en consecuencia
        // Abrir archivo de usuarios
        FILE *file = fopen(USERS_FILE, "a");
        if (file == NULL) {
            perror("Error al abrir el archivo de usuarios");
            result = 2;
        } else {
            // Comprobar si el usuario ya está registrado
            char line[MAX_USERNAME_LENGTH];
            while (fgets(line, MAX_USERNAME_LENGTH, file) != NULL) {
                if (strcmp(line, username) == 0) {
                    result = 1; // Usuario ya registrado
                    break;
                }
            }
            
            // Si el usuario no está registrado, agregarlo al archivo
            if (result == 0) {
                fputs(username, file);
                fputs("\n", file);
            }

            fclose(file);
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

        // Manejar la operación REGISTER en un hilo separado
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
