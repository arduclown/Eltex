#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
    // поэтому mq_receive и fgets прервутся и процесс увидит running == 0
    sigaction(SIGINT, &action, NULL);
}

// Прием работает в дочернем процессе: читает свою очередь и выводит сообщения
void runReceiver(mqd_t in) {
    char buffer[MSG_LEN + 1]; // +1 на завершающий ноль
    unsigned int priority;
    ssize_t length;

    while (running) {
        length = mq_receive(in, buffer, MSG_LEN, &priority);
        if (length < 0) {
            if (errno == EINTR) {
                break; // пришел SIGINT
            }
            fprintf(stderr, "Прием: очередь недоступна (%s)\n", strerror(errno));
            break;
        }
        buffer[length] = '\0';

        if (priority == QUIT_PRIO) { // условленный приоритет конца обмена
            printf("\nСобеседник завершил работу\n");
            break;
        }
        printf("Собеседник: %s\n", buffer);
    }

    // будим родителя, т.к. он ждет ввода с клавиатуры и о завершении обмена не знает
    kill(getppid(), SIGINT);
}

// Отправка работает в родительском процессе:
void runSender(mqd_t out) {
    char line[MSG_LEN];

    printf("Вводите сообщения, Ctrl+D или Ctrl+C для завершения\n");

    while (running && fgets(line, sizeof(line), stdin) != NULL) {
        line[strcspn(line, "\n")] = '\0'; // убираем перевод строки
        if (line[0] == '\0') {
            continue;
        }
        if (mq_send(out, line, strlen(line), CHAT_PRIO) < 0) {
            fprintf(stderr, "Отправка: очередь недоступна (%s)\n", strerror(errno));
            break;
        }
    }

    // уведомляем собеседника об окончании работы сообщением с известным приоритетом
    mq_send(out, "bye", 3, QUIT_PRIO);
    printf("Завершение работы\n");
}
