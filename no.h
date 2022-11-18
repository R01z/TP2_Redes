#include <sys/types.h>
#include <sys/socket.h>


typedef struct equipment_data equip;
struct equipment_data{
    int id;
    int csock;
    struct sockaddr_storage storage;

    equip *proximo;
};

equip* criaEquip(int id, int csock, struct sockaddr_storage storage);