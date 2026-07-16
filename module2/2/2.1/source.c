#include "header.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void freeContact(Contact* contact) {
	free(contact->name.lastName);
	free(contact->name.firstName);
	free(contact->name.surname);
	free(contact->workplace);
	free(contact->position);
	free(contact->details.mesProfile);
	free(contact->details.email);
	free(contact->details.phone);
	free(contact->details.socialLink);
}

static char* copyString(const char* src) {
	if (!src || strlen(src) == 0) return NULL;
	char* dest = malloc(strlen(src) + 1);
	if (dest) strcpy(dest, src);
	return dest;
}

int addContact(Contact** contacts, unsigned int* count, char* lastName, char* firstName,
		char* surname, char* workplace, char* position, char* phone, char* email,
		char* socialLink, char* mesProfile){
	Contact* tmp = realloc(*contacts, sizeof(Contact) * (*count + 1));
	if (!tmp) return -1;
	*contacts = tmp;
	Contact* newContact = &(*contacts)[*count];

	newContact->name.lastName = copyString(lastName ? lastName : "");
	newContact->name.firstName = copyString(firstName ? firstName : "");
	newContact->name.surname = copyString(surname ? surname : "");
	newContact->workplace = copyString(workplace ? workplace : "");
	newContact->position = copyString(position ? position : "");
	newContact->details.phone = copyString(phone ? phone : "");
	newContact->details.email = copyString(email ? email : "");
	newContact->details.socialLink = copyString(socialLink ? socialLink : "");
	newContact->details.mesProfile = copyString(mesProfile ? mesProfile : "");

	(*count)++;
	return 0;
}

int deleteContact(Contact** contacts, unsigned int* count, unsigned int id) {
	if (id >= *count) return 1;
	freeContact(&(*contacts)[id]);
	for (unsigned int i = id; i + 1 < *count; i++)
		(*contacts)[i] = (*contacts)[i+1];
	(*count)--;
	Contact* tmp = realloc(*contacts, *count * sizeof(Contact));
	if (tmp || *count == 0) *contacts = tmp;
	return 0;
}

int editeContact(Contact** contacts, unsigned int* count, unsigned int id,
		char* lastName, char* firstName,
		char* surname, char* workplace, char* position,
		char* phone, char* email,
		char* socialLink, char* mesProfile) {
	if (id >= *count) return -1;
	Contact* contact = &(*contacts)[id];

	if (lastName && strlen(lastName) > 0){
		free(contact->name.lastName);
		contact->name.lastName = copyString(lastName);
	}

	if (firstName && strlen(firstName) > 0){
		free(contact->name.firstName);
		contact->name.firstName = copyString(firstName);
	}

	if (surname && strlen(surname) > 0){
		free(contact->name.surname);
		contact->name.surname = copyString(surname);
	}

	if (workplace){
		free(contact->workplace);
		contact->workplace = copyString(workplace);
	}

	if (position){
		free(contact->position);
		contact->position = copyString(position);
	}

	if (phone){
		free(contact->details.phone);
		contact->details.phone = copyString(phone);
	}

	if (email){
		free(contact->details.email);
		contact->details.email = copyString(email);
	}

	if (socialLink){
		free(contact->details.socialLink);
		contact->details.socialLink = copyString(socialLink);
	}

	if (mesProfile){
		free(contact->details.mesProfile);
		contact->details.mesProfile = copyString(mesProfile);
	}

	return 0;
}

void printContactBook(const Contact* contacts, int count, char* output, int outputSize ) {
    output[0] = '\0';
    if (count == 0) {
        strncat(output, "Список контактов пуст.\n", outputSize - strlen(output) - 1);
        return;
    }

    for (int i = 0; i < count; i++) {
        const Contact* c = &contacts[i];
        char buffer[1024];
        snprintf(buffer, sizeof(buffer),
                 "Контакт ID %u:\n"
                 "  Фамилия: %s\n"
                 "  Имя: %s\n"
                 "  Отчество: %s\n"
                 "  Место работы: %s\n"
                 "  Должность: %s\n"
                 "  Телефон: %s\n"
                 "  Email: %s\n"
                 "  Соцсети: %s\n"
                 "  Мессенджер: %s\n"
                 "--------------------\n",
                 i,
                 c->name.lastName ? c->name.lastName : "(не указано)",
                 c->name.firstName ? c->name.firstName : "(не указано)",
                 c->name.surname ? c->name.surname : "(не указано)",
                 c->workplace ? c->workplace : "(не указано)",
                 c->position ? c->position : "(не указано)",
                 c->details.phone ? c->details.phone : "(не указано)",
                 c->details.email ? c->details.email : "(не указано)",
                 c->details.socialLink ? c->details.socialLink : "(не указано)",
                 c->details.mesProfile ? c->details.mesProfile : "(не указано)");
        strncat(output, buffer, outputSize - strlen(output) - 1);
    }
}

int saveContactBook(const Contact* contacts, int count, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return -1;
    for (int i = 0; i < count; i++) {
        const Contact* c = &contacts[i];
        fprintf(file, "%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
            c->name.lastName ? c->name.lastName : "",
            c->name.firstName ? c->name.firstName : "",
            c->name.surname ? c->name.surname : "",
            c->workplace ? c->workplace : "",
            c->position ? c->position : "",
            c->details.phone ? c->details.phone : "",
            c->details.email ? c->details.email : "",
            c->details.socialLink ? c->details.socialLink : "",
            c->details.mesProfile ? c->details.mesProfile : "");
    }
    fclose(file);
    return 0;
}

int readContactBook(Contact** contacts, unsigned int* count, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;
    for (unsigned int i = 0; i < *count; i++)
        freeContact(&(*contacts)[i]);
    free(*contacts);
    *contacts = NULL;
    *count = 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        Contact* temp = realloc(*contacts, (*count + 1) * sizeof(Contact));
        if (!temp) {
            fclose(file);
            return -1;
        }
        *contacts = temp;
        Contact* c = &(*contacts)[*count];

        char* token = strtok(line, ",");
        c->name.lastName = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        c->name.firstName = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        c->name.surname = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        c->workplace = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        c->position = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        c->details.phone = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        c->details.email = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        c->details.socialLink = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        c->details.mesProfile = token ? strdup(token) : NULL;

        (*count)++;
    }
    fclose(file);
    return 0; 

}

