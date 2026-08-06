#ifndef HEADER_H
#define HEADER_H

#include <fcntl.h> // O_CREAT, O_RDWR
#include <semaphore.h> // sem_open(), sem_wait(), sem_post(), sem_close(), sem_unlink()
#include <sys/mman.h> // shm_open(), mmap(), munmap(), shm_unlink()
#include <sys/types.h>
#include <unistd.h> // sleep(), getpid(), ftruncate()

#define SHM_NAME "/prodcons_shm" // имя объекта разделяемой памяти
#define SEM_NAME "/prodcons_sem" // имя семафора

#define SHM_SIZE 102400 // размер сегмента разделяемой памяти
#define MAX_COUNT 10 // максимальное количество чисел в одном наборе
#define MAX_VALUE 100 // числа генерируются в диапазоне [0, MAX_VALUE)

typedef struct {
    int count;
    long next;
} Block;

typedef struct {
    int done; // 1 - производитель больше не создает наборы
} Head;

#define FIRST ((long)sizeof(Head)) // смещение первого набора

void lockMemory(sem_t* semaphore); // занять семафор (операция P)
void unlockMemory(sem_t* semaphore); // освободить семафор (операция V)

void runProducer(sem_t* semaphore, char* memory); // заполнение памяти наборами чисел
void runConsumer(sem_t* semaphore, char* memory); // обработка наборов чисел

#endif
