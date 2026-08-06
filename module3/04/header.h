#ifndef HEADER_H
#define HEADER_H

#include <sys/ipc.h> // ftok()
#include <sys/sem.h> // semget(), semop(), semctl()
#include <sys/shm.h> // shmget(), shmat(), shmdt(), shmctl()
#include <sys/types.h>
#include <unistd.h> // sleep(), getpid()

#define SHM_SIZE 102400 // размер сегмента разделяемой памяти
#define MAX_COUNT 10 // максимальное количество чисел в одном наборе
#define MAX_VALUE 100 // числа генерируются в диапазоне [0, MAX_VALUE)

// заголовок набора: перед самими числами лежат
typedef struct {
    int count; // count = 0 - набор уже обработан
    long next; // смещение слде. набора. если = 0, то последний
} Block;

// служебный заголовок в начале сегмента, первый набор лежит сразу за ним
typedef struct {
    int done; // если 1 ->производитель больше не создает наборы
} Head;

#define FIRST ((long)sizeof(Head)) // смещение первого набора

void lockMemory(int semid); // занять семафор (операция P)
void unlockMemory(int semid); // освободить семафор (операция V)

void runProducer(int semid, char* memory); // заполнение памяти наборами чисел
void runConsumer(int semid, char* memory); // обработка наборов чисел

#endif
