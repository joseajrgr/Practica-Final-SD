#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include "../include/almacenamiento.h"

#define MAX_USERNAME_LENGTH 256
#define MAX_FILE_LENGTH 256
#define USERS_FILE "usuarios.txt"
#define CONNECTIONS_FILE "conexiones.txt"
#define USERS_DIRECTORY "servidor"

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

    // Construir la ruta de la carpeta del usuario
    char user_directory[MAX_USERNAME_LENGTH + sizeof(USERS_DIRECTORY) + 2];
    snprintf(user_directory, sizeof(user_directory), "%s/%s", USERS_DIRECTORY, username);

    // Verificar si la carpeta del usuario existe
    struct stat st = {0};
    if (stat(user_directory, &st) == -1) {
        printf("Usuario no encontrado: %s\n", username);
        result = 1;  // Usuario no encontrado
    } else {
        // La carpeta existe, eliminarla
        if (remove(user_directory) == -1) {
            perror("Error al eliminar la carpeta del usuario");
            result = 2;
        } else {
            printf("Usuario eliminado: %s\n", username);
            result = 0;  // Baja exitosa
        }
    }

    return result;
}

int connect_user(char username[MAX_USERNAME_LENGTH], char ip[16], int port) {
    int result = 0;

    // Construir la ruta de la carpeta del usuario
    char user_directory[MAX_USERNAME_LENGTH + sizeof(USERS_DIRECTORY) + 2];
    snprintf(user_directory, sizeof(user_directory), "%s/%s", USERS_DIRECTORY, username);
    printf("Directorio del usuario: %s\n", user_directory);
    // Verificar si la carpeta del usuario existe
    struct stat st = {0};
    if (stat(user_directory, &st) == 0) {
        // La carpeta del usuario existe
        FILE *connections_file = fopen(CONNECTIONS_FILE, "r");
        if (connections_file == NULL) {
            perror("Error al abrir el archivo de conexiones");
            result = 2;
        } else {
            char line[MAX_USERNAME_LENGTH + 20];
            int user_connected = 0;
            while (fgets(line, sizeof(line), connections_file)) {
                line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                if (strncmp(line, username, strlen(username)) == 0) {
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
        printf("Usuario no encontrado: %s\n", username);
        result = 1;  // Usuario no encontrado
    }
    return result;
}

int disconnect_user(char username[MAX_USERNAME_LENGTH]) {
    int result = 0;

    // Verificar si el usuario existe
    char user_directory[MAX_USERNAME_LENGTH + sizeof(USERS_DIRECTORY) + 2];
    snprintf(user_directory, sizeof(user_directory), "%s/%s", USERS_DIRECTORY, username);
    struct stat st = {0};
    if (stat(user_directory, &st) == -1) {
        result = 1;  // Usuario no existe
    } else {
        // Verificar si el usuario está conectado
        FILE *connections_file = fopen(CONNECTIONS_FILE, "r");
        if (connections_file == NULL) {
            perror("Error al abrir el archivo de conexiones");
            result = 3;
        } else {
            char line[MAX_USERNAME_LENGTH + 20];
            int user_connected = 0;
            while (fgets(line, sizeof(line), connections_file)) {
                line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                if (strncmp(line, username, strlen(username)) == 0) {
                    user_connected = 1;
                    break;
                }
            }
            fclose(connections_file);

            if (user_connected) {
                // Eliminar al usuario del archivo de conexiones
                remove_user_from_connections_file(username);
                result = 0;  // Éxito
            } else {
                result = 2;  // Usuario no conectado
            }
        }
    }

    return result;
}

void remove_user_from_connections_file(char username[MAX_USERNAME_LENGTH]) {
    FILE *connections_file = fopen(CONNECTIONS_FILE, "r");
    FILE *temp_file = fopen("temp.txt", "w");
    if (connections_file == NULL || temp_file == NULL) {
        perror("Error al abrir los archivos");
        return;
    }

    char line[MAX_USERNAME_LENGTH + 20];
    while (fgets(line, sizeof(line), connections_file)) {
        if (strncmp(line, username, strlen(username)) != 0) {
            fputs(line, temp_file);
        }
    }

    fclose(connections_file);
    fclose(temp_file);

    remove(CONNECTIONS_FILE);
    rename("temp.txt", CONNECTIONS_FILE);
}

int publish_file(char username[MAX_USERNAME_LENGTH], char file_name[MAX_FILE_LENGTH], char description[MAX_FILE_LENGTH]) {
    int result = 0;

    // Verificar si el usuario existe
    char user_directory[MAX_USERNAME_LENGTH + sizeof(USERS_DIRECTORY) + 2];
    snprintf(user_directory, sizeof(user_directory), "%s/%s", USERS_DIRECTORY, username);
    struct stat st = {0};
    if (stat(user_directory, &st) == -1) {
        result = 1;  // Usuario no existe
    } else {
        // Verificar si el usuario está conectado
        FILE *connections_file = fopen(CONNECTIONS_FILE, "r");
        if (connections_file == NULL) {
            perror("Error al abrir el archivo de conexiones");
            result = 4;
        } else {
            char line[MAX_USERNAME_LENGTH + 20];
            int user_connected = 0;
            while (fgets(line, sizeof(line), connections_file)) {
                line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                if (strncmp(line, username, strlen(username)) == 0) {
                    user_connected = 1;
                    break;
                }
            }
            fclose(connections_file);

            if (user_connected) {
                // Verificar si el fichero existe en la carpeta del usuario
                char file_path[MAX_USERNAME_LENGTH + MAX_FILE_LENGTH + sizeof(USERS_DIRECTORY) + 4];
                snprintf(file_path, sizeof(file_path), "%s", file_name);
                 FILE *file = fopen("publicaciones.txt", "r");
                if (file == NULL) {
                    perror("Error al abrir el archivo de publicaciones");
                    result = 4;
                } else {
                    char line[MAX_USERNAME_LENGTH + MAX_FILE_LENGTH + MAX_FILE_LENGTH + 4];
                    int file_already_published = 0;
                    while (fgets(line, sizeof(line), file)) {
                        char *token = strtok(line, " ");
                        if (token != NULL && strcmp(token, username) == 0) {
                            token = strtok(NULL, " ");
                            if (token != NULL && strcmp(token, file_path) == 0) {
                                file_already_published = 1;
                                break;
                            }
                        }
                    }
                    fclose(file);

                    if (file_already_published) {
                        result = 3;  // Fichero ya publicado
                    } else {
                        // Guardar la información en el archivo "publicaciones.txt"
                        file = fopen("publicaciones.txt", "a");
                        if (file == NULL) {
                            perror("Error al abrir el archivo de publicaciones");
                            result = 4;
                        } else {
                            fprintf(file, "%s %s %s\n", username, file_path, description);
                            fclose(file);
                            result = 0;  // Éxito
                        }
                    }
                }
            } else {
                result = 2;  // Usuario no conectado
            }
        }
    }

    return result;
}
int list_connected_users(char username[MAX_USERNAME_LENGTH], int client_socket) {
    int result = 0;

    // Verificar si el usuario existe
    char user_directory[MAX_USERNAME_LENGTH + sizeof(USERS_DIRECTORY) + 2];
    snprintf(user_directory, sizeof(user_directory), "%s/%s", USERS_DIRECTORY, username);
    struct stat st = {0};
    if (stat(user_directory, &st) == -1) {
        result = 1;  // Usuario no existe
    } else {
        // Verificar si el usuario está conectado
        FILE *connections_file = fopen(CONNECTIONS_FILE, "r");
        if (connections_file == NULL) {
            perror("Error al abrir el archivo de conexiones");
            result = 3;
        } else {
            char line[MAX_USERNAME_LENGTH + 20];
            int user_connected = 0;
            while (fgets(line, sizeof(line), connections_file)) {
                line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                if (strncmp(line, username, strlen(username)) == 0) {
                    user_connected = 1;
                    break;
                }
            }
            fclose(connections_file);

            if (user_connected) {
                // Enviar código de éxito al cliente
                char success_code = 0;
                send(client_socket, &success_code, sizeof(success_code), 0);

                // Leer el contenido del archivo de conexiones y enviarlo al cliente
                connections_file = fopen(CONNECTIONS_FILE, "r");
                if (connections_file == NULL) {
                    perror("Error al abrir el archivo de conexiones");
                    result = 3;
                } else {
                    char file_contents[1024] = {0};
                    while (fgets(line, sizeof(line), connections_file)) {
                        line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                        strcat(file_contents, line);
                        strcat(file_contents, "\n");
                    }
                    fclose(connections_file);

                    // Enviar el contenido del archivo al cliente
                    send(client_socket, file_contents, strlen(file_contents), 0);

                    result = 0;  // Éxito
                }
            } else {
                result = 2;  // Usuario no conectado
            }
        }
    }

    return result;
}


int list_user_content(char username[MAX_USERNAME_LENGTH], char remote_user[MAX_USERNAME_LENGTH], int client_socket) {
    int result = 0;

    // Verificar si el usuario existe
    char user_directory[MAX_USERNAME_LENGTH + sizeof(USERS_DIRECTORY) + 2];
    snprintf(user_directory, sizeof(user_directory), "%s/%s", USERS_DIRECTORY, username);
    struct stat st = {0};
    if (stat(user_directory, &st) == -1) {
        result = 1;  // Usuario no existe
    } else {
        // Verificar si el usuario está conectado
        FILE *connections_file = fopen(CONNECTIONS_FILE, "r");
        if (connections_file == NULL) {
            perror("Error al abrir el archivo de conexiones");
            result = 4;
        } else {
            char line[MAX_USERNAME_LENGTH + 20];
            int user_connected = 0;
            while (fgets(line, sizeof(line), connections_file)) {
                line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                if (strncmp(line, username, strlen(username)) == 0) {
                    user_connected = 1;
                    break;
                }
            }
            fclose(connections_file);

            if (user_connected) {
                // Verificar si el usuario remoto existe
                char remote_user_directory[sizeof(USERS_DIRECTORY) + 3];
                snprintf(remote_user_directory, sizeof(remote_user_directory), "%s/%s", USERS_DIRECTORY, remote_user);
                if (stat(remote_user_directory, &st) == -1) {
                    result = 3;  // Usuario remoto no existe
                } else {
                    // Leer el contenido del archivo "publicaciones.txt"
                    FILE *file = fopen("publicaciones.txt", "r");
                    if (file == NULL) {
                        perror("Error al abrir el archivo de publicaciones");
                        result = 4;
                    } else {
                        char line[MAX_USERNAME_LENGTH + MAX_FILE_LENGTH + MAX_FILE_LENGTH + 4];
                        char file_contents[1024] = {0};

                        while (fgets(line, sizeof(line), file)) {
                            line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                            char *token = strtok(line, " ");
                            if (token != NULL && strcmp(token, remote_user) == 0) {
                                token = strtok(NULL, " ");
                                if (token != NULL) {
                                    char file_name[MAX_FILE_LENGTH];
                                    strcpy(file_name, token);
                                    token = strtok(NULL, "");
                                    if (token != NULL) {
                                        char description[MAX_FILE_LENGTH + 4];
                                        snprintf(description, sizeof(description), "\"%s\"", token);
                                        strcat(file_contents, file_name);
                                        strcat(file_contents, " ");
                                        strcat(file_contents, description);
                                        strcat(file_contents, "\n");
                                    }
                                }
                            }
                        }
                        fclose(file);

                        // Enviar código de éxito al cliente
                        char success_code = 0;
                        send(client_socket, &success_code, sizeof(success_code), 0);

                        // Enviar el contenido del archivo al cliente
                        send(client_socket, file_contents, strlen(file_contents), 0);

                        result = 0;  // Éxito
                    }}
            } else {
                result = 2;  // Usuario no conectado
            }
        }
    }

    return result;
}

int delete_file(char username[MAX_USERNAME_LENGTH], char file_name[MAX_FILE_LENGTH]) {
    int result = 0;

    // Verificar si el usuario existe
    char user_directory[MAX_USERNAME_LENGTH + sizeof(USERS_DIRECTORY) + 2];
    snprintf(user_directory, sizeof(user_directory), "%s/%s", USERS_DIRECTORY, username);
    struct stat st = {0};
    if (stat(user_directory, &st) == -1) {
        result = 1;  // Usuario no existe
    } else {
        // Verificar si el usuario está conectado
        FILE *connections_file = fopen(CONNECTIONS_FILE, "r");
        if (connections_file == NULL) {
            perror("Error al abrir el archivo de conexiones");
            result = 4;
        } else {
            char line[MAX_USERNAME_LENGTH + 20];
            int user_connected = 0;
            while (fgets(line, sizeof(line), connections_file)) {
                line[strcspn(line, "\n")] = '\0';  // Eliminar el salto de línea
                if (strncmp(line, username, strlen(username)) == 0) {
                    user_connected = 1;
                    break;
                }
            }
            fclose(connections_file);

            if (user_connected) {
                // Verificar si el fichero está publicado en el archivo "publicaciones.txt"
                FILE *file = fopen("publicaciones.txt", "r");
                if (file == NULL) {
                    perror("Error al abrir el archivo de publicaciones");
                    result = 4;
                } else {
                    char line[MAX_USERNAME_LENGTH + MAX_FILE_LENGTH + MAX_FILE_LENGTH + 4];
                    int file_found = 0;
                    FILE *temp_file = fopen("temp.txt", "w");
                    if (temp_file == NULL) {
                        perror("Error al crear el archivo temporal");
                        result = 4;
                    } else {
                        while (fgets(line, sizeof(line), file)) {
                            char *token = strtok(line, " ");
                            if (token != NULL && strcmp(token, username) == 0) {
                                token = strtok(NULL, " ");
                                if (token != NULL && strcmp(token, file_name) == 0) {
                                    file_found = 1;
                                    continue;  // Omitir la línea correspondiente al fichero a eliminar
                                }
                            }
                            fputs(line, temp_file);
                        }
                        fclose(file);
                        fclose(temp_file);

                        if (file_found) {
                            remove("publicaciones.txt");
                            rename("temp.txt", "publicaciones.txt");
                            result = 0;  // Éxito
                        } else {
                            remove("temp.txt");
                            result = 3;  // Fichero no publicado previamente
                        }
                    }
                }
            } else {
                result = 2;  // Usuario no conectado
            }
        }
    }

    return result;
}

int get_file_info(char username[MAX_USERNAME_LENGTH], char file_name[MAX_FILE_LENGTH], char remote_ip[16], int *remote_port) {
    int result = 0;

    // Verificar si el fichero está publicado en el archivo "publicaciones.txt"
    FILE *file = fopen("publicaciones.txt", "r");
    if (file == NULL) {
        perror("Error al abrir el archivo de publicaciones");
        result = 2;
    } else {
        char line[MAX_USERNAME_LENGTH + MAX_FILE_LENGTH + MAX_FILE_LENGTH + 4];
        int file_found = 0;
        while (fgets(line, sizeof(line), file)) {
            char *token = strtok(line, " ");
            if (token != NULL && strcmp(token, username) == 0) {
                token = strtok(NULL, " ");
                if (token != NULL) {
                    char *file_path = token;
                    char *file_name_ptr = strrchr(file_path, '/');
                    if (file_name_ptr != NULL && strcmp(file_name_ptr + 1, file_name) == 0) {
                        file_found = 1;
                        break;
                    }
                }
            }
        }
        fclose(file);

        if (file_found) {
            // Obtener la IP y el puerto del cliente que publicó el fichero
            FILE *connections_file = fopen(CONNECTIONS_FILE, "r");
            if (connections_file == NULL) {
                perror("Error al abrir el archivo de conexiones");
                result = 2;
            } else {
                char line[MAX_USERNAME_LENGTH + 20];
                while (fgets(line, sizeof(line), connections_file)) {
                    char *token = strtok(line, " ");
                    if (token != NULL && strcmp(token, username) == 0) {
                        token = strtok(NULL, " ");
                        if (token != NULL) {
                            strcpy(remote_ip, token);
                            token = strtok(NULL, " ");
                            if (token != NULL) {
                                *remote_port = atoi(token);
                                result = 0;  // Éxito
                                break;
                            }
                        }
                    }
                }
                fclose(connections_file);
            }
        } else {
            result = 1;  // Fichero no publicado
        }
    }

    return result;
}