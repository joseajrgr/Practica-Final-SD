import socket
import sys
import threading
import os


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
    thread = threading.Thread(target=handle_download_requests, args=(server_socket,), daemon=True)
    thread.start()

def handle_download_requests(server_socket):
    while True:
        MAX_FILE_LENGTH = 256
        client_socket, address = server_socket.accept()
        file_name = client_socket.recv(MAX_FILE_LENGTH).decode('utf-8').rstrip('\0')
        
        file_path = os.path.join("./", file_name)
        if os.path.exists(file_path):
            with open(file_path, 'rb') as f:
                while True:
                    data = f.read(1024)
                    if not data:
                        break
                    client_socket.sendall(data)
        else:
            print("File not found:", file_name)

        
        client_socket.close()
        


def stop_listen_thread(thread):
    # Detener la ejecución del hilo
    thread.join()

    # Cerrar el socket de escucha
    server_socket = thread._args[0]
    server_socket.close()

    print("stop_listen_thread()")