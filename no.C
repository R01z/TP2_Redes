#include <stdio.h>
#include <stdlib.h>
#include "no.h"

equip* criaEquip(int id, int csock, struct sockaddr_storage cstorage){
    struct equip *cdata = malloc(sizeof(*cdata));
    cdata->csock = csock;
    memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));
    cdata->id = id;

    cdata->proximo = NULL;
}