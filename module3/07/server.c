#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 32
#define MAX_EVENTS 32

#define NAME_SIZE 32
#define FILE_NAME_SIZE 256
#define DATA_SIZE 1024

// события 
#define TYPE_JOIN 1 // новый участник
#define TYPE_MESSAGE 2 // сообщение
#define TYPE_LEAVE 3 // выход
#define TYPE_FILE_START 4 // пересылка файла
#define TYPE_FILE_DATA 5 
#define TYPE_FILE_END 6

typedef struct {
    uint32_t type; // тип события
    uint32_t data_len; // длина данных
    char name[NAME_SIZE]; // имя отправителя
    char filename[FILE_NAME_SIZE]; // имя файла
    char data[DATA_SIZE]; // содержимое файла
} Packet;

typedef struct {
    int sock; // дескриптор сокета
    char name[NAME_SIZE];
    Packet packet;
    size_t received_bytes; // сколько байт принято 
} Client;

static void error(const char *msg) {
    perror(msg);
    exit(1);
}

static int send_all(int sock, const void *buffer, size_t size) {
    const char *p = (const char *)buffer;
    size_t sent = 0;

    while (sent < size) {
        ssize_t n = send(sock, p + sent, size - sent, 0);
        if (n <= 0)
            return -1;
        sent += (size_t)n;
    }
    return 0;
}

static int find_client(Client clients[], int sock) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock == sock)
            return i;
    }
    return -1;
}

static int add_client(Client clients[], int sock) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock == -1) {
            clients[i].sock = sock;
            clients[i].name[0] = '\0';
            clients[i].received_bytes = 0;
            memset(&clients[i].packet, 0, sizeof(clients[i].packet));
            return i;
        }
    }
    return -1;
}

static void broadcast_packet(Client clients[], int sender_sock, const Packet *packet) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock != -1 && clients[i].sock != sender_sock) {
            if (send_all(clients[i].sock, packet, sizeof(*packet)) < 0) {
                /* Клиент будет удалён, когда epoll сообщит о разрыве. */
            }
        }
    }
}

// пакет для выхода
static void make_leave_packet(Packet *packet, const char *name) {
    memset(packet, 0, sizeof(*packet));
    packet->type = htonl(TYPE_LEAVE); // устанивливаем тип выхода
    strncpy(packet->name, name, NAME_SIZE - 1);
}

static void remove_client(int epoll_fd, Client clients[], int index, int notify) {
    if (index < 0 || clients[index].sock == -1)
        return;

    int sock = clients[index].sock;

    if (notify && clients[index].name[0] != '\0') {
        Packet leave_packet;
        make_leave_packet(&leave_packet, clients[index].name);
        broadcast_packet(clients, sock, &leave_packet);
        printf("%s отключился\n", clients[index].name);
    }

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock, NULL); // удаление сокета из epoll
    close(sock);
    clients[index].sock = -1;
    clients[index].name[0] = '\0';
    clients[index].received_bytes = 0;
}

static int process_packet(Client clients[], int index, const Packet *packet) {
    uint32_t type = ntohl(packet->type);
    uint32_t data_len = ntohl(packet->data_len);

    if (data_len > DATA_SIZE)
        return 0;

    if (type == TYPE_JOIN) {
        strncpy(clients[index].name, packet->name, NAME_SIZE - 1);
        clients[index].name[NAME_SIZE - 1] = '\0';
        printf("Подключился: %s\n", clients[index].name);
    }

    broadcast_packet(clients, clients[index].sock, packet);

    if (type == TYPE_MESSAGE) {
        char text[DATA_SIZE + 1];
        memcpy(text, packet->data, data_len);
        text[data_len] = '\0';
        printf("%s: %s\n", packet->name, text);
    } else if (type == TYPE_FILE_START) {
        printf("%s отправляет файл %s\n", packet->name, packet->filename);
    } else if (type == TYPE_FILE_END) {
        printf("Файл %s от %s переслан всем\n", packet->filename, packet->name);
    } else if (type == TYPE_LEAVE) {
        printf("%s вышел из чата\n", packet->name);
        return 1;
    }

    fflush(stdout);
    return 0;
}

