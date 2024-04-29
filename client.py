from enum import Enum
import argparse
import socket
import argparse

class client :

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum) :
        OK = 0
        ERROR = 1
        USER_ERROR = 2

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1

    # ******************** METHODS *******************


    @staticmethod
    def register(user):
        server_address = client._server
        server_port = client._port
        REGISTER = 0

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((server_address, server_port))
            s.sendall(REGISTER.to_bytes(4, byteorder='big'))
            user_data = user.encode('utf-8') + b'\0'
            s.sendall(user_data)

            response = s.recv(1)
            if response == b'\x00':
                print("c> REGISTER OK")
            elif response == b'\x01':
                print("c> USERNAME IN USE")
            else:
                print("c> REGISTER FAIL")

        return client.RC.ERROR

   
    @staticmethod
    def unregister(user):
        server_address = client._server
        server_port = client._port
        UNREGISTER = 1

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((server_address, server_port))
            s.sendall(UNREGISTER.to_bytes(4, byteorder='big'))
            user_data = user.encode('utf-8') + b'\0'
            s.sendall(user_data)

            response = s.recv(1)
            if response == b'\x00':
                print("c> UNREGISTER OK")
            elif response == b'\x01':
                print("c> USER DOES NOT EXIST")
            else:
                print("c> UNREGISTER FAIL")

        return client.RC.ERROR


    
    @staticmethod
    def connect(user, listen_port=5555):
        server_address = client._server
        server_port = client._port
        CONNECT = 2
        listen_port = client.find_free_port()

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((server_address, server_port))
            s.sendall(CONNECT.to_bytes(4, byteorder='big'))
            user_data = user.encode('utf-8') + b'\0'
            s.sendall(user_data)

            # Enviar IP y puerto del cliente al servidor
            ip = socket.gethostbyname(socket.gethostname())
            ip_data = ip.encode('utf-8') + b'\0'
            s.sendall(ip_data)

            port_data = str(listen_port).encode('utf-8') + b'\0'
            s.sendall(port_data)

            response = s.recv(1)
            if response == b'\x00':
                print("c> CONNECT OK")
                 # Crear el socket de escucha del cliente y el hilo para atender las peticiones de descarga
                client.start_listen_thread(listen_port)
            elif response == b'\x01':
                print("c> CONNECT FAIL, USER DOES NOT EXIST")
            elif response == b'\x02':
                print("c> USER ALREADY CONNECTED")
            else:
                print("c> CONNECT FAIL")

        return client.RC.ERROR

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

    
    @staticmethod
    def  disconnect(user) :
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def  publish(fileName,  description) :
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def  delete(fileName) :
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def  listusers() :
        server_address = client._server
        server_port = client._port
        LISTUSERS = 6

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((server_address, server_port))
            s.sendall(LISTUSERS.to_bytes(4, byteorder='big'))

            response = s.recv(1)
            if response == b'\x00':
                print("c> LIST_USERS OK")
            elif response == b'\x01':
                print("c> LIST_USERS FAIL, USER DOES NOT EXIST")
            elif response == b'\x02':
                print("c> LIST_USERS FAIL, USER NOT CONNECTED")
            else:
                print("c> LIST_USERS FAIL")

        return client.RC.ERROR

    @staticmethod
    def  listcontent(user) :
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def  getfile(user,  remote_FileName,  local_FileName) :
        #  Write your code here
        return client.RC.ERROR

    # *
    # **
    # * @brief Command interpreter for the client. It calls the protocol functions.
    @staticmethod
    def shell():

        while (True) :
            try :
                command = input("c> ")
                line = command.split(" ")
                if (len(line) > 0):

                    line[0] = line[0].upper()

                    if (line[0]=="REGISTER") :
                        if (len(line) == 2) :
                            client.register(line[1])
                        else :
                            print("Syntax error. Usage: REGISTER <userName>")

                    elif(line[0]=="UNREGISTER") :
                        if (len(line) == 2) :
                            client.unregister(line[1])
                        else :
                            print("Syntax error. Usage: UNREGISTER <userName>")

                    elif(line[0]=="CONNECT") :
                        if (len(line) == 2) :
                            client.connect(line[1])
                        else :
                            print("Syntax error. Usage: CONNECT <userName>")
                    
                    elif(line[0]=="PUBLISH") :
                        if (len(line) >= 3) :
                            #  Remove first two words
                            description = ' '.join(line[2:])
                            client.publish(line[1], description)
                        else :
                            print("Syntax error. Usage: PUBLISH <fileName> <description>")

                    elif(line[0]=="DELETE") :
                        if (len(line) == 2) :
                            client.delete(line[1])
                        else :
                            print("Syntax error. Usage: DELETE <fileName>")

                    elif(line[0]=="LIST_USERS") :
                        if (len(line) == 1) :
                            client.listusers()
                        else :
                            print("Syntax error. Use: LIST_USERS")

                    elif(line[0]=="LIST_CONTENT") :
                        if (len(line) == 2) :
                            client.listcontent(line[1])
                        else :
                            print("Syntax error. Usage: LIST_CONTENT <userName>")

                    elif(line[0]=="DISCONNECT") :
                        if (len(line) == 2) :
                            client.disconnect(line[1])
                        else :
                            print("Syntax error. Usage: DISCONNECT <userName>")

                    elif(line[0]=="GET_FILE") :
                        if (len(line) == 4) :
                            client.getfile(line[1], line[2], line[3])
                        else :
                            print("Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>")

                    elif(line[0]=="QUIT") :
                        if (len(line) == 1) :
                            break
                        else :
                            print("Syntax error. Use: QUIT")
                    else :
                        print("Error: command " + line[0] + " not valid.")
            except Exception as e:
                print("Exception: " + str(e))

    # *
    # * @brief Prints program usage
    @staticmethod
    def usage() :
        print("Usage: python3 client.py -s <server> -p <port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def  parseArguments(argv) :
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', type=str, required=True, help='Server IP')
        parser.add_argument('-p', type=int, required=True, help='Server Port')
        args = parser.parse_args()

        if (args.s is None):
            parser.error("Usage: python3 client.py -s <server> -p <port>")
            return False

        if ((args.p < 1024) or (args.p > 65535)):
            parser.error("Error: Port must be in the range 1024 <= port <= 65535");
            return False;
        
        client._server = args.s
        client._port = args.p

        return True


    # ******************** MAIN *********************
    @staticmethod
    def main(argv) :
        if (not client.parseArguments(argv)) :
            client.usage()
            return

        #  Write code here
        client.shell()
        print("+++ FINISHED +++")
    

if __name__=="__main__":
    client.main([])
