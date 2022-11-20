#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
//Windows não possui "sys/socket.h", esse programa rodará apenas no linux

#define BUFSZ 1024
#define THREAD_NUMBER 3

int id;
int s;

void usage(int argc, char **argv){
    printf("ERRO NA CHAMADA\n");
    exit(EXIT_FAILURE);
}

void obtemId(const char *buf){
    char *tokens;
    tokens = strtok(buf,":");
    if(strcmp(tokens,"New ID") == 0){
        tokens = strtok(NULL,"\n");
        id = atoi(tokens);
    }
}

void obtemInfo(const char *buf){
    memset(buf, 0, BUFSZ);
    float dado = rand()%1000/100.0;
    sprintf(buf,"Value from 0%d: %.2f\n",id,dado);
}

void recebeMensagem(void *arg){
    int csock = *((int *)arg);

    int count;
    char buf[BUFSZ];

    while (1){
       //Receber mensagem
        memset(buf, 0, BUFSZ);
        count = recv(csock, buf, BUFSZ, 0);
        printf("%s",buf);
        if(strcmp(buf, "Success\n") == 0){
            break;
        }
        else if(strcmp(buf,"requested information\n") == 0){
            obtemInfo(buf);
            count = send(s, buf, strlen(buf), 0);
        }
    }

    close(csock);

    exit(EXIT_SUCCESS);
    
}

int main(int argc, char **argv){
    if(argc < 3) usage(argc, argv); //Verificar chamada correta

    size_t count=0;
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    //Chamada do connect
    struct sockaddr_storage storage;
    if(addrparse(argv[1], argv[2], &storage) !=0) usage(argc, argv);

    //Criar Socket
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if(s == -1) logexit("socket");

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if(connect(s, addr, sizeof(storage)) != 0) logexit("connect");

    //Confirmar conexão
    count = recv(s, buf, BUFSZ, 0);
    printf(buf);
    if(strcmp(buf,"Equipment limit exceeded\n") == 0){
        close(s);
        exit(EXIT_SUCCESS);
        return 0;
    }
    else obtemId(buf);

    //Comunicação cliente-servidor
    unsigned total = 0;
    pthread_t tid; //Mensagens serão recebidas usando thread

    pthread_create(&tid, NULL, recebeMensagem, (void *)&s);

    while(1){
        memset(buf, 0, BUFSZ);
        fgets(buf, BUFSZ-1, stdin);
        count = send(s, buf, strlen(buf)+1, 0);
        if(count != strlen(buf)+1) logexit("send");
    }

    printf("[log] Conexão Encerrada\n");
    close(s);

    exit(EXIT_SUCCESS);
    return 0;
}