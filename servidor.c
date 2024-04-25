#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_USERNAME_LENGTH 256
#define USERS_FILE "usuarios.txt"
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
    
    // Recibir nombre de usuario del cliente
    if (recv(client_socket, buffer, sizeof(int32_t), 0) == -1) {
        perror("Error al recibir el código de operación");
        result = 2;
    } else {
        operacion = ntohl(*(int*)buffer);

        // Recibir nombre de usuario del cliente
        if (recv(client_socket, username, sizeof(char[MAX_USERNAME_LENGTH]), 0) == -1) {
            perror("Error al recibir el código de operación");
            result = 2;
        }

         // Verificar si la operación es REGISTER
    if (operacion == 0) {
        // Abrir archivo de usuarios
        FILE *file = fopen(USERS_FILE, "r");
        if (file == NULL) {
            perror("Error al abrir el archivo de usuarios");
            result = 2;
        } else {
            char line[MAX_USERNAME_LENGTH];
            int user_exists = 0;

            // Verificar si el usuario ya está registrado
            while (fgets(line, sizeof(line), file)) {
                line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                if (strcmp(line, username) == 0) {
                    user_exists = 1;
                    break;
                }
            }
            fclose(file);

            if (user_exists) {
                result = 1;  // Usuario ya registrado
            } else {
                // Abrir archivo de usuarios en modo append
                file = fopen(USERS_FILE, "a");
                if (file == NULL) {
                    perror("Error al abrir el archivo de usuarios");
                    result = 2;
                } else {
                    // Almacenar el nombre de usuario en el archivo
                    fputs(username, file);
                    fputs("\n", file);
                    fclose(file);
                    printf("Usuario almacenado: %s\n", username);
                }
            }
        }
    } else if (operacion == 1) {
            // Lógica para UNREGISTER
            FILE *file = fopen(USERS_FILE, "r");
            if (file == NULL) {
                perror("Error al abrir el archivo de usuarios");
                result = 2;
            } else {
                char line[MAX_USERNAME_LENGTH];
                int user_exists = 0;

                // Verificar si el usuario existe
                while (fgets(line, sizeof(line), file)) {
                    line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                    if (strcmp(line, username) == 0) {
                        user_exists = 1;
                        break;
                    }
                }
                fclose(file);

                if (user_exists) {
                    // Eliminar el usuario del archivo
                    FILE *temp_file = fopen("temp.txt", "w");
                    if (temp_file == NULL) {
                        perror("Error al crear el archivo temporal");
                        result = 2;
                    } else {
                        file = fopen(USERS_FILE, "r");
                        if (file == NULL) {
                            perror("Error al abrir el archivo de usuarios");
                            result = 2;
                        } else {
                            while (fgets(line, sizeof(line), file)) {
                                line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                                if (strcmp(line, username) != 0) {
                                    fputs(line, temp_file);
                                    fputs("\n", temp_file);
                                }
                            }
                            fclose(file);
                            fclose(temp_file);
                            remove(USERS_FILE);
                            rename("temp.txt", USERS_FILE);
                            printf("Usuario eliminado: %s\n", username);
                        }
                    }
                } else {
                    result = 1;  // Usuario no encontrado
                }
            }
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
            FILE *file = fopen(USERS_FILE, "r");
            if (file == NULL) {
                perror("Error al abrir el archivo de usuarios");
                result = 2;
            } else {
                char line[MAX_USERNAME_LENGTH];
                int user_exists = 0;

                // Verificar si el usuario existe
                while (fgets(line, sizeof(line), file)) {
                    line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                    if (strcmp(line, username) == 0) {
                        user_exists = 1;
                        break;
                    }
                }
                fclose(file);

                if (user_exists) {
                    // Verificar si el usuario ya está conectado
                    FILE *connections_file = fopen(CONNECTIONS_FILE, "r");
                    if (connections_file == NULL) {
                        perror("Error al abrir el archivo de conexiones");
                        result = 2;
                    } else {
                        int user_connected = 0;
                        while (fgets(line, sizeof(line), connections_file)) {
                            line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                            if (strcmp(line, username) == 0) {
                                user_connected = 1;
                                break;
                            }
                        }
                        fclose(connections_file);

                        if (!user_connected) {
                            // Agregar el usuario al archivo de conexiones
                            connections_file = fopen(CONNECTIONS_FILE, "a");
                            if (connections_file == NULL) {
                                perror("Error al abrir el archivo de conexiones");
                                result = 2;
                            } else {
                                fprintf(connections_file, "%s %s %d\n", username, ip, port);
                                fclose(connections_file);
                                result = 0;  // Éxito
                            }
                        } else {
                            result = 2;  // Usuario ya conectado
                        }
                    }
                } else {
                    result = 1;  // Usuario no existe
                }
            }
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
