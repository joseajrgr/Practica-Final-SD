import logging
from wsgiref.simple_server import make_server
from spyne import Application, Unicode, rpc, ServiceBase
from spyne.protocol.soap import Soap11
from spyne.server.wsgi import WsgiApplication
import datetime

class ServicioFechaHora(ServiceBase):
    @rpc(_returns=Unicode)
    def get_datetime(ctx):
        fecha_hora = datetime.datetime.now()
        fecha_hora_formateada = fecha_hora.strftime("%d-%m-%Y %H:%M:%S")
        return fecha_hora_formateada

application = Application([ServicioFechaHora],
    tns='http://testnamespace.com',
    in_protocol=Soap11(validator='lxml'),
    out_protocol=Soap11()
)
application = WsgiApplication(application)

if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    logging.getLogger('spyne.protocol.xml').setLevel(logging.DEBUG)
    logging.info("listening to http://127.0.0.1:7900; wsdl is at: http://localhost:7900/?wsdl ")
    server = make_server('localhost', 7900, application)
    print("El servidor está corriendo en http://localhost:7900")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        server.shutdown()
        print("El servidor se ha cerrado correctamente.")
