#include <stdio.h>
#include <string.h>
#include "header.h"

int socketFd;
int myId;
char myName[NAME_SIZE];
struct sockaddr_in broadcastAddress; // куда рассылаются сообщения

// UDP-сокет привязывается к порту PORT. на этот же порт приходят пакеты от остальных участников
int createSocket(void) {
    struct sockaddr_in address;
    int option = 1;

    socketFd = socket(AF_INET, SOCK_DGRAM, 0); // создали сокет
    if (socketFd == -1) {
        perror("socket");
        return -1;
    }

    // несколько участников могут работать и слушать один порт
    setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    setsockopt(socketFd, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option));

    // разрешаем отправку на широковещательный адрес
    if (setsockopt(socketFd, SOL_SOCKET, SO_BROADCAST, &option, sizeof(option)) == -1) {
        perror("setsockopt");
        return -1;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY); // принимаем пакеты со всех интерфейсов
    address.sin_port = htons(PORT);
    if (bind(socketFd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("bind");
        return -1;
    }

    memset(&broadcastAddress, 0, sizeof(broadcastAddress));
    broadcastAddress.sin_family = AF_INET;
    broadcastAddress.sin_addr.s_addr = inet_addr(BROADCAST_ADDRESS);
    broadcastAddress.sin_port = htons(PORT);

    return 0;
}

// пакет уходит на широковещательный адрес
void sendPacket(int type, const char* text) {
    Packet packet;

    memset(&packet, 0, sizeof(packet));
    packet.type = type;
    packet.id = myId;
    strncpy(packet.name, myName, NAME_SIZE - 1);
    if (text != NULL)
        strncpy(packet.text, text, TEXT_SIZE - 1);

    if (sendto(socketFd, &packet, sizeof(packet), 0,
               (struct sockaddr*)&broadcastAddress, sizeof(broadcastAddress)) == -1) // отправляем пакет
        perror("sendto");
}

// ждет пакеты от остальных участников и выводит их на экран
void* runReceiver(void* argument) {
    Packet packet;
    struct sockaddr_in sender;
    socklen_t length;
    ssize_t received;

    while (1) {
        length = sizeof(sender);
        received = recvfrom(socketFd, &packet, sizeof(packet), 0,
                            (struct sockaddr*)&sender, &length); // ждем что-нибудь 
        if (received != sizeof(packet)) // сокет закрыт или пришел чужой пакет
            break;

        if (packet.id == myId) // собственная рассылка вернулась обратно
            continue;

        packet.name[NAME_SIZE - 1] = '\0';
        packet.text[TEXT_SIZE - 1] = '\0';

        switch (packet.type) {
            case JOIN:
                printf("в сети новый участник: %s (%s)\n", packet.name, inet_ntoa(sender.sin_addr));
                break;
            case MESSAGE:
                printf("%s: %s\n", packet.name, packet.text);
                break;
            case LEAVE:
                printf("участник %s вышел из сети\n", packet.name);
                break;
        }
        fflush(stdout);
    }

    return NULL;
}
