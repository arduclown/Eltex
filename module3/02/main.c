#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "header.h"

int main(int argc, char* argv[]) {
    int option = 0;
    char role = 0; // 'b' - брокер, 'p' - издатель, 's' - подписчик
    const char* topic = NULL; // тема издателя из ключа -p
    int qid; // идентификатор очереди сообщений
    key_t key; // ключ очереди, общий для всех ролей

    while ((option = getopt(argc, argv, "bp:sh")) != -1) {
        switch (option) {
        case 'b': // брокер
            role = 'b';
            break;
        case 'p': // издатель, тема указана после ключа
            role = 'p';
            topic = optarg;
            if (strlen(topic) >= TOPIC_LEN) {
                fprintf(stderr, "Тема превышает максимальную длину.\n");
                return 1;
            }
            break;
        case 's': // подписчик, темы идут дальше в аргументах
            role = 's';
            break;
        case 'h':
            printf("Использование: %s -b | -p тема | -s тема1 [тема2 ...]\n", argv[0]);
            return 0;
        default:
            fprintf(stderr, "Использование: %s -b | -p тема | -s тема1 [тема2 ...]\n", argv[0]);
            return 1;
        }
    }

    if (role == 0) {
        fprintf(stderr, "Не указана роль. Использование: %s -b | -p тема | -s тема1 [тема2 ...]\n", argv[0]);
        return 1;
    }
    if (role == 's' && optind >= argc) {
        fprintf(stderr, "Подписчику нужно указать хотя бы одну тему.\n");
        return 1;
    }

    setupSignals();

    key = getQueueKey(); // ключ вычисляется по пути KEY_PATH и номеру проекта
    if (key == (key_t)-1) {
        return 1;
    }

    if (role == 'b') {
        // IPC_EXCL: если очередь уже есть, значит брокер уже запущен
        qid = msgget(key, IPC_CREAT | IPC_EXCL | 0666); // создаем очередь
        if (qid < 0) {
            if (errno == EEXIST) {
                fprintf(stderr, "Брокер уже запущен (очередь сообщений существует).\n");
            } else {
                perror("msgget");
            }
            return 1;
        }
        runBroker(qid);
        return 0;
    }

    // издателю и подписчику очередь должен был создать брокер
    qid = msgget(key, 0); // ищется уже созданная очередь с тем же ключом
    if (qid < 0) {
        fprintf(stderr, "Очередь сообщений недоступна: брокер не запущен.\n");
        return 1;
    }

    if (role == 'p') {
        runPublisher(qid, topic);
    } else {
        runSubscriber(qid, argc, argv, optind);
    }

    return 0;
}
