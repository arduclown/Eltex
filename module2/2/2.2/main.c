#include "header.h"
#include <stdio.h>

int main() {
    int choice;
    double numbers[2];
    double result = 0;

    while(1) {
        printf("\nМеню:\n"
			"1. Сложение\n"
			"2. Вычитание\n"
			"3. Умножение\n"
			"4. Деление\n"
			"5. Выход\n"
			"Выберите действие: ");
        scanf("%d", &choice);

        if (choice == 5) {
            printf("Выход из калькулятора...\n");
            return 0;
        }
        printf("Введите два числа: ");
        for (int i = 0; i < 2; i++)
            scanf("%lf", &numbers[i]);
        
        switch(choice) {
            case 1: {
                add(&result, numbers[0], numbers[1]);
                printf("%lf", result);
                break;
            }
            case 2: {
                subtract(&result, numbers[0], numbers[1]);
                printf("%lf", result);
                break;
            }
            case 3: {
                multiply(&result, numbers[0], numbers[1]);
                printf("%lf", result);
                break;
            }
            case 4: {
                division(&result, numbers[0], numbers[1]);
                printf("%lf", result);
                break;
            }
            default: {
                printf("Введите цифру от 1 до 5!\n");
            }
        }
    }
    return 0;
}