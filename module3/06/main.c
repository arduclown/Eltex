#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "header.h"

int main(int argc, char* argv[]) {
    pthread_t receiver;
    char text[TEXT_SIZE];
    size_t length;

    if (argc != 2) {
        fprintf(stderr, "Использование: %s имя\n", argv[0]);
        return 1;
    }

    myId = getpid(); // по нему отличаем свои пакеты от чужих
    strncpy(myName, argv[1], NAME_SIZE - 1);

    if (createSocket() == -1)
        return 1;

    // прием сообщений идет в отдельном потоке, основной поток читает ввод пользователя
    if (pthread_create(&receiver, NULL, runReceiver, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }

    sendPacket(JOIN, NULL); // сообщаем всем, что мы появились в сети
    printf("%s подключен к чату, порт %d (для выхода Ctrl+D)\n", myName, PORT);

    while (fgets(text, sizeof(text), stdin) != NULL) {
        length = strlen(text);
        if (length > 0 && text[length - 1] == '\n') // fgets оставляет перевод строки
            text[length - 1] = '\0';
        if (text[0] != '\0')
            sendPacket(MESSAGE, text);
    }

    sendPacket(LEAVE, NULL); // сообщаем всем, что мы уходим
    printf("%s отключен от чата\n", myName);

    close(socketFd);
    return 0;
}
