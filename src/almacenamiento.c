#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include "../include/almacenamiento.h"

#define MAX_USERNAME_LENGTH 256
#define USERS_FILE "usuarios.txt"
#define CONNECTIONS_FILE "conexiones.txt"
#define USERS_DIRECTORY "users"

int register_user(char username[MAX_USERNAME_LENGTH]) {
    int result = 0;

    // Crear la carpeta "users" si no existe
    struct stat st = {0};
    if (stat(USERS_DIRECTORY, &st) == -1) {
        if (mkdir(USERS_DIRECTORY, 0777) == -1) {
            perror("Error al crear la carpeta 'users'");
            return 2;
        }
    }

    // Construir la ruta de la carpeta del usuario
    char user_directory[MAX_USERNAME_LENGTH + sizeof(USERS_DIRECTORY) + 2];
    snprintf(user_directory, sizeof(user_directory), "%s/%s", USERS_DIRECTORY, username);

    // Verificar si la carpeta del usuario ya existe
    if (stat(user_directory, &st) == -1) {
        // La carpeta no existe, crearla
        if (mkdir(user_directory, 0777) == -1) {
            perror("Error al crear la carpeta del usuario");
            result = 2;
        } else {
            printf("Usuario registrado y carpeta creada: %s\n", user_directory);
            result = 0;  // Registro exitoso
        }
    } else {
        printf("El usuario ya está registrado: %s\n", username);
        result = 1;  // Usuario ya registrado
    }

    return result;
}

int unregister_user(char username[MAX_USERNAME_LENGTH]) {
    int result = 0;

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
    
    return result;
}

int connect_user(char username[MAX_USERNAME_LENGTH], char ip[16], int port) {
    int result = 0;

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

    return result;
}
