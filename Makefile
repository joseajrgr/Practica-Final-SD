# Nombre del ejecutable
TARGET = bin/server

# Compilador
CC = gcc

# Opciones del compilador
CFLAGS = -Wall -Wextra -pthread -g -I/usr/include/tirpc
LDFLAGS = -lnsl -ltirpc

# Archivos fuente
SRCS = src/server.c src/almacenamiento.c src/sockets.c

# Objetos generados
OBJS = $(SRCS:src/%.c=obj/%.o)

# Regla por defecto para generar el ejecutable
all: directories $(TARGET)

# Regla para compilar el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) rpc/impresion_clnt.o rpc/impresion_xdr.o

# Regla para compilar los archivos fuente en objetos
obj/%.o: src/%.c include/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Regla para compilar el server
obj/server.o: src/server.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Para crear las carpetas necesarias
directories:
	mkdir -p bin obj

# Limpiar archivos generados
clean:
	rm -f $(TARGET) $(OBJS)
