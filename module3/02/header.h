#ifndef HEADER_H
#define HEADER_H

#include <sys/types.h>
#include <sys/ipc.h> // ключи и флаги IPC_CREAT, IPC_EXCL, IPC_RMID, ftok()
#include <sys/msg.h> // msgget(), msgsnd(), msgrcv(), msgctl()
#include <signal.h> // kill(), sigaction(), SIGINT
#include <unistd.h> // getpid(), sleep()

#define KEY_PATH "/tmp" // существующий путь, по которому ftok строит ключ
#define KEY_PROJ_ID 'B' // номер проекта для ftok (одинаков у всех ролей)
#define BROKER_TYPE 1 // приоритет сообщений, которые читает брокер
#define MSG_LEN 256 // максимальная длина текста сообщения
#define TOPIC_LEN 32 // максимальная длина темы
#define MAX_SUBS 64 // сколько подписок помещается в списке брокера
#define MAX_PUBS 64 // сколько издателей помещается в списке брокера
#define DRAIN_TIMEOUT 5 // сколько секунд брокер ждет освобождения очереди

// сообщение очереди System V: mtype - приоритет, mtext - текст
typedef struct {
    long mtype;
    char mtext[MSG_LEN];
} Message;

//одна подписка: какой процесс на какую тему подписан
typedef struct {
    pid_t pid;
    char topic[TOPIC_LEN];
} Subscription;

// флаг работы, сбрасывается обработчиком SIGINT
extern volatile sig_atomic_t running;

// ставим обработчик SIGINT (без SA_RESTART, чтобы прерывались msgrcv и fgets)
void setupSignals(void);

// ключ очереди, полученный через ftok(); (key_t)-1 при ошибке
key_t getQueueKey(void);

void runBroker(int qid);
void runPublisher(int qid, const char* topic);
void runSubscriber(int qid, int argc, char* argv[], int firstTopic);

#endif
