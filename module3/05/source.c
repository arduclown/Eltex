#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"

// P: уменьшаем семафор на 1, если он уже равен нулю - процесс ждет
void lockMemory(sem_t* semaphore) {
    sem_wait(semaphore);
}

// V: увеличиваем семафор на 1, память снова свободна
void unlockMemory(sem_t* semaphore) {
    sem_post(semaphore);
}

// Производитель периодически дописывает в память новый набор чисел, пока в сегменте есть свободное место
void runProducer(sem_t* semaphore, char* memory) {
    Head* head = (Head*)memory;
    Block* block;
    int* numbers;
    long offset = FIRST; // куда положим следующий набор
    long last = 0; // смещение последнего набора, 0 - наборов еще нет
    long need;
    int count, i, working;

    lockMemory(semaphore);
    memset(memory, 0, SHM_SIZE); // память могла остаться от прошлого запуска
    unlockMemory(semaphore);

    while (1) {
        count = rand() % MAX_COUNT + 1;
        need = sizeof(Block) + count * sizeof(int);

        lockMemory(semaphore);
        if (offset + need > SHM_SIZE) { // объем памяти исчерпан
            head->done = 1;
            unlockMemory(semaphore);
            break;
        }

        block = (Block*)(memory + offset);
        block->count = count;
        block->next = 0; // пока этот набор последний
        numbers = (int*)(block + 1); // числа лежат сразу за заголовком
        for (i = 0; i < count; i++)
            numbers[i] = rand() % MAX_VALUE;
        if (last != 0) // предыдущий набор теперь ссылается на новый
            ((Block*)(memory + last))->next = offset;
            
        printf("Производитель: набор из %d чисел по смещению %ld\n", count, offset);

        last = offset;
        offset += need;
        unlockMemory(semaphore);

        usleep(1000);
    }

    printf("Производитель: память заполнена, ждем обработки наборов\n");

    // ждем, пока потребители не пометят все наборы как обработанные
    do {
        working = 0;
        lockMemory(semaphore);
        for (offset = FIRST; offset != 0; offset = ((Block*)(memory + offset))->next) {
            if (((Block*)(memory + offset))->count > 0) {
                working = 1;
                break;
            }
        }
        unlockMemory(semaphore);
        if (working) {
            sleep(1); // еще не все обработано
        }
    } while (working);

    printf("Производитель: все наборы обработаны\n");
}

// Потребитель находит необработанный набор, выводит его минимум и максимум и помечает набор обработанным
void runConsumer(sem_t* semaphore, char* memory) {
    Head* head = (Head*)memory;
    Block* block;
    int numbers[MAX_COUNT]; // копия набора, чтобы считать min и max без блокировки
    long offset;
    int count, done, min, max, i;

    while (1) {
        count = 0;

        lockMemory(semaphore);
        // идем по цепочке наборов и берем первый необработанный
        for (offset = FIRST; offset != 0; offset = block->next) {
            block = (Block*)(memory + offset);
            if (block->count > 0) {
                count = block->count;
                memcpy(numbers, block + 1, count * sizeof(int));
                block->count = 0; // набор обработан
                break;
            }
        }
        done = head->done;
        unlockMemory(semaphore);

        if (count == 0) { // необработанных наборов нет
            if (done) {
                break; // производитель закончил, работать больше не с чем
            }
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

        usleep(10000); // после одного набора потребитель "засыпает"
    }

    printf("Потребитель %d: все наборы обработаны, завершение работы\n", getpid());
}
