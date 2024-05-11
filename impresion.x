struct entrada {
    string user<256>; // Nombre de usuario
    int operacion; // Entero
    string file<256> // Nombre del fichero
    string datetime<20>; // Cadena de 20 caracteres
};

program RPC_PROG {
    version RPC_VERS {
        void impimir_peticion(entrada) = 0;
    } = 1;6
} = 1;
