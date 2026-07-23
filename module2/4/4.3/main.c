#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 256

static char* inputString(const char* prompt) {
    printf("%s", prompt);
    char buffer[MAX_INPUT];
    if (fgets(buffer, MAX_INPUT, stdin) == NULL) return NULL;
    buffer[strcspn(buffer, "\n")] = 0;
    if (strlen(buffer) == 0) return NULL;
    char* str = malloc(strlen(buffer) + 1);
    if (str) strcpy(str, buffer);
    return str;
}

static void freeInput(char* str) {
    if (str) free(str);
}

int main() {
    AVLNode* root = NULL;
    const char* filename = "contacts.txt";

    if (loadFromFile(&root, filename) != 0) {
        printf("Файл %s не найден, начинаем с пустого списка.\n", filename);
    }

    while (1) {
        printf("\nМеню:\n");
        printf("1. Добавить контакт\n");
        printf("2. Редактировать контакт\n");
        printf("3. Удалить контакт\n");
        printf("4. Сохранить контакты в файл\n");
        printf("5. Загрузить контакты из файла\n");
        printf("6. Вывести все контакты\n");
        printf("7. Выход\n");
        printf("Выберите действие: ");

        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: {
                char* lastName = inputString("Введите фамилию: ");
                char* firstName = inputString("Введите имя: ");
                char* middleName = inputString("Введите отчество (Enter, если нет): ");
                char* workplace = inputString("Введите место работы (Enter, если нет): ");
                char* position = inputString("Введите должность (Enter, если нет): ");
                char* phone = inputString("Введите номер телефона (Enter, если нет): ");
                char* email = inputString("Введите email (Enter, если нет): ");
                char* socialLink = inputString("Введите ссылку на соцсети (Enter, если нет): ");
                char* messengerProfile = inputString("Введите профиль мессенджера (Enter, если нет): ");

                if (addContact(&root, lastName, firstName, middleName, workplace, position,
                                phone, email, socialLink, messengerProfile) == 0) {
                    printf("Контакт успешно добавлен.\n");
                } else {
                    printf("Ошибка при добавлении контакта!\n");
                }

                freeInput(lastName);
                freeInput(firstName);
                freeInput(middleName);
                freeInput(workplace);
                freeInput(position);
                freeInput(phone);
                freeInput(email);
                freeInput(socialLink);
                freeInput(messengerProfile);
                break;
            }
            case 2: {
                char* idStr = inputString("Введите ID контакта для редактирования: ");
                if (!idStr) {
                    printf("Неверный ввод.\n");
                    break;
                }
                int id = atoi(idStr);
                freeInput(idStr);
                AVLNode* node = findNodeById(root, id);
                if (!node) {
                    printf("Контакт с ID %d не найден!\n", id);
                    break;
                }
                char* newLastName = inputString("Новая фамилия (Enter, чтобы не менять): ");
                char* newFirstName = inputString("Новое имя (Enter, чтобы не менять): ");
                char* newMiddleName = inputString("Новое отчество (Enter, чтобы не менять): ");
                char* newWorkplace = inputString("Новое место работы (Enter, чтобы не менять): ");
                char* newPosition = inputString("Новая должность (Enter, чтобы не менять): ");
                char* newPhone = inputString("Новый номер телефона (Enter, чтобы не менять): ");
                char* newEmail = inputString("Новый email (Enter, чтобы не менять): ");
                char* newSocialLink = inputString("Новая ссылка на соцсети (Enter, чтобы не менять): ");
                char* newMessengerProfile = inputString("Новый профиль мессенджера (Enter, чтобы не менять): ");

                if (editContact(&root, id, newLastName, newFirstName, newMiddleName,
                                 newWorkplace, newPosition, newPhone, newEmail,
                                 newSocialLink, newMessengerProfile) == 0) {
                    printf("Контакт обновлен.\n");
                } else {
                    printf("Ошибка при редактировании контакта с ID %d!\n", id);
                }

                freeInput(newLastName);
                freeInput(newFirstName);
                freeInput(newMiddleName);
                freeInput(newWorkplace);
                freeInput(newPosition);
                freeInput(newPhone);
                freeInput(newEmail);
                freeInput(newSocialLink);
                freeInput(newMessengerProfile);
                break;
            }
            case 3: {
                char* idStr = inputString("Введите ID контакта для удаления: ");
                if (!idStr) {
                    printf("Неверный ввод.\n");
                    break;
                }
                int id = atoi(idStr);
                freeInput(idStr);
                if (deleteContact(&root, id) == 0) {
                    printf("Контакт удален.\n");
                } else {
                    printf("Контакт с ID %d не найден!\n", id);
                }
                break;
            }
            case 4:
                if (saveToFile(root, filename) == 0) {
                    printf("Контакты сохранены в файл %s\n", filename);
                } else {
                    printf("Ошибка открытия файла!\n");
                }
                break;
            case 5:
                if (loadFromFile(&root, filename) == 0) {
                    printf("Контакты загружены из файла %s\n", filename);
                } else {
                    printf("Файл %s не найден, начинаем с пустого списка.\n", filename);
                }
                break;
            case 6: {
                char output[4096];
                output[0] = '\0';
                printContacts(root, output, sizeof(output));
                if (strlen(output) == 0) {
                    printf("Список контактов пуст.\n");
                } else {
                    printf("%s", output);
                }
                printf("\n\n");
                printTree(root,0);
                break;
            }
            case 7:
                freeTree(root);
                printf("Выход из программы.\n");
                return 0;
            default:
                printf("Неверный выбор, попробуйте снова.\n");
        }
    }
    return 0;
}