#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>

int main() {
    Operation operations[MAX_OPERATIONS];
    int numOperations = 0;

    DIR* dir;
    struct dirent* entry;
    char libDir[] = "./libs"; 

    if ((dir = opendir(libDir)) == NULL) {
        fprintf(stderr, "Error: Не удалось открыть директорию %s.\n", libDir);
        return 1;
    }

    while ((entry = readdir(dir)) != NULL && numOperations < MAX_OPERATIONS) {
        if(strstr(entry->d_name, ".so") != NULL) {
            char lib_path[256];
            snprintf(lib_path, sizeof(lib_path), "%s/%s", libDir, entry->d_name);

            void* handle = dlopen(lib_path, RTLD_LAZY);
            if (!handle) {
                fprintf(stderr, "Error: Библиотека %s не загрузилась: %s.\n", entry->d_name, dlerror());
                continue;
            }

            //Извлекаем название операции
            char* funcName = strdup(entry->d_name+3);// Пропускаем "lib"
            funcName[strlen(funcName)-3] = '\0'; // Убираем ".so"
            action func = (action)dlsym(handle, funcName);
            char* error = dlerror();
            if (error){
                fprintf(stderr, "Error: Функция %s не извлечен: %s\n", funcName, error);
                free(funcName);
                dlclose(handle);
                continue;
            }

            char* op_name = NULL;
            if (strcmp(funcName, "add") == 0) op_name = "Сложение";
            else if (strcmp(funcName, "subtract") == 0) op_name = "Вычитание";
            else if (strcmp(funcName, "multiply") == 0) op_name = "Умножение";
            else if (strcmp(funcName, "division") == 0) op_name = "Деление";
            else {
                fprintf(stderr, "Неизвестная операция %s\n", funcName);
                free(funcName);
                dlclose(handle);
                continue;
            }

            // Добавляем операцию в массив
            operations[numOperations].name = op_name;
            operations[numOperations].func = func;
            operations[numOperations].handle = handle;
            numOperations++;
            free(funcName);
        }
    }

    closedir(dir);
    if (numOperations == 0) {
        fprintf(stderr, "Не удалось загрузить ни одну операцию\n");
        return 1;
    }

    for (int i = 0; i < numOperations; i++) {
        printf("%d. %s\n", i+1, operations[i].name);
    }

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

        if (choice == numOperations + 1) {
            printf("Выход из калькулятора...\n");
            return 0;
        }

        if (choice < 1 || choice > numOperations + 1) {
            printf("Введите цифру от 1 до %d!\n", numOperations + 1);
            continue;
        }
            
        printf("Введите два числа: ");
        for (int i = 0; i < 2; i++)
            scanf("%lf", &numbers[i]);
        
        action func = operations[choice - 1].func;
        func(&result, numbers[0], numbers[1]);
        printf("Ответ: %lf\n", result);
    }
    // Закрываем все библиотеки
    for (int i = 0; i < numOperations; i++) {
        dlclose(operations[i].handle);
    }
    return 0;
}