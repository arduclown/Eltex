#include "header.h"
#include <stdio.h>

int main() {
    Operation operations[] = {
        {add, "Сложение"},
        {subtract, "Вычитание"},
        {multiply, "Умножение"},
        {division, "Деление"}
    };
    int numOperations = sizeof(operations) / sizeof(operations[0]);

    int choice;
    double numbers[2];
    double result = 0;

    while(1) {
        printf("Меню:\n");
        for (int i = 0; i < numOperations; i++) {
            printf("%d. %s\n", i + 1, operations[i].name);
        }
        printf("%d. Выход\n", numOperations + 1);
        printf("Выберите действие: ");
        scanf("%d", &choice);

        if (choice == 5) {
            printf("Выход из калькулятора...\n");
            return 0;
        }

        if (choice < 1 || choice > 5) {
            printf("Введите цифру от 1 до 5!\n");
            continue;
        }
            
        printf("Введите два числа: ");
        for (int i = 0; i < 2; i++)
            scanf("%lf", &numbers[i]);
        
        action func = operations[choice - 1].func;
        func(&result, numbers[0], numbers[1]);
        printf("Ответ: %lf\n", result);
    }
    return 0;
}