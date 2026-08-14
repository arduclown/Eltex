#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define NAME_SIZE 32
#define FILE_NAME_SIZE 256
#define DATA_SIZE 1024

#define TYPE_JOIN 1
#define TYPE_MESSAGE 2
#define TYPE_LEAVE 3
#define TYPE_FILE_START 4
#define TYPE_FILE_DATA 5
#define TYPE_FILE_END 6

typedef struct {
    uint32_t type;
    uint32_t data_len;
    char name[NAME_SIZE];
    char filename[FILE_NAME_SIZE];
    char data[DATA_SIZE];
} Packet;

static int sockfd = -1;
static char my_name[NAME_SIZE];

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

static void init_packet(Packet *packet, uint32_t type) {
    memset(packet, 0, sizeof(*packet));
    packet->type = htonl(type);
    strncpy(packet->name, my_name, NAME_SIZE - 1);
}

// джоин, мэсседж,лив
static int send_simple_packet(uint32_t type, const char *text) {
    Packet packet;
    size_t len = 0;

    init_packet(&packet, type);

    if (text != NULL) {
        len = strlen(text);
        if (len > DATA_SIZE - 1)
            len = DATA_SIZE - 1;
        memcpy(packet.data, text, len);
    }

    packet.data_len = htonl((uint32_t)len);
    return send_all(sockfd, &packet, sizeof(packet));
}

// функция для извлечения имени
static const char *base_name(const char *path) {
    const char *slash1 = strrchr(path, '/');
    const char *slash2 = strrchr(path, '\\');
    const char *base = path;

    if (slash1 && slash1 + 1 > base)
        base = slash1 + 1;
    if (slash2 && slash2 + 1 > base)
        base = slash2 + 1;

    return base;
}

// формируем имя для полученного файла
static void make_received_name(char *out, size_t out_size,
                               const char *sender, const char *filename) {
    const char *base = base_name(filename);
    snprintf(out, out_size, "received_%s_%s", sender, base);
}

static int send_file(const char *path) {
    FILE *file = fopen(path, "rb"); // открыли для чтения
    if (!file) {
        perror("fopen");
        return -1;
    }

    const char *filename = base_name(path);
    Packet packet;

    init_packet(&packet, TYPE_FILE_START);
    strncpy(packet.filename, filename, FILE_NAME_SIZE - 1);
    if (send_all(sockfd, &packet, sizeof(packet)) < 0) {
        fclose(file);
        return -1;
    }

    while (1) {
        char chunk[DATA_SIZE];
        size_t n = fread(chunk, 1, sizeof(chunk), file);

        if (n > 0) {
            init_packet(&packet, TYPE_FILE_DATA);
            strncpy(packet.filename, filename, FILE_NAME_SIZE - 1);
            memcpy(packet.data, chunk, n);
            packet.data_len = htonl((uint32_t)n);

            if (send_all(sockfd, &packet, sizeof(packet)) < 0) {
                fclose(file);
                return -1;
            }
        }

        if (n < sizeof(chunk)) {
            if (ferror(file)) {
                perror("fread");
                fclose(file);
                return -1;
            }
            break;
        }
    }

    fclose(file);

    init_packet(&packet, TYPE_FILE_END);
    strncpy(packet.filename, filename, FILE_NAME_SIZE - 1);
    if (send_all(sockfd, &packet, sizeof(packet)) < 0)
        return -1;

    printf("Файл %s отправлен\n", filename);
    return 0;
}
 // входящие пакеты
