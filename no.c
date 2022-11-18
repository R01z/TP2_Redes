#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "no.h"

equip* criaEquip(int id, int csock, struct sockaddr_storage cstorage){
    equip *cdata = (equip*)malloc(sizeof(*cdata));
    cdata->csock = csock;
    memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));
    cdata->id = id;

    cdata->proximo = NULL;
}