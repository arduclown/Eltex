#ifndef HEADER_H
#define HEADER_H

#include <fcntl.h>   // open() и флаги O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC
#include <unistd.h>  // read(), write(), close(), pipe(), fork()
#include <sys/types.h>

#define NAME_LEN 256 // максимальная длина имени файла
#define BLOCK_SIZE 512 // размер блока, которым передается содержимое файла
#define COPY_SUFFIX ".copy"// что дописываем к имени копии

// Первое сообщение о файле: как он называется и сколько байт будет передано.
// size = -1 означает "файлов больше нет, завершай работу".
typedef struct {
    char name[NAME_LEN];
    long size;
} Message;

// вспомогательные функции
ssize_t readAll(int fd, void* buffer, size_t count);
ssize_t writeAll(int fd, const void* buffer, size_t count);

void runParent(int writeFd, int readFd, int argc, char* argv[], int firstFile);

// работа потомка принимает файлы, пока родитель не скажет стоп
void runChild(int readFd, int writeFd);

#endif