static void handle_packet(Packet *packet) {
    uint32_t type = ntohl(packet->type);
    uint32_t data_len = ntohl(packet->data_len);

    packet->name[NAME_SIZE - 1] = '\0';
    packet->filename[FILE_NAME_SIZE - 1] = '\0';
    if (data_len > DATA_SIZE)
        data_len = DATA_SIZE;

    switch (type) {
        case TYPE_JOIN:
            printf("в сети новый участник: %s\n", packet->name);
            break;

        case TYPE_MESSAGE:
            if (data_len < DATA_SIZE)
                packet->data[data_len] = '\0';
            else
                packet->data[DATA_SIZE - 1] = '\0';
            printf("%s: %s\n", packet->name, packet->data);
            break;

        case TYPE_LEAVE:
            printf("участник %s вышел из сети\n", packet->name);
            break;

        case TYPE_FILE_START: {
            char local_name[NAME_SIZE + FILE_NAME_SIZE + 32];
            make_received_name(local_name, sizeof(local_name), packet->name, packet->filename);
            FILE *file = fopen(local_name, "wb");
            if (file)
                fclose(file);
            printf("%s отправляет файл %s\n", packet->name, packet->filename);
            break;
        }

        case TYPE_FILE_DATA: {
            char local_name[NAME_SIZE + FILE_NAME_SIZE + 32];
            make_received_name(local_name, sizeof(local_name), packet->name, packet->filename);
            FILE *file = fopen(local_name, "ab");
            if (!file) {
                perror("fopen");
                break;
            }
            fwrite(packet->data, 1, data_len, file);
            fclose(file);
            break;
        }

        case TYPE_FILE_END: {
            char local_name[NAME_SIZE + FILE_NAME_SIZE + 32];
            make_received_name(local_name, sizeof(local_name), packet->name, packet->filename);
            printf("файл от %s получен: %s\n", packet->name, local_name);
            break;
        }
    }

    fflush(stdout);
}

int main(int argc, char *argv[]) {
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    struct pollfd fds[2];
    Packet incoming;
    size_t received_bytes = 0;
    char input[DATA_SIZE + 32];

    if (argc != 4) {
        fprintf(stderr, "Использование: %s hostname port имя\n", argv[0]);
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    portno = atoi(argv[2]);
    if (portno <= 0 || portno > 65535) {
        fprintf(stderr, "ERROR: неверный порт\n");
        return 1;
    }

    memset(my_name, 0, sizeof(my_name));
    strncpy(my_name, argv[3], NAME_SIZE - 1);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR: сервер не найден\n");
        close(sockfd);
        return 1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("connect");

    if (send_simple_packet(TYPE_JOIN, NULL) < 0)
        error("send JOIN");

    printf("%s подключен к чату\n", my_name);
    printf("Команды: /file путь_к_файлу, /quit\n");

    // настройка poll
    fds[0].fd = STDIN_FILENO; // стандартный ввод
    fds[0].events = POLLIN;
    fds[1].fd = sockfd; // сокет
    fds[1].events = POLLIN;

    while (1) {
        int ready = poll(fds, 2, -1);
        if (ready < 0) {
            perror("poll");
            break;
        }

        // читаем данные
        if (fds[1].revents & (POLLIN | POLLHUP | POLLERR)) {
            ssize_t n = recv(sockfd,
                             ((char *)&incoming) + received_bytes,
                             sizeof(incoming) - received_bytes,
                             0); 

            if (n <= 0) {
                printf("Сервер отключился\n");
                break;
            }

            received_bytes += (size_t)n;
            if (received_bytes == sizeof(incoming)) {
                handle_packet(&incoming);
                received_bytes = 0;
            }
        }

        // ввод с клавиатуры
        if (fds[0].revents & POLLIN) {
            if (fgets(input, sizeof(input), stdin) == NULL) {
                send_simple_packet(TYPE_LEAVE, NULL);
                break;
            }

            input[strcspn(input, "\n")] = '\0';

            if (strcmp(input, "/quit") == 0) {
                send_simple_packet(TYPE_LEAVE, NULL);
                break;
            }

            if (strncmp(input, "/file ", 6) == 0) {
                if (input[6] == '\0') {
                    printf("Укажи путь: /file путь_к_файлу\n");
                    continue;
                }
                if (send_file(input + 6) < 0)
                    printf("Не удалось отправить файл\n");
                continue;
            }

            if (input[0] != '\0') {
                if (send_simple_packet(TYPE_MESSAGE, input) < 0) {
                    printf("Не удалось отправить сообщение\n");
                    break;
                }
            }
        }

        if (fds[0].revents & (POLLHUP | POLLERR | POLLNVAL)) {
            send_simple_packet(TYPE_LEAVE, NULL);
            break;
        }
    }

    close(sockfd);
    return 0;
}
