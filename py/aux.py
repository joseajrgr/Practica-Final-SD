import socket
import sys

def readNumber(sock):
    a = ''
    while True:
        msg = sock.recv(1)
        if (msg == b'\0'):
            break;
        a += msg.decode()

    return(int(a,10))

def find_free_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('', 0))
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        return s.getsockname()[1]

def start_listen_thread(port):
    # Crear el socket de escucha del cliente
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('', port))
    server_socket.listen(1)

    # Crear el hilo para atender las peticiones de descarga
    #thread = threading.Thread(target=handle_download_requests, args=(server_socket,))
    #thread.start()

def handle_download_requests(server_socket):
    while True:
        client_socket, address = server_socket.accept()
        # Lógica para atender las peticiones de descarga de ficheros
        # ...