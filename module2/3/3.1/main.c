#include <string.h>
#include "header.h"
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Введите: %s <права или команда> [имя файла]\n", argv[0]);
        printf("Примеры: \n"
            "./main rwxr-xr-x\n"
            "./main 755\n"
            "./main test.txt\n"
            "./main chmod u+x tset.txt\n"
            "./main chmod 657 test.txt\n");
            return 1;
    }

    char bits[10];

    if (argc == 4 && strcmp(argv[1], "chmod") == 0) {
        if (getFilePermission(argv[3], bits) == -1) {
            fprintf(stderr, "Error: Файл %s не найден.\n", argv[3]);
            return 1;
        }
        printf("Права доступа к файлу %s:\n", argv[3]);
        printPermissions(bits);

        if (isPermissionsNumeric(argv[2])) {
            numbersToBits(argv[2], bits);
            printf("\nУстановки прав доступа к %s на %s:\n", argv[3], argv[2]);
            printPermissions(bits);
        } else {
            applyModification(argv[2], bits);
            printf("\'%s' применено к файлу %s:\n", argv[2], argv[3]);
            printPermissions(bits);
        }
    } 
    else if (argc == 2) {
        if (strlen(argv[1]) == 3 && isPermissionsNumeric(argv[1])) {
            numbersToBits(argv[1], bits);
            printPermissions(bits);
        } else if (strlen(argv[1]) == 9) {
            letterToBits(argv[1], bits);
            printPermissions(bits);
        } else {
            
            if (getFilePermission(argv[1], bits) == -1) {
                fprintf(stderr, "Error: Файл %s не найден.\n", argv[1]);
                return 1;
            }
            printf("Права доступа к файлу %s:\n", argv[1]);
            printPermissions(bits);
        }
    }
    else {
        printf("Введена некорректная команада или неверное количество аргументов.\n");
        printf("Введите: %s <права или команда> [имя файла]\n", argv[0]);
        printf("Примеры: \n"
            "./3_1 rwxr-xr-x\n"
            "./3_1 755\n"
            "./3_1 test.txt\n"
            "./3_1 chmod u+x tset.txt\n"
            "./3_1 chmod 657 test.txt\n");
    }
}