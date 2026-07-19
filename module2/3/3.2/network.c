#include "header.h"

uint32_t parseIP(const char* ip) {
    uint8_t a, b, c, d; // 0-255
    sscanf(ip, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d); // распарсили по abcd unsigned char
    return (a << 24) | (b << 16) | (c << 8) | d;
}

uint32_t generateRandomIp() {
    uint32_t ip = 0;
    for (int i = 3; i >= 0; i--)
        ip |= (uint32_t)(rand() % 256) << (i * 8);
    return ip;
}