#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "header.h"

// read() может вернуть меньше байт, чем просили,
// поэтому читаем в цикле, пока не наберем count байт или не кончатся данные
ssize_t readAll(int fd, void* buffer, size_t count) {
    char* position = buffer;
    size_t done = 0;
    while (done < count) {
        ssize_t n = read(fd, position + done, count - done);
        if (n <= 0) {
            break; // 0 - канал закрыт с другой стороны, <0 - ошибка
        }
        done += n;
    }
    return done;
}

// то же самое для записи: write() тоже может записать не все за один раз
ssize_t writeAll(int fd, const void* buffer, size_t count) {
    const char* position = buffer;
    size_t done = 0;
    while (done < count) {
        ssize_t n = write(fd, position + done, count - done);
        if (n <= 0) {
            break;
        }
        done += n;
    }
    return done;
}

// Родитель
// writeFd - прямой канал (родитель -> потомок), туда уходят сообщения и данные
// readFd  - обратный канал (потомок -> родитель), оттуда приходит сигнал готовности
void runParent(int writeFd, int readFd, int argc, char* argv[], int firstFile) {
    char buffer[BLOCK_SIZE];
    Message message;
    char ready;

    for (int i = firstFile; i < argc; i++) {
        int fileFd = open(argv[i], O_RDONLY);
        if (fileFd < 0) {
            fprintf(stderr, "Родитель: не могу открыть %s: %s\n", argv[i], strerror(errno));
            continue; // файла нет - просто переходим к следующему
        }

        // узнаем размер файла: переходим в конец и смотрим позицию, потом возвращаемся в начало
        long size = lseek(fileFd, 0, SEEK_END);
        lseek(fileFd, 0, SEEK_SET);

        // ждем от потомка сообщение R 
        if (readAll(readFd, &ready, 1) != 1) {
            fprintf(stderr, "Родитель: потомок не отвечает\n");
            close(fileFd);
            return;
        }

        // первое сообщение о файле: имя и количество байт
        snprintf(message.name, NAME_LEN, "%s", argv[i]);
        message.size = size;
        writeAll(writeFd, &message, sizeof(message));

        // содержимое передаем блоками
        ssize_t n;
        while ((n = read(fileFd, buffer, BLOCK_SIZE)) > 0) {
            writeAll(writeFd, buffer, n);
        }
        close(fileFd);

        printf("Родитель: файл %s передан (%ld байт)\n", argv[i], size);
    }

    // все файлы переданы - сообщаем потомку о завершении (size = -1)
    if (readAll(readFd, &ready, 1) == 1) {
        message.name[0] = '\0';
        message.size = -1;
        writeAll(writeFd, &message, sizeof(message));
    }
}

// Потомок
// readFd  - прямой канал, оттуда читаем сообщения и данные
// writeFd - обратный канал, туда пишем сигнал готовности
void runChild(int readFd, int writeFd) {
    char buffer[BLOCK_SIZE];
    char copyName[NAME_LEN + sizeof(COPY_SUFFIX)];
    Message message;

    while (1) {
        // сообщаем родителю, что готовы принимать очередной файл
        char ready = 'R';
        writeAll(writeFd, &ready, 1);

        // ждем сообщение с именем и размером
        if (readAll(readFd, &message, sizeof(message)) != sizeof(message)) {
            break; // родитель закрыл канал
        }
        if (message.size < 0) {
            break; // родитель сказал: файлов больше нет
        }

        snprintf(copyName, sizeof(copyName), "%s%s", message.name, COPY_SUFFIX);
        int copyFd = open(copyName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (copyFd < 0) {
            fprintf(stderr, "Потомок: не могу создать %s: %s\n", copyName, strerror(errno));
        }

        // принимаем ровно message.size байт блоками
        long left = message.size;
        while (left > 0) {
            size_t want = (left < BLOCK_SIZE) ? (size_t)left : BLOCK_SIZE;
            ssize_t n = read(readFd, buffer, want);
            if (n <= 0) {
                break;
            }
            if (copyFd >= 0) {
                writeAll(copyFd, buffer, n);
            }
            left -= n;
        }

        if (copyFd >= 0) {
            close(copyFd);
            printf("Потомок: создан файл %s (%ld байт)\n", copyName, message.size);
        }
    }

    printf("Потомок: работа завершена\n");
}
