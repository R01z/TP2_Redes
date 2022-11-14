#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024
#define CONEXOES 10

void usage(int argc, char **argv){
    printf("usage\n");
    exit(EXIT_FAILURE);
}

struct client_data{
    int id;
    int csock;
    struct sockaddr_storage storage;
};

void * client_thread(void *data){
    struct client_data *cdata = (struct client_data *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log]Conexão de %s\n", caddrstr);
    
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    while(1){
        memset(buf, 0, BUFSZ);
        size_t count = recv(cdata->csock, buf, BUFSZ, 0);
        printf("[msg]Cliente %d > %s",cdata->id, buf);
        
        //Encerra conexão
        if(strcmp(buf,"exit\n") == 0) {
            sprintf(buf, "Conexao Encerrada\n");
            break;
        }


        sprintf(buf, "Mensagem recebida\n");
        printf("[msg]Servidor > %s", buf);
        count = send(cdata->csock, buf, strlen(buf)+1, 0);
        if(count != strlen(buf)+1) logexit("send");
    }

    send(cdata->csock, buf, strlen(buf)+1, 0);
    printf("[log]%s",buf);
    close(cdata->csock);

    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){
    if(argc < 3) usage(argc, argv); //Verificar chamada correta

    char buf[BUFSZ];
    size_t count;

    //Chamada do connect
    struct sockaddr_storage storage;
    if(server_sockaddr_init(argv[1], argv[2], &storage) !=0) usage(argc, argv);

    //Criar Socket
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if(s == -1) logexit("socket");

    int enable = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if(bind(s, addr, sizeof(storage))!=0) logexit("bind");

    if(listen(s, 10)!=0) logexit("listen");

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("[log]Conectado a %s, aguardando conexões\n", addrstr);
    
    int ids = 0;
    while(1){
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if(csock == -1) logexit("accept");

        ids++;

        struct client_data *cdata = malloc(sizeof(*cdata));
        if(!cdata){
            logexit("Malloc");
        }
        cdata->csock = csock;
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));
        cdata->id = ids;

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);
    }
    exit(EXIT_SUCCESS);
    return 0;
}