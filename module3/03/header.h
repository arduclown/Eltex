#ifndef HEADER_H
#define HEADER_H

#include <mqueue.h> // mq_open(), mq_send(), mq_receive(), mq_close(), mq_unlink()
#include <signal.h> // kill(), sigaction(), SIGINT
#include <unistd.h> // getpid(), fork()
#include <sys/types.h>

#define MSG_LEN 256 // максимальная длина сообщения
#define NAME_LEN 64 // максимальная длина имени очереди
#define MAX_MESSAGES 10 // сколько сообщений помещается в очередь
#define CHAT_PRIO 1 // приоритет обычного сообщения
#define QUIT_PRIO 5 // приоритет сообщения "я завершаю работу"

// флаг работы, сбрасывается обработчиком SIGINT
extern volatile sig_atomic_t running;

// ставим обработчик SIGINT
void setupSignals(void);

void runReceiver(mqd_t in); // цикл приема сообщений (дочерний процесс)
void runSender(mqd_t out); // цикл отправки сообщений (родительский процесс)

#endif
