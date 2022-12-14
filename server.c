#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024
#define THREAD_NUMBER 10

pthread_t tid[THREAD_NUMBER];
int socketsList[THREAD_NUMBER];
int threadsOcupadas[THREAD_NUMBER];

void usage(int argc, char **argv){
    printf("usage\n");
    exit(EXIT_FAILURE);
}

struct equipment_data{
    int id;
    int csock;
    struct sockaddr_storage storage;
};

void pegaLista(const char *buf, int id){
    char aux[5];
    memset(buf, 0, BUFSZ);

    for(int i=0;i<THREAD_NUMBER;i++){
        if(i+1 != id && threadsOcupadas[i] == 1){
            sprintf(aux, "0%d ", i+1);
            strcat(buf, aux);
        }
    }
    strcat(buf, "\n");
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

void trataMensagem(const char *buf, struct equipment_data* cdata){
    if(threadsOcupadas[cdata->id-1] == 0){
        errorMessage(buf, 2);
        enviaMensagem(buf, cdata->csock);
    }
    else if(strcmp(buf, "list equipment\n") == 0){
        pegaLista(buf, cdata->id);
        enviaMensagem(buf, cdata->csock);
    }
    else{
        char *tokens;
        tokens = strtok(buf,"0");
        if(strcmp(tokens,"request information from ") == 0){
            tokens = strtok(NULL, "\n");
            int req = atoi(tokens);
            if(threadsOcupadas[req-1] != 1){
                printf("Equipment 0%d not found\n",req);
                errorMessage(buf, 3);
                enviaMensagem(buf, cdata->csock);
            }
            else{
                memset(buf, 0, BUFSZ);
                sprintf(buf,"requested information\n");
                enviaMensagem(buf,socketsList[req-1]);
                memset(buf, 0, BUFSZ);
                size_t count = recv(socketsList[req-1], buf, BUFSZ, 0);
                enviaMensagem(buf, cdata->csock);
            }
        }
        else{
            errorMessage(buf,0);
            enviaMensagem(buf, cdata->csock);
        }
    }
}

void * client_thread(void *data){
    struct equipment_data *cdata = (struct equipment_data *)data;
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
        
        //Encerra conex??o
        if(strcmp(buf,"close connection\n") == 0) {
            socketsList[cdata->id-1] = 0;
            threadsOcupadas[cdata->id-1] = 0;
            sprintf(buf, "Success\n");
            enviaMensagem(buf, cdata->csock);
            sprintf(buf, "Equipment 0%d removed\n", cdata->id);
            broadcastMessage(buf);
            break;
        }
        else{
            trataMensagem(buf, cdata);
        }
    }

    printf(buf);
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

    int ids = 0;// Controla quantidade de conex??es
    while(1){
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if(csock == -1) logexit("accept");

        for(int i=0;i<THREAD_NUMBER;i++){
            if(threadsOcupadas[i] == 0){
                ids = i+1;
                break;
            }
            else ids = THREAD_NUMBER+10;
        }
        if(ids>THREAD_NUMBER){
            errorMessage(buf, 4);
            enviaMensagem(buf, csock);
            close(csock);
            continue;
        }
        
        struct equipment_data *cdata = malloc(sizeof(*cdata));
        if(!cdata){
            logexit("Malloc");
        }
        cdata->csock = csock;
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));
        cdata->id = ids;

        //Envia broadcast da adi????o de equipamento
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