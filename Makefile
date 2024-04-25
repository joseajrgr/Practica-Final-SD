# Nombre del ejecutable
TARGET = servidor

# Compilador
CC = gcc

# Opciones del compilador
CFLAGS = -Wall -Wextra -pthread

# Archivos fuente
SRCS = servidor.c

# Objetos generados
OBJS = $(SRCS:.c=.o)

# Regla por defecto para generar el ejecutable
all: $(TARGET)

# Regla para compilar el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Regla para compilar los archivos fuente en objetos
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Limpiar archivos generados
clean:
	rm -f $(TARGET) $(OBJS)
