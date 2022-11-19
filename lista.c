#include <stdio.h>
#include <stdlib.h>
#include "lista.h"
#include "no.h"

int tamanho(Lista *lista){
    return lista->tamanho;
}

void insere(Lista *lista, equip *equipamento){
    equip *novo = equipamento;
    lista->cauda->proximo = novo;
    lista->cauda = novo;
}

void remove(Lista *lista, int id){
    equip *aux = lista->cabeca;
    if(aux->id != id){
        
    }
}