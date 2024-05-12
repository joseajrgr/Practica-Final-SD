struct entrada {
    string user<256>;
    int operacion;
    string file<256>;
    string datetime<20>;
};

program RPC_PROG {
    version RPC_VERS {
        void IMPRIMIR(entrada) = 0;
    } = 1;
} = 99;
