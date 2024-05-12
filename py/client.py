from enum import Enum
import argparse
import socket
import aux

import os
import zeep

WSDL = "http://localhost:7901/?wsdl"

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
    _listen_thread = None
    _users_info = {}
    # ******************** METHODS *******************


    @staticmethod
    def register(user):
        REGISTER = 0

        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(REGISTER.to_bytes(4, byteorder='big'))

                client_ws = zeep.Client(wsdl=WSDL)
                datetime = client_ws.service.get_datetime()
                print("c> Momento del envío al servidor:", datetime)
                datetime = datetime.encode('utf-8') + b'\0'
                s.sendall(datetime)

                user_data = user.encode('utf-8') + b'\0'
                s.sendall(user_data)

                response = s.recv(1)
                if response == b'\x00':
                    print("c> REGISTER OK")
                    return client.RC.OK
                elif response == b'\x01':
                    print("c> USERNAME IN USE")
                    return client.RC.USER_ERROR
                else:
                    print("c> REGISTER FAIL")
                    return client.RC.ERROR
        except Exception as e:
            print("c> REGISTER FAIL")
            return client.RC.ERROR

   
    @staticmethod
    def unregister(user):
        UNREGISTER = 1


        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(UNREGISTER.to_bytes(4, byteorder='big'))

                client_ws = zeep.Client(wsdl=WSDL)
                datetime = client_ws.service.get_datetime()
                print("c> Momento del envío al servidor:", datetime)
                datetime = datetime.encode('utf-8') + b'\0'
                s.sendall(datetime)

                user_data = user.encode('utf-8') + b'\0'
                s.sendall(user_data)

                response = s.recv(1)
                if response == b'\x00':
                    print("c> UNREGISTER OK")
                    return client.RC.OK
                elif response == b'\x01':
                    print("c> USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                else:
                    print("c> UNREGISTER FAIL")
                    return client.RC.ERROR
        except Exception as e:
            print("c> UNREGISTER FAIL")
            return client.RC.ERROR


    
    @staticmethod
    def connect(user, listen_port=None):
        CONNECT = 2
        
        if client._user is not None:
            print("c> CONNECT FAIL, USER ALREADY CONNECTED")
            return client.RC.ERROR
        
        try:
            # Obtener un puerto libre si no se proporciona uno
            if listen_port is None:
                listen_port = aux.find_free_port()
                    
            # Crear el socket de escucha del cliente y el hilo para atender las peticiones de descarga
            listen_thread = aux.start_listen_thread(listen_port)
            
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(CONNECT.to_bytes(4, byteorder='big'))

                client_ws = zeep.Client(wsdl=WSDL)
                datetime = client_ws.service.get_datetime()
                print("c> Momento del envío al servidor:", datetime)
                datetime = datetime.encode('utf-8') + b'\0'
                s.sendall(datetime)

                user_data = user.encode('utf-8') + b'\0'
                s.sendall(user_data)

                # Enviar IP y puerto del cliente al servidor
                ip = socket.gethostbyname(socket.gethostname())
                ip_data = ip.encode('utf-8') + b'\0'
                s.sendall(ip_data)
                port_data = listen_port.to_bytes(4, byteorder='big')
                s.sendall(port_data)

                response = s.recv(1)
                if response == b'\x00':
                    print("c> CONNECT OK")
                    client._user = user
                    client._listen_thread = listen_thread  # Guardar el hilo de escucha en el cliente
                    return client.RC.OK
                elif response == b'\x01':
                    print("c> CONNECT FAIL, USER DOES NOT EXIST")
                    aux.stop_listen_thread(listen_thread)  # Detener el hilo de escucha si falla la conexión
                    return client.RC.USER_ERROR
                elif response == b'\x02':
                    print("c> USER ALREADY CONNECTED")
                    client._user = user
                    client._listen_thread = listen_thread  # Guardar el hilo de escucha en el cliente
                    return client.RC.ERROR
                else:
                    print("c> CONNECT FAIL")
                    aux.stop_listen_thread(listen_thread)  # Detener el hilo de escucha si falla la conexión
                    return client.RC.ERROR
        except Exception as e:
            print("c> CONNECT FAIL")
            return client.RC.ERROR
        
    
    @staticmethod
    def disconnect(user):
        DISCONNECT = 3

        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(DISCONNECT.to_bytes(4, byteorder='big'))

                client_ws = zeep.Client(wsdl=WSDL)
                datetime = client_ws.service.get_datetime()
                print("c> Momento del envío al servidor:", datetime)
                datetime = datetime.encode('utf-8') + b'\0'
                s.sendall(datetime)

                user_data = user.encode('utf-8') + b'\0'
                s.sendall(user_data)

                response = s.recv(1)
                if response == b'\x00':
                    print("c> DISCONNECT OK")
                    client._user = None
                    # Detener la ejecución del hilo y cerrar el puerto de escucha
                    aux.stop_listen_thread(client._listen_thread)
                    client._listen_thread = None
                    return client.RC.OK
                elif response == b'\x01':
                    print("c> DISCONNECT FAIL / USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                elif response == b'\x02':
                    print("c> DISCONNECT FAIL / USER NOT CONNECTED")
                    return client.RC.ERROR
                else:
                    print("c> DISCONNECT FAIL")
                    return client.RC.ERROR
        except Exception as e:
            print("c> DISCONNECT FAIL")
            return client.RC.ERROR

        

    @staticmethod
    def publish(file_name, description):
        PUBLISH = 4
        
        if client._user is None:
            print("c> PUBLISH FAIL, USER NOT CONNECTED")
            return client.RC.ERROR

        # Verificar si el fichero existe en la ruta del cliente
        if not os.path.exists("./" + file_name):
            print("c> PUBLISH FAIL, FILE DOES NOT EXIST")
            return client.RC.ERROR
        
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(PUBLISH.to_bytes(4, byteorder='big'))

                client_ws = zeep.Client(wsdl=WSDL)
                datetime = client_ws.service.get_datetime()
                print("c> Momento del envío al servidor:", datetime)
                datetime = datetime.encode('utf-8') + b'\0'
                s.sendall(datetime)

                user_data = client._user.encode('utf-8') + b'\0'
                s.sendall(user_data)
                
                
                file_name_data = file_name.encode('utf-8') + b'\0'
                s.sendall(file_name_data)
                
                description_data = description.encode('utf-8') + b'\0'
                s.sendall(description_data)

                response = s.recv(1)
                if response == b'\x00':
                    print("c> PUBLISH OK")
                    return client.RC.OK
                elif response == b'\x01':
                    print("c> PUBLISH FAIL, USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                elif response == b'\x02':
                    print("c> PUBLISH FAIL, USER NOT CONNECTED")
                    return client.RC.ERROR
                elif response == b'\x03':
                    print("c> PUBLISH FAIL, CONTENT ALREADY PUBLISHED")
                    return client.RC.ERROR
                else:
                    print("c> PUBLISH FAIL")
                    return client.RC.ERROR
        except Exception as e:
            print("c> PUBLISH FAIL")
            return client.RC.ERROR
        

    @staticmethod
    def delete(file_name):
        DELETE = 5

        if client._user is None:
            print("c> DELETE FAIL, USER NOT CONNECTED")
            return client.RC.ERROR

        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(DELETE.to_bytes(4, byteorder='big'))

                client_ws = zeep.Client(wsdl=WSDL)
                datetime = client_ws.service.get_datetime()
                print("c> Momento del envío al servidor:", datetime)
                datetime = datetime.encode('utf-8') + b'\0'
                s.sendall(datetime)

                user_data = client._user.encode('utf-8') + b'\0'
                s.sendall(user_data)
            
                file_name_data = file_name.encode('utf-8') + b'\0'
                s.sendall(file_name_data)

                response = s.recv(1)
                if response == b'\x00':
                    print("c> DELETE OK")
                    return client.RC.OK
                elif response == b'\x01':
                    print("c> DELETE FAIL, USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                elif response == b'\x02':
                    print("c> DELETE FAIL, USER NOT CONNECTED")
                    return client.RC.ERROR
                elif response == b'\x03':
                    print("c> DELETE FAIL, CONTENT NOT PUBLISHED")
                    return client.RC.ERROR
                else:
                    print("c> DELETE FAIL")
                    return client.RC.ERROR
        except Exception as e:
            print("c> DELETE FAIL")
            return client.RC.ERROR
        

    @staticmethod
    def listusers():
        LISTUSERS = 6

        if client._user is None:
            print("c> LIST_USERS FAIL, USER NOT CONNECTED")
            return client.RC.ERROR

        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(LISTUSERS.to_bytes(4, byteorder='big'))

                client_ws = zeep.Client(wsdl=WSDL)
                datetime = client_ws.service.get_datetime()
                print("c> Momento del envío al servidor:", datetime)
                datetime = datetime.encode('utf-8') + b'\0'
                s.sendall(datetime)

                print(f"Enviando nombre de usuario: {client._user}")
                user_data = client._user.encode('utf-8') + b'\0'
                s.sendall(user_data)
                
                response = s.recv(1)
                if response == b'\x00':
                    print("c> LIST_USERS OK")
                    num_users = s.recv(1024).decode('utf-8').rstrip('\0')  # Eliminar caracteres nulos al final
                    num_users = num_users.replace('\0', '')  # Eliminar todos los caracteres nulos
                    print(num_users)
                    # Procesar la información recibida y almacenarla en _users_info
                    for line in num_users.split('\n'):
                        line = line.strip()  # Eliminar espacios en blanco al inicio y al final
                        if line:  # Ignorar líneas vacías
                            parts = line.split()
                            if len(parts) == 3:  # Verificar que la línea tenga exactamente tres partes
                                username, ip, port = parts
                                client._users_info[username] = (ip, int(port))
                            else:
                                print(f"Error: Unexpected format in line '{line}'")
                    
                    print(client._users_info)
                    return client.RC.OK
                elif response == b'\x01':
                    print("c> LIST_USERS FAIL, USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                elif response == b'\x02':
                    print("c> LIST_USERS FAIL, USER NOT CONNECTED")
                    return client.RC.ERROR
                else:
                    print("c> LIST_USERS FAIL")
                    return client.RC.ERROR
        except Exception as e:
            print("c> LIST_USERS FAIL")
            return client.RC.ERROR

       

    @staticmethod
    def listcontent(remote_user):
        LIST_CONTENT = 7

        if client._user is None:
            print("c> LIST_CONTENT FAIL, USER NOT CONNECTED")
            return client.RC.ERROR

        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(LIST_CONTENT.to_bytes(4, byteorder='big'))

                client_ws = zeep.Client(wsdl=WSDL)
                datetime = client_ws.service.get_datetime()
                print("c> Momento del envío al servidor:", datetime)
                datetime = datetime.encode('utf-8') + b'\0'
                s.sendall(datetime)

                user_data = client._user.encode('utf-8') + b'\0'
                
                print("Enviando nombre de usuario: ", client._user)
                s.sendall(user_data)
                remote_user_data = remote_user.encode('utf-8') + b'\0'
                
                print("Enviando nombre de usuario remoto: ", remote_user)
                s.sendall(remote_user_data)

                response = s.recv(1)
                print("Respuesta del servidor:", response)
                if response == b'\x00':
                    print("c> LIST_CONTENT OK")
                    num_files = s.recv(1024).decode('utf-8')
                    print(num_files)
                    return client.RC.OK
                elif response == b'\x01':
                    print("c> LIST_CONTENT FAIL, USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                elif response == b'\x02':
                    print("c> LIST_CONTENT FAIL, USER NOT CONNECTED")
                    return client.RC.ERROR
                elif response == b'\x03':
                    print("c> LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                else:
                    print("c> LIST_CONTENT FAIL")
                    return client.RC.ERROR
        except Exception as e:
            print("c> LIST_CONTENT FAIL")
            return client.RC.ERROR

        

    @staticmethod
    def getfile(user, remote_FileName, local_FileName):
        GET_FILE = 8
        result = 0

        if client._user is None:
            print("c> GET_FILE FAIL, USER NOT CONNECTED")
            return 2

        # Verificar si el usuario existe en el diccionario
        if user in client._users_info:
            # Obtener la tupla (IP, puerto) asociada al usuario
            ip, port = client._users_info[user]
            try:
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
                        client_socket.connect((ip, port))
                        client_ws = zeep.Client(wsdl=WSDL)
                        datetime = client_ws.service.get_datetime()
                        print("c> Momento del envío al servidor:", datetime)
                        datetime = datetime.encode('utf-8') + b'\0'
                        client_socket.sendall(datetime)

                        
                        client_socket.sendall(remote_FileName.encode('utf-8') + b'\0')

                        response = client_socket.recv(1)
                        if response == b'\x00':
                            with open(local_FileName, 'wb') as f:
                                while True:
                                    data = client_socket.recv(1024)
                                    if not data:
                                        break
                                    f.write(data)
                            result = 0
                            print("c> GET_FILE OK")
                        elif response == b'\x01':
                            print("c> GET_FILE FAIL / FILE NOT EXIST")
                            return client.RC.USER_ERROR
                        else:
                            print("c> GET_FILE FAIL")
                            return client.RC.ERROR
            except Exception as e:
                print("c> GET_FILE FAIL")
                return client.RC.ERROR


        else:
            print("c> GET_FILE FAIL, REMOTE USER DOES NOT EXIST")
            return client.RC.USER_ERROR

        if result != 0 and os.path.exists(local_FileName):
            os.remove(local_FileName)

        return result

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
