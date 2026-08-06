#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "header.h"

int main(int argc, char* argv[]) {
    int fd; // дескриптор объекта разделяемой памяти
    sem_t* semaphore;
    char* memory;
    int producer; // 1 - этот процесс производитель

    if (argc != 2 || (strcmp(argv[1], "p") != 0 && strcmp(argv[1], "c") != 0)) {
        fprintf(stderr, "Использование: %s p|c (производитель или потребитель)\n", argv[0]);
        return 1;
    }
    producer = (argv[1][0] == 'p');

    if (producer) {
        // объекты могли остаться от прошлого запуска, у семафора было бы старое значение счетчика
        shm_unlink(SHM_NAME);
        sem_unlink(SEM_NAME);

        fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
        if (fd != -1 && ftruncate(fd, SHM_SIZE) == -1) { // задаем размер памяти (ftruncate)
            perror("ftruncate");
            return 1;
        }
        // семафор сразу со значением 1, т.е доступ разрешен одному процессу
        semaphore = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    } else {
        // потребитель только подключается к уже созданным объектам
        fd = shm_open(SHM_NAME, O_RDWR, 0666);
        semaphore = sem_open(SEM_NAME, 0);
    }

    if (fd == -1 || semaphore == SEM_FAILED) {
        perror("shm_open/sem_open");
        if (!producer) {
            fprintf(stderr, "Сначала нужно запустить производителя\n");
        }
        return 1;
    }

    // подключаем сегмент к адресному пространству процесса
    memory = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    srand(time(NULL) ^ getpid());

    if (producer)
        runProducer(semaphore, memory);
    else
        runConsumer(semaphore, memory);

    munmap(memory, SHM_SIZE); // отключаем разделяемую память
    close(fd);
    sem_close(semaphore);

    if (producer) { // объекты удаляет тот, кто их создал
        shm_unlink(SHM_NAME);
        sem_unlink(SEM_NAME);
        printf("Разделяемая память и семафор удалены\n");
    }

    return 0;
}