int main(int argc, char *argv[]) {
    int listen_sock;
    int epoll_fd;
    int portno;
    struct sockaddr_in serv_addr;
    Client clients[MAX_CLIENTS];
    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];

    if (argc != 2) {
        fprintf(stderr, "Использование: %s port\n", argv[0]);
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    portno = atoi(argv[1]);
    if (portno <= 0 || portno > 65535) {
        fprintf(stderr, "ERROR: неверный порт\n");
        return 1;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sock = -1; // слот свободен
        clients[i].name[0] = '\0';
        clients[i].received_bytes = 0;
    }

    listen_sock = socket(AF_INET, SOCK_STREAM, 0); // слущающий сокет
    if (listen_sock < 0)
        error("socket");

    int option = 1;
    // чтобы сразу переиспользовать порт сразу после закрытия сокета
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
        error("setsockopt");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portno);

    // привязываем сокет к адресу и порту
    if (bind(listen_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("bind");
    
    // переходим в режим прослушивания
    if (listen(listen_sock, 10) < 0)
        error("listen");

    // создаем епол дескриптор
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0)
        error("epoll_create1");

    memset(&event, 0, sizeof(event));
    // добавляем слущающий сокет
    event.events = EPOLLIN; // готовность к чтению всех сокетов 
    event.data.fd = listen_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &event) < 0)
        error("epoll_ctl");

    printf("TCP chat server запущен на порту %d\n", portno);

    while (1) {
        // блокируем выполнение пока не произойдет хотя бы одно событие на отслеживаемых дескр.
        int count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (count < 0) {
            if (errno == EINTR)
                continue;
            error("epoll_wait");
        }

        for (int e = 0; e < count; e++) {
            int fd = events[e].data.fd;

            // событие на слущающем сокете 
            if (fd == listen_sock) {
                struct sockaddr_in cli_addr;
                socklen_t cli_len = sizeof(cli_addr);
                // первое ожидающее соединение
                int client_sock = accept(listen_sock, (struct sockaddr *)&cli_addr, &cli_len);
                if (client_sock < 0) {
                    perror("accept");
                    continue;
                }
                // заполняем массив клиентов
                int index = add_client(clients, client_sock);
                if (index < 0) {
                    fprintf(stderr, "Слишком много клиентов\n");
                    close(client_sock);
                    continue;
                }

                memset(&event, 0, sizeof(event));
                event.events = EPOLLIN | EPOLLRDHUP | EPOLLET; // читать и отслеживать закрытие (EPOLLRDHUP удалённая строна отключилас/закрыла соединение)
                event.data.fd = client_sock;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &event) < 0) {
                    perror("epoll_ctl");
                    close(client_sock);
                    clients[index].sock = -1;
                    continue;
                }

                printf("Новое TCP-соединение: %s:%d\n",
                       inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                continue;
            }

            // событие на клиентском сокете
            // ищем индекс клиента
            int index = find_client(clients, fd);
            if (index < 0)
                continue;

            // проверка на ошибки и зыкрытия
            if (events[e].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                remove_client(epoll_fd, clients, index, 1);
                continue;
            }

            // обработка данных (EPOLLIN означает, что есть данные для чтения)
            if (events[e].events & EPOLLIN) {
                int disconnected = 0;

                while (1) {
                    ssize_t n = recv(fd,
                                     ((char *)&clients[index].packet) + clients[index].received_bytes,
                                     sizeof(Packet) - clients[index].received_bytes,
                                     MSG_DONTWAIT); // 1 - дескриптор, 2 - указатель на начало свободной памяти в буфере, 3 - сколько байт до получения полного пакета, 4 - неблок. чтение

                    // прочитали
                    if (n > 0) {
                        clients[index].received_bytes += (size_t)n;
                        
                        // пакет получен целиком 
                        if (clients[index].received_bytes == sizeof(Packet)) {
                            // смотрим тип пакета
                            int should_close = process_packet(clients, index, &clients[index].packet);
                            // сброс состояния для след. пакета
                            clients[index].received_bytes = 0;
                            memset(&clients[index].packet, 0, sizeof(clients[index].packet));

                            if (should_close) {
                                remove_client(epoll_fd, clients, index, 0);
                                disconnected = 1;
                                break;
                            }
                        }
                        continue;
                    }

                    if (n == 0) {
                        remove_client(epoll_fd, clients, index, 1);
                        disconnected = 1;
                        break;
                    }

                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        break;

                    perror("recv");
                    remove_client(epoll_fd, clients, index, 1);
                    disconnected = 1;
                    break;
                }

                if (disconnected)
                    continue;
            }
        }
    }

    close(epoll_fd);
    close(listen_sock);
    return 0;
}
