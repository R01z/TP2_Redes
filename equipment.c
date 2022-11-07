#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
//Windows não possui "sys/socket.h", esse programa rodará apenas no linux

#define BUFSZ 1024

void usage(int argc, char **argv){
    printf("ERRO NA CHAMADA\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
    if(argc < 3) usage(argc, argv); //Verificar chamada correta

    size_t count=0;

    //Chamada do connect
    struct sockaddr_storage storage;
    if(addrparse(argv[1], argv[2], &storage) !=0) usage(argc, argv);

    //Criar Socket
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if(s == -1) logexit("socket");

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if(connect(s, addr, sizeof(storage)) != 0) logexit("connect");

    //Imprimir endereço conectado
    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("[log]Conectado a %s\n", addrstr);

    //Comunicação cliente-servidor
    char buf[BUFSZ];
    unsigned total = 0;
    while(1){
        memset(buf, 0, BUFSZ);
        printf("[msg]Cliente > ");
        fgets(buf, BUFSZ-1, stdin);
        count = send(s, buf, strlen(buf)+1, 0);
        if(count != strlen(buf)+1) logexit("send");

        memset(buf, 0, BUFSZ);
        total = 0;
        
        //Receber mensagem
        count = recv(s, buf + total, BUFSZ - total, 0);
        
        printf("[msg]Server > %s",buf);

        //Encerra conexão
        if(strncmp(buf,"Conexao Encerrada", 17) == 0){
            break;
        }
    }

    printf("[log] Conexão Encerrada\n");
    close(s);

    exit(EXIT_SUCCESS);
    return 0;
}