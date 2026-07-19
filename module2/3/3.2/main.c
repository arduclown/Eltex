#include "header.h"
#include <time.h>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Введите: %s <IP_шлюза> <маска_подсети> <N>\n", argv[0]);
        return 1;
    }

    int N;
    char symb;
    int result = sscanf(argv[3], "%d%c", &N, &symb);
    if (result != 1 || N <= 0) {
        fprintf(stderr, "N должно быть целым положительным числом. \n");
        return 1;
    }

    const char* netmaskStr = argv[2];
    const char* gatewayIPstr = argv[1];

    uint32_t getewayIP = parseIP(gatewayIPstr);
    uint32_t netmask = parseIP(netmaskStr);

    uint32_t networkAddress = getewayIP & netmask;

    int sameNetCnt = 0;

    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        uint32_t randomIP = generateRandomIp();
        uint32_t randomNetworkAddress = randomIP & netmask;

        if (randomNetworkAddress == networkAddress)
            sameNetCnt ++;
    }

    double sameNetPercent = (double)sameNetCnt / N * 100;
    double otherNetPercent = (double)(N - sameNetCnt) / N * 100;

    printf("Пакеты предназначены своей подсети: %d (%.2f%%)\n", sameNetCnt, sameNetPercent);
    printf("Пакеты для других сетей: %d (%.2f%%)\n", N - sameNetCnt, otherNetPercent);

    return 0;
}