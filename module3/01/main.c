#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h> // mkfifo()
#include <sys/wait.h> // wait()
#include "header.h"

int main(int argc, char* argv[]) {
    int option = 0;
    const char* fifoName = NULL; // имя канала из ключа -p
    int parentChild[2]; // прямой неименованный канал (родитель -> потомок)
    int childParent[2]; // обратный неименованный канал (потомок -> родитель)
    int readFd, writeFd; // каким дескриптором процесс читает и пишет

    while ((option = getopt(argc, argv, "p:h")) != -1) {
        switch (option) {
        case 'p': // ключ для именованного канала
            fifoName = optarg;
            if (strlen(fifoName) > NAME_LEN) {
                fprintf(stderr, "Имя канала превышает максимальную длину.\n");
                return 1;
            }
            break;
        case 'h':
            printf("Использование: %s [-p имя_канала] файл1 [файл2 ...]\n", argv[0]);
            return 0;
        default:
            fprintf(stderr, "Использование: %s [-p имя_канала] файл1 [файл2 ...]\n", argv[0]);
            return 1;
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "Не указано ни одного имени файла. Использование: %s [-p имя_канала] файл1 [файл2 ...]\n", argv[0]);
        return 1;
    }

    // создание каналов
    // обратный канал (потомок сообщает о готовности) всегда неименованный
    if (pipe(childParent) < 0) {
        perror("pipe");
        return 1;
    }

    if (fifoName != NULL) {
        // прямой канал - именованный: создается файл-FIFO в файловой системе
        if (mkfifo(fifoName, 0600) < 0 && errno != EEXIST) {
            fprintf(stderr, "mkfifo(%s): %s\n", fifoName, strerror(errno));
            return 1;
        }
        printf("Прямой канал: именованный (%s), обратный: неименованный\n", fifoName);
    } else {
        // прямой канал тоже неименованный
        if (pipe(parentChild) < 0) {
            perror("pipe");
            return 1;
        }
        printf("Прямой и обратный каналы: неименованные\n");
    }

    //сбрасываем буфер stdout, иначе потомок унаследует его копию
    // и уже напечатанные строки выведутся второй раз
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        if (fifoName) {
            unlink(fifoName);
        }
        return 1;
    }

    if (pid == 0) {
        //дочерний процесс
        close(childParent[0]); // из обратного канала он не читает
        writeFd = childParent[1];

        if (fifoName != NULL) {
            // открытие FIFO на чтение ждет, пока родитель откроет его на запись
            readFd = open(fifoName, O_RDONLY);
            if (readFd < 0) {
                fprintf(stderr, "Потомок: open(%s): %s\n", fifoName, strerror(errno));
                exit(1);
            }
        } else {
            close(parentChild[1]); // в прямой канал он не пишет
            readFd = parentChild[0];
        }

        runChild(readFd, writeFd);

        close(readFd);
        close(writeFd);
        exit(0);
    }

    // родительский процесс
    close(childParent[1]); // в обратный канал он не пишет
    readFd = childParent[0];

    if (fifoName != NULL) {
        writeFd = open(fifoName, O_WRONLY);
        if (writeFd < 0) {
            fprintf(stderr, "Родитель: open(%s): %s\n", fifoName, strerror(errno));
            unlink(fifoName);
            return 1;
        }
    } else {
        close(parentChild[0]); // из прямого канала он не читает
        writeFd = parentChild[1];
    }

    runParent(writeFd, readFd, argc, argv, optind);

    close(writeFd);
    close(readFd);

    wait(NULL);// ждем завершения потомка
    if (fifoName != NULL) {
        unlink(fifoName); // убираем файл канала
    }

    printf("Родитель: работа завершена\n");
    return 0;
}
