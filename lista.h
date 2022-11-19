#include "no.h"

typedef struct Lista Lista;

struct Lista
{
    equip* cabeca;
    equip* cauda;
    int tamanho;
};

int tamanho(Lista *lista);
void insere(Lista *lista, equip *equipamento);
void remove(Lista *lista, int id);
equip* busca(Lista *lista, int id);