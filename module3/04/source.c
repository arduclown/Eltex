#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"

// P: уменьшаем семафор на 1, если он уже равен нулю - процесс ждет
void lockMemory(int semid) {
    struct sembuf lock = {0, -1, 0};
    semop(semid, &lock, 1);
}

// V: увеличиваем семафор на 1, память снова свободна
void unlockMemory(int semid) {
    struct sembuf unlock = {0, 1, 0};
    semop(semid, &unlock, 1);
}

// Производитель периодически дописывает в память новый набор чисел, пока в сегменте есть свободное место
void runProducer(int semid, char* memory) {
    Head* head = (Head*)memory;
    Block* block;
    int* numbers;
    long offset = FIRST; // куда положим следующий набор
    long last = 0; // смещение последнего набора, 0 - наборов еще нет
    long need;
    int count, i, working;

    // чистим память
    lockMemory(semid);
    memset(memory, 0, SHM_SIZE); // память могла остаться от прошлого запуска
    unlockMemory(semid);

    while (1) {
        count = rand() % MAX_COUNT + 1;
        need = sizeof(Block) + count * sizeof(int); // необходимый размер блока

        lockMemory(semid);
        if (offset + need > SHM_SIZE) { // объем памяти исчерпан
            head->done = 1; //выходим
            unlockMemory(semid);
            break;
        }

        block = (Block*)(memory + offset); // памяти хватило, записали блок по смещению
        block->count = count;
        block->next = 0; // пока этот набор последний
        numbers = (int*)(block + 1); // числа лежат сразу за заголовком
        for (i = 0; i < count; i++) {
            numbers[i] = rand() % MAX_VALUE;
        }
        if (last != 0) { // предыдущий набор теперь ссылается на новый
            ((Block*)(memory + last))->next = offset;
        }
        printf("Производитель: набор из %d чисел по смещению %ld\n", count, offset);

        last = offset;
        offset += need;
        unlockMemory(semid);

        usleep(100);
    }

    printf("Производитель: память заполнена, ждем обработки наборов\n");

    // ждем, пока потребители не пометят все наборы как обработанные
    do {
        working = 0;
        lockMemory(semid);
        // проверка, есть ли хотя бы один блок с count > 0
        for (offset = FIRST; offset != 0; offset = ((Block*)(memory + offset))->next) {
            if (((Block*)(memory + offset))->count > 0) {
                working = 1;
                break;
            }
        }
        unlockMemory(semid);
        if (working) {
            usleep(10); // еще не все обработано
        }
    } while (working);

    printf("Производитель: все наборы обработаны\n");
}

// Потребитель находит необработанный набор, выводит его минимум и максимум и помечает набор обработанным
void runConsumer(int semid, char* memory) {
    Head* head = (Head*)memory;
    Block* block;
    int numbers[MAX_COUNT]; // копия набора, чтобы считать min и max без блокировки
    long offset;
    int count, done, min, max, i;

    while (1) {
        count = 0;

        lockMemory(semid);
        // идем по цепочке наборов и берем первый необработанный
        for (offset = FIRST; offset != 0; offset = block->next) {
            block = (Block*)(memory + offset);
            if (block->count > 0) {
                count = block->count;
                memcpy(numbers, block + 1, count * sizeof(int)); // копируем набор
                block->count = 0; // набор обработан
                break;
            }
        }
        done = head->done;
        unlockMemory(semid);

        if (count == 0) { // необработанных наборов нет
            if (done)
                break; // производитель закончил, работать больше не с чем
            sleep(1);
            continue;
        }

        min = max = numbers[0];
        for (i = 1; i < count; i++) {
            if (numbers[i] < min)
                min = numbers[i];
            if (numbers[i] > max)
                max = numbers[i];
        }
        printf("Потребитель %d: набор по смещению %ld из %d чисел, min = %d, max = %d\n",
               getpid(), offset, count, min, max);

        usleep(10000); // после одного набора потребитель засыпает
    }

    printf("Потребитель %d: все наборы обработаны, завершение работы\n", getpid());
}

