#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "almacenamiento.h"

#define MAX_USERNAME_LENGTH 256
#define USERS_FILE "usuarios.txt"
#define CONNECTIONS_FILE "conexiones.txt"

int result = 0;

int registrar_usuario(username) {
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

    return result;
}