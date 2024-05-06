from enum import Enum
import argparse
import socket
import aux
import time

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
    _user = None

    # ******************** METHODS *******************


    @staticmethod
    def register(user):
        REGISTER = 0

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((client._server, client._port))
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
        UNREGISTER = 1

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((client._server, client._port))
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
        if client._user is not None:
            print("c> USER ALREADY CONNECTED")
            return client.RC.USER_ERROR
        CONNECT = 2
        print("c> Connecting to server...")
        listen_port = aux.find_free_port()
        print("c> Port: " + str(listen_port))
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((client._server, client._port))
            s.sendall(CONNECT.to_bytes(4, byteorder='big'))
            user_data = user.encode('utf-8') + b'\0'
            s.sendall(user_data)

            # Enviar IP y puerto del cliente al servidor
            print("c> Port: " + str(listen_port))
            ip = socket.gethostbyname(socket.gethostname())
            ip_data = ip.encode('utf-8') + b'\0'
            s.sendall(ip_data)
            print("c> Port: " + str(listen_port))
            port_data = listen_port.to_bytes(4, byteorder='big')
            s.sendall(port_data)

            response = s.recv(1)
            if response == b'\x00':
                print("c> CONNECT OK")
                client._user = user
                # Crear el socket de escucha del cliente y el hilo para atender las peticiones de descarga
                #aux.start_listen_thread(listen_port)
            elif response == b'\x01':
                print("c> CONNECT FAIL, USER DOES NOT EXIST")
            elif response == b'\x02':
                print("c> USER ALREADY CONNECTED")
                client._user = user
            else:
                print("c> CONNECT FAIL")
                
        return client.RC.ERROR
    
    @staticmethod
    def disconnect(user):
        DISCONNECT = 3

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((client._server, client._port))
            s.sendall(DISCONNECT.to_bytes(4, byteorder='big'))
            user_data = user.encode('utf-8') + b'\0'
            s.sendall(user_data)

            response = s.recv(1)
            if response == b'\x00':
                print("c> DISCONNECT OK")
                client._user = None
                # Detener la ejecución del hilo y cerrar el puerto de escucha
                # ...
            elif response == b'\x01':
                print("c> DISCONNECT FAIL / USER DOES NOT EXIST")
            elif response == b'\x02':
                print("c> DISCONNECT FAIL / USER NOT CONNECTED")
            else:
                print("c> DISCONNECT FAIL")

        return client.RC.ERROR

    @staticmethod
    def publish(file_name, description):
        PUBLISH = 4
        
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((client._server, client._port))
            s.sendall(PUBLISH.to_bytes(4, byteorder='big'))

            print("Enviando nombre de usuario: ", client._user)
            user_data = client._user.encode('utf-8') + b'\0'
            s.sendall(user_data)
            time.sleep(1)
            print("Enviando nombre del fichero: ", file_name)
            file_name_data = file_name.encode('utf-8') + b'\0'
            print("Datos del nombre del fichero:", file_name_data)  # Imprimir los datos enviados
            s.sendall(file_name_data)
            time.sleep(1)
            print("Enviando descripción del fichero: ", description)
            description_data = description.encode('utf-8') + b'\0'
            print("Datos de la descripción del fichero:", description_data)
            s.sendall(description_data)

            response = s.recv(1)
            if response == b'\x00':
                print("c> PUBLISH OK")
            elif response == b'\x01':
                print("c> PUBLISH FAIL, USER DOES NOT EXIST")
            elif response == b'\x02':
                print("c> PUBLISH FAIL, USER NOT CONNECTED")
            elif response == b'\x03':
                print("c> PUBLISH FAIL, CONTENT ALREADY PUBLISHED")
            else:
                print("c> PUBLISH FAIL")

        return client.RC.ERROR

    @staticmethod
    def  delete(fileName) :
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def listusers():
        LISTUSERS = 6

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((client._server, client._port))
            s.sendall(LISTUSERS.to_bytes(4, byteorder='big'))

            print(f"Enviando nombre de usuario: {client._user}")
            user_data = client._user.encode('utf-8') + b'\0'
            s.sendall(user_data)
            
            response = s.recv(1)
            if response == b'\x00':
                print("c> LIST_USERS OK")
                num_users = s.recv(1024).decode('utf-8')
                print(num_users)
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
                            if client._user is not None:
                                client.disconnect(client._user)
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
