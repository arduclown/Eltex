#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "header.h"

volatile sig_atomic_t running = 1;

static void onSignal(int signalNumber) {
    (void)signalNumber;
    running = 0;
}

void setupSignals(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = onSignal;
    // sa_flags = 0: системные вызовы не перезапускаются,
    // поэтому msgrcv и fgets прервутся и процесс увидит running == 0
    sigaction(SIGINT, &action, NULL);
}

// ftok строит ключ по номеру устройства и inode файла KEY_PATH и номеру проекта,
// поэтому все роли получают один и тот же ключ независимо от рабочего каталога
key_t getQueueKey(void) {
    key_t key = ftok(KEY_PATH, KEY_PROJ_ID);
    if (key == (key_t)-1) {
        perror("ftok");
    }
    return key;
}

// Брокер
// читает сообщения с приоритетом 1 и рассылает их подписчикам
void runBroker(int qid) {
    Message message;
    Subscription subs[MAX_SUBS]; // список подписок: pid + тема
    int subCount = 0;
    pid_t pubs[MAX_PUBS]; // список издателей, чтобы уведомить о завершении
    int pubCount = 0;

    printf("Брокер запущен (pid %d), очередь создана. Ctrl+C для завершения\n", getpid());

    while (running) {
        if (msgrcv(qid, &message, MSG_LEN, BROKER_TYPE, 0) < 0) {
            if (errno == EINTR) {
                break; // пришел SIGINT
            }
            perror("msgrcv");
            break;
        }

        //текст вида "команда,pid,тема[,содержимое]"
        char* command = strtok(message.mtext, ",");
        char* pidText = strtok(NULL, ",");
        char* topic = strtok(NULL, ",");
        char* payload = strtok(NULL, ""); // весь остаток строки (сообщение)
        if (command == NULL || pidText == NULL || topic == NULL) {
            continue;
        }
        pid_t sender = (pid_t)atoi(pidText);

        if (strcmp(command, "subscribe") == 0) {
            if (subCount < MAX_SUBS) {
                subs[subCount].pid = sender;
                snprintf(subs[subCount].topic, TOPIC_LEN, "%s", topic);
                subCount++;
                printf("Брокер: подписчик %d подписан на тему \"%s\"\n", sender, topic);
            } else {
                fprintf(stderr, "Брокер: список подписок переполнен\n");
            }
        } else if (strcmp(command, "unsubscribe") == 0) {
            for (int i = 0; i < subCount; i++) {
                if (subs[i].pid == sender && strcmp(subs[i].topic, topic) == 0) {
                    subs[i] = subs[subCount - 1]; // на место удаленной ставим последнюю
                    subCount--;
                    printf("Брокер: подписчик %d отписан от темы \"%s\"\n", sender, topic);
                    break;
                }
            }
        } else if (strcmp(command, "send") == 0) {
            // запоминаем издателя, если он встретился впервые
            int known = 0;
            for (int i = 0; i < pubCount; i++) {
                if (pubs[i] == sender) {
                    known = 1;
                    break;
                }
            }
            if (!known && pubCount < MAX_PUBS) {
                pubs[pubCount++] = sender;
            }

            // рассылаем всем, кто подписан на эту тему
            Message out;
            int sent = 0;
            for (int i = 0; i < subCount; i++) {
                if (strcmp(subs[i].topic, topic) != 0) {
                    continue;
                }
                out.mtype = subs[i].pid; // приоритет = pid получателя
                snprintf(out.mtext, MSG_LEN, "тема \"%s\", издатель %d: %s",
                         topic, sender, payload ? payload : "");
                if (msgsnd(qid, &out, strlen(out.mtext) + 1, 0) < 0) {
                    perror("msgsnd");
                } else {
                    sent++;
                }
            }
            printf("Брокер: сообщение по теме \"%s\" от %d разослано %d подписчикам\n",
                   topic, sender, sent);
        }
    }

    // завершение: новые сообщения больше не рассылаем, уведомляем всех сигналом
    printf("\nБрокер: завершение работы, рассылаю SIGINT\n");
    for (int i = 0; i < subCount; i++) {
        kill(subs[i].pid, SIGINT);
    }
    for (int i = 0; i < pubCount; i++) {
        kill(pubs[i], SIGINT);
    }

    // ждем, пока очередь освободится
    time_t start = time(NULL);
    while (time(NULL) - start < DRAIN_TIMEOUT) {
        if (msgrcv(qid, &message, MSG_LEN, 0, IPC_NOWAIT) < 0) {
            if (errno == ENOMSG) {
                sleep(1); // даем процессам время дописать последние сообщения
                if (msgrcv(qid, &message, MSG_LEN, 0, IPC_NOWAIT) < 0) {
                    break; // очередь пуста
                }
            } else {
                break;
            }
        }
    }


    msgctl(qid, IPC_RMID, NULL); // удаляем очередь
    printf("Брокер: очередь удалена, работа завершена\n");
}

// Издатель
// строки читаются с клавиатуры и уходят в очередь с приоритетом 1
void runPublisher(int qid, const char* topic) {
    Message message;
    char line[MSG_LEN];

    printf("Издатель (pid %d), тема \"%s\". Вводите текст сообщений, Ctrl+D для выхода\n",
           getpid(), topic);

    while (running && fgets(line, sizeof(line), stdin) != NULL) {
        line[strcspn(line, "\n")] = '\0'; // убираем перевод строки
        if (line[0] == '\0') {
            continue;
        }

        message.mtype = BROKER_TYPE;
        // формируем сообщение
        snprintf(message.mtext, MSG_LEN, "send,%d,%s,%s", getpid(), topic, line);
        if (msgsnd(qid, &message, strlen(message.mtext) + 1, 0) < 0) {
            fprintf(stderr, "Издатель: очередь недоступна (%s)\n", strerror(errno));
            break;
        }
        printf("Издатель: сообщение отправлено\n");
    }

    printf("Издатель %d: работа завершена\n", getpid());
}

// Подписчик
// подписывается на темы из аргументов и читает сообщения с приоритетом = своему pid
void runSubscriber(int qid, int argc, char* argv[], int firstTopic) {
    Message message;
    pid_t self = getpid();

    // сообщаем брокеру о подписке на каждую тему
    for (int i = firstTopic; i < argc; i++) {
        message.mtype = BROKER_TYPE;
        snprintf(message.mtext, MSG_LEN, "subscribe,%d,%s", self, argv[i]);
        if (msgsnd(qid, &message, strlen(message.mtext) + 1, 0) < 0) {
            fprintf(stderr, "Подписчик: очередь недоступна (%s)\n", strerror(errno));
            return;
        }
        printf("Подписчик %d: подписка на тему \"%s\"\n", self, argv[i]);
    }

    while (running) {
        // читаем только свои сообщения: приоритет равен pid
        if (msgrcv(qid, &message, MSG_LEN, self, 0) < 0) {
            if (errno == EINTR) {
                break; // пришел SIGINT от брокера или с клавиатуры
            }
            fprintf(stderr, "Подписчик: очередь недоступна (%s)\n", strerror(errno));
            return; // очередь удалена, отписываться уже некуда
        }
        printf("Подписчик %d получил: %s\n", self, message.mtext);
    }

    // уведомляем брокера об отписке (если очередь еще жива)
    for (int i = firstTopic; i < argc; i++) {
        message.mtype = BROKER_TYPE;
        snprintf(message.mtext, MSG_LEN, "unsubscribe,%d,%s", self, argv[i]);
        msgsnd(qid, &message, strlen(message.mtext) + 1, IPC_NOWAIT);
    }

    printf("Подписчик %d: работа завершена\n", self);
}
