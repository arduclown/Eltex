#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "header.h"

int main(int argc, char* argv[]) {
    key_t key;
    int shmid, semid;
    char* memory;
    int producer; // 1 - этот процесс производитель

    if (argc != 2 || (strcmp(argv[1], "p") != 0 && strcmp(argv[1], "c") != 0)) {
        fprintf(stderr, "Использование: %s p|c (производитель или потребитель)\n", argv[0]);
        return 1;
    }
    producer = (argv[1][0] == 'p');

    key = ftok(".", 'S');
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    if (producer) {
        // память и семафор создает производитель
        shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
        semid = semget(key, 1, 0666 | IPC_CREAT);
    } else {
        // потребитель только подключается к уже созданным
        shmid = shmget(key, SHM_SIZE, 0666);
        semid = semget(key, 1, 0666);
    }

    if (shmid == -1 || semid == -1) { 
        perror("shmget/semget");
        if (!producer) { // производителя еще нет
            fprintf(stderr, "Сначала нужно запустить производителя\n");
        }
        return 1;
    }

    if (producer) {
        semctl(semid, 0, SETVAL, 1); // семафор свободен: доступ разрешен одному процессу
    }

    memory = shmat(shmid, NULL, 0); // подключаем сегмент к адресному пространству
    if (memory == (char*)-1) {
        perror("shmat");
        return 1;
    }

    srand(time(NULL) ^ getpid()); // побитовое смещение, чтобы везде были разные последовательностИ. а не одинаковые

    if (producer)
        runProducer(semid, memory);
    else
        runConsumer(semid, memory);

    shmdt(memory); // отключаем разделяемую память

    if (producer) { // ресурсы удаляет тот, кто их создал
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        printf("Разделяемая память и семафор удалены\n");
    }

    return 0;
}
