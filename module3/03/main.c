#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "header.h"

int main(int argc, char* argv[]) {
    char name1[NAME_LEN], name2[NAME_LEN];
    mqd_t q1, q2;
    mqd_t in, out; // очередь для приема и очередь для отправки
    int creator = 1; // 1 - очереди создал этот процесс
    pid_t child;

    if (argc != 2 || strlen(argv[1]) + 3 >= NAME_LEN) {
        fprintf(stderr, "Использование: %s имя_очереди\n", argv[0]);
        return 1;
    }

    // имена POSIX-очередей должны начинаться со слэша
    snprintf(name1, NAME_LEN, "/%s_1", argv[1]);
    snprintf(name2, NAME_LEN, "/%s_2", argv[1]);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MSG_LEN;
    attr.mq_curmsgs = 0;

    // O_EXCL: если первая очередь уже есть, значит собеседник запустился раньше
    q1 = mq_open(name1, O_RDWR | O_CREAT | O_EXCL, 0666, &attr); // вернули дескриптор очереди. Если успех, то эта очередь для приема
    if (q1 == (mqd_t)-1) { // если ошибка возникла, то верется такое значение
        if (errno != EEXIST) {
            perror("mq_open");
            return 1;
        }
        creator = 0;
        q1 = mq_open(name1, O_RDWR); // открываем обе очереди, т.к. собеседник уже создал очереди
        q2 = mq_open(name2, O_RDWR);
    } else {
        q2 = mq_open(name2, O_RDWR | O_CREAT, 0666, &attr); // если собеседника нет, то открываем и вторую очередь
    }

    if (q1 == (mqd_t)-1 || q2 == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    // создатель принимает через первую очередь и отправляет через вторую,
    // второй участник - наоборот
    if (creator) {
        in = q1;
        out = q2;
        printf("Очереди %s и %s созданы, ждем собеседника\n", name1, name2);
    } else {
        in = q2;
        out = q1;
        printf("Подключились к очередям %s и %s\n", name1, name2);
    }

    setupSignals();

    // разделяем работу: чтение очереди и чтение клавиатуры блокирующие поэтому прием выносим в отдельный процесс
    child = fork();
    if (child < 0) {
        perror("fork");
        return 1;
    }
    if (child == 0) { // только на приём
        runReceiver(in);
        _exit(0);
    }

    runSender(out);

    kill(child, SIGINT); // прием больше не нужен
    waitpid(child, NULL, 0);

    mq_close(q1);
    mq_close(q2);
    if (creator) { // очереди удаляет тот, кто их создал
        mq_unlink(name1);
        mq_unlink(name2);
        printf("Очереди удалены\n");
    }

    return 0;
}
