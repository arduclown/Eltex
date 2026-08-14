#ifndef HEADER_H
#define HEADER_H

#include <arpa/inet.h> // inet_addr()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h> // socket(), bind(), setsockopt(), sendto(), recvfrom()
#include <unistd.h> // close(), getpid()

#define PORT 5000 // порт, на котором работают все участники чата
#define BROADCAST_ADDRESS "192.168.64.255" // широковещательный адрес

#define NAME_SIZE 32
#define TEXT_SIZE 256

#define JOIN 1 // участник появился в сети
#define MESSAGE 2 // обычное сообщение чата
#define LEAVE 3 // участник вышел из сети

typedef struct {
    int type; // JOIN, MESSAGE или LEAVE
    int id; // идентификатор отправителя (pid), чтобы не показывать свои же сообщения
    char name[NAME_SIZE];
    char text[TEXT_SIZE];
} Packet;

extern int socketFd;
extern int myId;
extern char myName[NAME_SIZE];
extern struct sockaddr_in broadcastAddress;

int createSocket(void); // создать UDP-сокет, разрешить рассылку и привязать его к порту
void sendPacket(int type, const char* text); // отправить пакет всем участникам
void* runReceiver(void* argument); // поток приема чужих пакетов

#endif
