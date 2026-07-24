#include "header.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INPUT 256

// функция для ввода строки
static char* readString(const char* text) {
	printf("%s", text);
	char buffer[MAX_INPUT];
	if (fgets(buffer, MAX_INPUT, stdin) == NULL) return NULL;
	buffer[strcspn(buffer, "\n")] = 0;
	if (strlen(buffer) == 0) return NULL;
	char* str = malloc(strlen(buffer) + 1);
	if (str) strcpy(str, buffer);
	return str;
}

static void free_input(char* str) {
    if (str) free(str);
}

int main(){
    Node* head = NULL;
	const char* filename = "contacts.txt";

	while (1){
		printf("\nМеню:\n"
			"1. Добавить контакт\n"
			"2. Редактировать контакт\n"
			"3. Удалить контакт\n"
			"4. Сохранить контакты в файл\n"
			"5. Загрузить контакты из файла\n"
			"6. Вывести все контакты\n"
			"7. Выход\n"
			"Выберите действие: ");

		int choice;
		scanf("%d", &choice);
		getchar();

		switch(choice) {
		case 1: {
			char* lastName = readString("Введите фамилию: ");
			char* firstName = readString("Введите имя: ");
			char* surname = readString("Введите отчество (Enter, если нет): ");
			char* workplace = readString("Введите место работы (Enter, если нет): ");
			char* position = readString("Введите должность (Enter, если нет): ");
			char* phone = readString("Введите номер телефона (Enter, если нет): ");
			char* email = readString("Введите email (Enter, если нет): ");
			char* socialLink = readString("Введите ссылку на соцсети (Enter, если нет): ");
			char* mesProfile = readString("Введите профиль мессенджера (Enter, если нет): ");

			if (addContact(&head, lastName, firstName, surname, workplace,
					position, phone, email, socialLink, mesProfile) == 0)
				printf("Контакт добавлен.\n");
			else{
				printf("Ошибка при добавлении контакта!\n");
			}
			free_input(lastName);
            free_input(firstName);
            free_input(surname);
            free_input(workplace);
            free_input(position);
            free_input(phone);
            free_input(email);
            free_input(socialLink);
            free_input(mesProfile);
			break;
		}
        case 2: {
            unsigned int id;
            printf("Введите ID контакта: ");
            scanf("%u", &id);
            getchar();

            char* lastName = readString("Новая фамилия (Enter, чтобы не менять):");
			char* firstName = readString("Новое имя (Enter, чтобы не менять): ");
			char* surname = readString("Новое отчество (Enter, чтобы не менять): ");
			char* workplace = readString("Новое место работы (Enter, чтобы не менять): ");
			char* position = readString("Новая должность (Enter, чтобы не менять): ");
			char* phone = readString("Новый номер телефона (Enter, чтобы не менять): ");
			char* email = readString("Новый email (Enter, чтобы не менять): ");
			char* socialLink = readString("Новая ссылка на соцсети (Enter, чтобы не менять): ");
			char* mesProfile = readString("Новый профиль мессенджера (Enter, чтобы не менять): ");

            if (editeContact(&head, id, lastName, firstName, surname,
            workplace, position, phone, email, socialLink, mesProfile) == 0)
                printf("Контакт обнавлен. \n");
            else
                printf("Контакт с ID %u не найден!\n", id);

            free_input(lastName);
            free_input(firstName);
            free_input(surname);
            free_input(workplace);
            free_input(position);
            free_input(phone);
            free_input(email);
            free_input(socialLink);
            free_input(mesProfile);
            break;
        }
        case 3: {
            unsigned int id;
            printf("Введите ID контакта для удаления: ");
            scanf("%u", &id);
            getchar();

            if (deleteContact(&head, id) == 0)
                printf("Контакт удален.\n");
            else
                printf("ID %u не найден! \n", id);
            break;
        }
        case 4: {
            if (saveContactBook(head, filename) == 0)
                printf("Контакты сохранены в файл %s\n", filename);
            else
                printf("Ошибка открытия файла!\n");
            break;
        }
        case 5: {
            if (readContactBook(&head, filename) == 0)
                printf("Контакты загружены из файла %s\n.", filename);
            else
                printf("Файл %s не найден.\n", filename);
            break;
        }
        case 6: {
            char output[4096]; // Буфер для вывода
            printContactBook(head, output, sizeof(output));
            printf("%s", output);
            break;
        }
        case 7: {
            freeList(head);
            printf("Выход из программы.\n");
            return 0;
        }
        default:
            printf("Неверный выбор, введите цифру от 1 до 7!\n");
		}
	}
    return 0;
}
