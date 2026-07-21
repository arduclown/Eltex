#include "header.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void freeList(Node* head) {
    while(head) {
        Node* temp = head;
        head = head->next;
        freeContact(&temp->contact);
        free(temp);
    }
}

static int compareContacts(const Contact* a, const Contact* b) {
    if (a->name.lastName && b->name.lastName) {
        return strcmp(a->name.lastName, b->name.lastName);
    } else if (a->name.lastName) {
        return 1;
    } else if (b->name.lastName) {
        return -1;
    } else {
        return 0;
    }
}

// пересчёт ID: после вставки или удаления контакты нумеруются заново с 0
static void reindexContacts(Node* head) {
    unsigned int id = 0;
    while (head) {
        head->contact.id = id++;
        head = head->next;
    }
}

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

int addContact(Node** head, char* lastName, char* firstName,
		char* surname, char* workplace, char* position, char* phone, char* email,
		char* socialLink, char* mesProfile){
    Node* newNode = malloc(sizeof(Node));
    if (!newNode) return -1;
    newNode->next = NULL;
    newNode->prev = NULL;
    newNode->contact.id = 0;

	newNode->contact.name.lastName = copyString(lastName ? lastName : "");
	newNode->contact.name.firstName = copyString(firstName ? firstName : "");
	newNode->contact.name.surname = copyString(surname ? surname : "");
	newNode->contact.workplace = copyString(workplace ? workplace : "");
	newNode->contact.position = copyString(position ? position : "");
	newNode->contact.details.phone = copyString(phone ? phone : "");
	newNode->contact.details.email = copyString(email ? email : "");
	newNode->contact.details.socialLink = copyString(socialLink ? socialLink : "");
	newNode->contact.details.mesProfile = copyString(mesProfile ? mesProfile : "");

    // список пуст
    if (!*head){
        *head = newNode;
        reindexContacts(*head);
        return 0;
    }

    Node* cur = *head;
    // ищем место для вставки нового контакта
    while (cur && compareContacts(&cur->contact, &newNode->contact) < 0)
        cur = cur->next; // указывает на первый контакт, который больше или равен новому
    
    if (cur) {
        newNode->next = cur;
        newNode->prev = cur->prev;
        if (cur->prev)
            cur->prev->next = newNode;
        else
            *head = newNode;
        cur->prev = newNode;
    } 
    else {
        Node* last = *head;
        while (last->next) 
            last = last->next;
        last->next = newNode;
        newNode->prev = last;
    }
    reindexContacts(*head);
	return 0;
}

int deleteContact(Node** head, unsigned int id) {
	Node* cur = *head;

    while(cur) {
        if (cur->contact.id == id) {
            if (cur->prev)
                    cur->prev->next = cur->next;
                else
                    *head = cur->next;
                if (cur->next) 
                    cur->next->prev = cur->prev;
                freeContact(&cur->contact);
                free(cur);
                reindexContacts(*head);
                return 0;
        }
        cur = cur->next;

    }
	return -1;
}

int editeContact(Node** head, unsigned int id,
		char* lastName, char* firstName,
		char* surname, char* workplace, char* position,
		char* phone, char* email,
		char* socialLink, char* mesProfile) {
    Node* cur = *head;

    while (cur) {
        if (cur->contact.id == id) {
            int lastNameChanged = (lastName && strlen(lastName) > 0 && strcmp(lastName, cur->contact.name.lastName)!=0);
            
            if (lastName && strlen(lastName) > 0){
                free(cur->contact.name.lastName);
                cur->contact.name.lastName = copyString(lastName);
            }

            if (firstName && strlen(firstName) > 0){
                free(cur->contact.name.firstName);
                cur->contact.name.firstName = copyString(firstName);
            }

            if (surname && strlen(surname) > 0){
                free(cur->contact.name.surname);
                cur->contact.name.surname = copyString(surname);
            }

            if (workplace){
                free(cur->contact.workplace);
                cur->contact.workplace = copyString(workplace);
            }

            if (position){
                free(cur->contact.position);
                cur->contact.position = copyString(position);
            }

            if (phone){
                free(cur->contact.details.phone);
                cur->contact.details.phone = copyString(phone);
            }

            if (email){
                free(cur->contact.details.email);
                cur->contact.details.email = copyString(email);
            }

            if (socialLink){
                free(cur->contact.details.socialLink);
                cur->contact.details.socialLink = copyString(socialLink);
            }

            if (mesProfile){
                free(cur->contact.details.mesProfile);
                cur->contact.details.mesProfile = copyString(mesProfile);
            }

            if (lastNameChanged) {
                Node* temp = cur;
                if (cur->prev)
                    cur->prev->next = cur->next;
                else
                    *head = cur->next;
                if (cur->next) 
                    cur->next->prev = cur->prev;

                addContact(head, temp->contact.name.lastName, temp->contact.name.firstName, temp->contact.name.surname,
                    temp->contact.workplace, temp->contact.position, temp->contact.details.phone,
                    temp->contact.details.email, temp->contact.details.socialLink, temp->contact.details.mesProfile);
                
                freeContact(&temp->contact);
                free(temp);
            }

            reindexContacts(*head);
            return 0;
        }
        cur = cur->next;
    }
	
	return 0;
}

void printContactBook(const Node* head, char* output, int outputSize ) {
    output[0] = '\0';
    if (!head) {
        strncat(output, "Список контактов пуст.\n", outputSize - strlen(output) - 1);
        return;
    }
    const Node* cur = head;
    while (cur) {
        const Contact* c = &cur->contact;
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
                 c->id,
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
        cur = cur->next;
    }
}

int saveContactBook(const Node* head, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return -1;
    const Node* cur = head;

    while (cur) {
        const Contact* c = &cur->contact;
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
        cur = cur->next;
    }
    fclose(file);
    return 0;
}

int readContactBook(Node** head, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;
    *head = NULL;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        char* token = strtok(line, ",");
        char* lastName = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        char* firstName = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        char* surname = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        char* workplace = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        char* position = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        char* phone = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        char* email = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        char* socialLink = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        char* mesProfile = token ? strdup(token) : NULL;

        addContact(head, lastName, firstName, surname, workplace, position, 
            phone, email, socialLink, mesProfile);
        
        free(lastName);
        free(firstName);
        free(surname);
        free(workplace);
        free(position);
        free(phone);
        free(email);
        free(socialLink);
        free(mesProfile);
    }
    fclose(file);
    return 0; 

}
