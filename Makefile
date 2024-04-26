# Nombre del ejecutable
TARGET = servidor

# Compilador
CC = gcc

# Opciones del compilador
CFLAGS = -Wall -Wextra -pthread

# Archivos fuente
SRCS = servidor.c almacenamiento.c

# Objetos generados
OBJS = $(SRCS:.c=.o)

# Regla por defecto para generar el ejecutable
all: $(TARGET)

# Regla para compilar el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Regla para compilar los archivos fuente en objetos
%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Para crear las carpetas necesarias
directories:
	mkdir -p bin obj

# Limpiar archivos generados
clean:
	rm -f $(TARGET) $(OBJS)
