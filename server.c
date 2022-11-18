#include "common.h"
#include "no.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define BUFSZ 1024
#define THREAD_NUMBER 10

pthread_t tid[THREAD_NUMBER];
int socketsList[THREAD_NUMBER];
int threadsOcupadas[THREAD_NUMBER];

void usage(int argc, char **argv){
    printf("usage\n");
    exit(EXIT_FAILURE);
}

void enviaMensagem(const char *msg, int socket){
    int count = send(socket, msg, strlen(msg), 0);
    if(count != strlen(msg)) logexit("Envia Mensagem");
}

void broadcastMessage(const char *msg){
    int count;
    for(int i=0;i<THREAD_NUMBER;i++)
        if(threadsOcupadas[i]){
            enviaMensagem(msg, socketsList[i]);
        }
}

void errorMessage(const char *buf, int i){
    memset(buf, 0, BUFSZ);
    switch(i){
        case 1:
            sprintf(buf, "Equipment not found\n");
            break;
        case 2:
            sprintf(buf, "Source equipment not found\n");
            break;
        case 3:
            sprintf(buf, "Target equipment not found\n");
            break;
        case 4:
            sprintf(buf, "Equipment limit exceeded\n");
            break;
        default:
            sprintf(buf, "Unknow error\n");
    }
}

void trataMensagem(const char *buf){

}

void * client_thread(void *data){
    equip *cdata = (equip *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);

    //Equipamento entra na lista de sockets
    socketsList[cdata->id-1] = cdata->csock;
    threadsOcupadas[cdata->id-1] = 1;


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

        sprintf(buf, "Broadcast: mensagem de %d\n",cdata->id);
        printf("[msg]Servidor > %s", buf);
        //count = send(socketsList[cdata->id-1], buf, strlen(buf)+1, 0);
        //if(count != strlen(buf)+1) logexit("send");
        broadcastMessage(buf);
    }

    send(cdata->csock, buf, strlen(buf)+1, 0);
    printf("[log]%s",buf);
    socketsList[cdata->id-1] = 0;
    threadsOcupadas[cdata->id-1] = 0;
    close(cdata->csock);

    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){
    if(argc < 2) usage(argc, argv); //Verificar chamada correta

    char buf[BUFSZ];
    size_t count;

    //Limpa threads
    for(int i=0;i<THREAD_NUMBER;i++) socketsList[i] = 0;

    //Chamada do connect
    struct sockaddr_storage storage;
    if(server_sockaddr_init("v4", argv[1], &storage) !=0) usage(argc, argv);

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
    
    int ids = 0;// Controla quantidade de conexões
    while(1){
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if(csock == -1) logexit("accept");

        ids++;
        if(ids>THREAD_NUMBER){
            errorMessage(buf, 4);
            enviaMensagem(buf, csock);
            close(csock);
            continue;
        }
        
        equip *cdata = criaEquip(ids, csock, cstorage);

        //Envia broadcast da adição de equipamento
        memset(buf, 0, BUFSZ);
        sprintf(buf,"Equipment 0%d added\n", ids);
        printf(buf);
        broadcastMessage(buf);

        memset(buf, 0, BUFSZ);
        sprintf(buf,"New ID: 0%d\n", ids);
        enviaMensagem(buf, csock);

        //Cria thread para equipamento
        pthread_create(&(tid[ids-1]), NULL, client_thread, cdata);
    }
    exit(EXIT_SUCCESS);
    return 0;
}