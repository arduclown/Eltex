#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int nextId = 0;  // Следующий доступный ID для новых контактов

static char* copyString(const char* src) {
    if (!src || strlen(src) == 0) return NULL;
    char* dest = malloc(strlen(src) + 1);
    if (dest) strcpy(dest, src);
    return dest;
}
void copyContact(Contact* dest, const Contact* src) {
    dest->id = src->id;
    dest->name.lastName = copyString(src->name.lastName);
    dest->name.firstName = copyString(src->name.firstName);
    dest->name.middleName = copyString(src->name.middleName);
    dest->workplace = copyString(src->workplace);
    dest->position = copyString(src->position);
    dest->details.phone = copyString(src->details.phone);
    dest->details.email = copyString(src->details.email);
    dest->details.socialLink = copyString(src->details.socialLink);
    dest->details.messengerProfile = copyString(src->details.messengerProfile);
}
AVLNode* findMin(AVLNode* node) {
    while (node->left) node = node->left;
    return node;
}
int getHeight(AVLNode* node) {
    return node ? node->height : 0;
}

int getBalance(AVLNode* node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

int chooseDirection(const char* newLastName, const char* newFirstName,
                     const char* currentLastName, const char* currentFirstName) {
    int lastNameCmp = strcmp(newLastName, currentLastName);
    if (lastNameCmp != 0) {
        return lastNameCmp;
    }
    return strcmp(newFirstName, currentFirstName);
}

AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = 1 + (getHeight(y->left) > getHeight(y->right) ? getHeight(y->left) : getHeight(y->right));
    x->height = 1 + (getHeight(x->left) > getHeight(x->right) ? getHeight(x->left) : getHeight(x->right));
    return x;
}

AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = 1 + (getHeight(x->left) > getHeight(x->right) ? getHeight(x->left) : getHeight(x->right));
    y->height = 1 + (getHeight(y->left) > getHeight(y->right) ? getHeight(y->left) : getHeight(y->right));
    return y;
}

AVLNode* createNode(const Contact* contact) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    if (node) {
        node->contact.id = contact->id;  // Копируем ID
        node->contact.name.lastName = copyString(contact->name.lastName);
        node->contact.name.firstName = copyString(contact->name.firstName);
        node->contact.name.middleName = copyString(contact->name.middleName);
        node->contact.workplace = copyString(contact->workplace);
        node->contact.position = copyString(contact->position);
        node->contact.details.phone = copyString(contact->details.phone);
        node->contact.details.email = copyString(contact->details.email);
        node->contact.details.socialLink = copyString(contact->details.socialLink);
        node->contact.details.messengerProfile = copyString(contact->details.messengerProfile);
        node->left = NULL;
        node->right = NULL;
        node->height = 1;
    }
    return node;
}
AVLNode* deleteNode(AVLNode* root, const char* lastName, const char* firstName) {
    if (!root) return root;

    // Сравнение для определения направления поиска
    int cmp = chooseDirection(lastName, firstName,
                               root->contact.name.lastName, root->contact.name.firstName);
    
    if (cmp < 0) {
        root->left = deleteNode(root->left, lastName, firstName);
    } else if (cmp > 0) {
        root->right = deleteNode(root->right, lastName, firstName);
    } else {
        // Узел для удаления найден
        if (!root->left || !root->right) {
            // Случай: нет дочерних узлов или один дочерний узел
            AVLNode* temp = root->left ? root->left : root->right;
            if (!temp) {
                // Нет дочерних узлов
                freeContact(&root->contact); // Освобождаем данные контакта
                free(root);                   // Освобождаем узел
                root = NULL;
            } else {
                // Один дочерний узел
                AVLNode* child = temp;
                *root = *child;           // Копируем данные из дочернего узла
                free(child);              // Освобождаем память дочернего узла
            }
        } else {
            // Случай: два дочерних узла
            AVLNode* temp = findMin(root->right); // Находим минимальный узел в правом поддереве
            freeContact(&root->contact);          // Освобождаем текущий контакт
            copyContact(&root->contact, &temp->contact); 
            root->right = deleteNode(root->right, temp->contact.name.lastName,
                                      temp->contact.name.firstName); // Удаляем минимальный узел
        }
    }

    if (!root) return root;

    // Обновление высоты и балансировка дерева
    root->height = 1 + (getHeight(root->left) > getHeight(root->right) ?
                        getHeight(root->left) : getHeight(root->right));
    int balance = getBalance(root);

    // Выполняем вращения для балансировки
    if (balance > 1 && getBalance(root->left) >= 0) {
        return rotateRight(root);
    }
    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }
    if (balance < -1 && getBalance(root->right) <= 0) {
        return rotateLeft(root);
    }
    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }
    return root;
}

AVLNode* insert(AVLNode* node, const Contact* contact) {
    if (!node) return createNode(contact);

    int direction = chooseDirection(contact->name.lastName, contact->name.firstName,
                                     node->contact.name.lastName, node->contact.name.firstName);
    if (direction < 0) {
        node->left = insert(node->left, contact);
    } else if (direction > 0) {
        node->right = insert(node->right, contact);
    } else {
        return node; // Дубликат (одинаковые фамилия и имя)
    }

    node->height = 1 + (getHeight(node->left) > getHeight(node->right) ?
                        getHeight(node->left) : getHeight(node->right));
    int balance = getBalance(node);

    if (balance > 1 && chooseDirection(contact->name.lastName, contact->name.firstName,
                                        node->left->contact.name.lastName,
                                        node->left->contact.name.firstName) < 0) {
        return rotateRight(node);
    }
    if (balance < -1 && chooseDirection(contact->name.lastName, contact->name.firstName,
                                         node->right->contact.name.lastName,
                                         node->right->contact.name.firstName) > 0) {
        return rotateLeft(node);
    }
    if (balance > 1 && chooseDirection(contact->name.lastName, contact->name.firstName,
                                        node->left->contact.name.lastName,
                                        node->left->contact.name.firstName) > 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (balance < -1 && chooseDirection(contact->name.lastName, contact->name.firstName,
                                         node->right->contact.name.lastName,
                                         node->right->contact.name.firstName) < 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}





AVLNode* findNode(AVLNode* node, const char* lastName, const char* firstName) {
    if (!node) return NULL;

    int cmp = chooseDirection(lastName, firstName,
                               node->contact.name.lastName, node->contact.name.firstName);
    if (cmp == 0) return node;
    if (cmp < 0) return findNode(node->left, lastName, firstName);
    return findNode(node->right, lastName, firstName);
}

AVLNode* findNodeById(AVLNode* node, int id) {
    if (!node) return NULL;
    if (node->contact.id == id) return node;
    AVLNode* leftResult = findNodeById(node->left, id);
    if (leftResult) return leftResult;
    return findNodeById(node->right, id);
}

void freeTree(AVLNode* node) {
    if (node) {
        freeTree(node->left);
        freeTree(node->right);
        freeContact(&node->contact);
        free(node);
    }
}

int addContact(AVLNode** root, const char* lastName, const char* firstName, const char* middleName,
                const char* workplace, const char* position, const char* phone, const char* email,
                const char* socialLink, const char* messengerProfile) {
    Contact newContact = {
        .id = nextId++,  // Присваиваем уникальный ID и увеличиваем счетчик
        .name = {
            .lastName = copyString(lastName ? lastName : ""),
            .firstName = copyString(firstName ? firstName : ""),
            .middleName = copyString(middleName)
        },
        .workplace = copyString(workplace),
        .position = copyString(position),
        .details = {
            .phone = copyString(phone),
            .email = copyString(email),
            .socialLink = copyString(socialLink),
            .messengerProfile = copyString(messengerProfile)
        }
    };
    *root = insert(*root, &newContact);
    freeContact(&newContact);
    return 0;
}

int editContact(AVLNode** root, int id, const char* newLastName,
                 const char* newFirstName, const char* newMiddleName, const char* newWorkplace,
                 const char* newPosition, const char* newPhone, const char* newEmail,
                 const char* newSocialLink, const char* newMessengerProfile) {
    AVLNode* node = findNodeById(*root, id);
    if (!node) return -1;

    Contact* contact = &node->contact;
    int keyChanged = (newLastName && strcmp(newLastName, contact->name.lastName) != 0) ||
                      (newFirstName && strcmp(newFirstName, contact->name.firstName) != 0);

    if (keyChanged) {
        Contact newContact = {
            .id = contact->id,
            .name = {
                .lastName = copyString(newLastName ? newLastName : contact->name.lastName),
                .firstName = copyString(newFirstName ? newFirstName : contact->name.firstName),
                .middleName = copyString(newMiddleName ? newMiddleName : contact->name.middleName)
            },
            .workplace = copyString(newWorkplace ? newWorkplace : contact->workplace),
            .position = copyString(newPosition ? newPosition : contact->position),
            .details = {
                .phone = copyString(newPhone ? newPhone : contact->details.phone),
                .email = copyString(newEmail ? newEmail : contact->details.email),
                .socialLink = copyString(newSocialLink ? newSocialLink : contact->details.socialLink),
                .messengerProfile = copyString(newMessengerProfile ? newMessengerProfile : contact->details.messengerProfile)
            }
        };
        *root = deleteNode(*root, contact->name.lastName, contact->name.firstName);
        *root = insert(*root, &newContact);
        freeContact(&newContact);
    } else {
        if (newMiddleName) {
            free(contact->name.middleName);
            contact->name.middleName = copyString(newMiddleName);
        }
        if (newWorkplace) {
            free(contact->workplace);
            contact->workplace = copyString(newWorkplace);
        }
        if (newPosition) {
            free(contact->position);
            contact->position = copyString(newPosition);
        }
        if (newPhone) {
            free(contact->details.phone);
            contact->details.phone = copyString(newPhone);
        }
        if (newEmail) {
            free(contact->details.email);
            contact->details.email = copyString(newEmail);
        }
        if (newSocialLink) {
            free(contact->details.socialLink);
            contact->details.socialLink = copyString(newSocialLink);
        }
        if (newMessengerProfile) {
            free(contact->details.messengerProfile);
            contact->details.messengerProfile = copyString(newMessengerProfile);
        }
    }
    return 0;
}

int deleteContact(AVLNode** root, int id) {
    AVLNode* node = findNodeById(*root, id);
    if (!node) return -1;
    *root = deleteNode(*root, node->contact.name.lastName, node->contact.name.firstName);
    return 0;
}


static void saveToFileHelper(const AVLNode* node, FILE* file) {
    if (!node) return;
    saveToFileHelper(node->left, file);
    const Contact* c = &node->contact;
    fprintf(file, "%d,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
            c->id,
            c->name.lastName ? c->name.lastName : "",
            c->name.firstName ? c->name.firstName : "",
            c->name.middleName ? c->name.middleName : "",
            c->workplace ? c->workplace : "",
            c->position ? c->position : "",
            c->details.phone ? c->details.phone : "",
            c->details.email ? c->details.email : "",
            c->details.socialLink ? c->details.socialLink : "",
            c->details.messengerProfile ? c->details.messengerProfile : "");
    saveToFileHelper(node->right, file);
}

int saveToFile(const AVLNode* root, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return -1;
    saveToFileHelper(root, file);
    fclose(file);
    return 0;
}

int loadFromFile(AVLNode** root, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;
    freeTree(*root);
    *root = NULL;

    int maxId = 0;
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        char* token = strtok(line, ",");
        int id = token ? atoi(token) : 0;
        if (id > maxId) maxId = id;
        token = strtok(NULL, ",");
        char* lastName = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        char* firstName = token ? strdup(token) : NULL;
        token = strtok(NULL, ",");
        char* middleName = token ? strdup(token) : NULL;
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
        char* messengerProfile = token ? strdup(token) : NULL;

        Contact newContact = {
            .id = id,
            .name = {
                .lastName = lastName,
                .firstName = firstName,
                .middleName = middleName
            },
            .workplace = workplace,
            .position = position,
            .details = {
                .phone = phone,
                .email = email,
                .socialLink = socialLink,
                .messengerProfile = messengerProfile
            }
        };
        *root = insert(*root, &newContact);
        free(lastName);
        free(firstName);
        free(middleName);
        free(workplace);
        free(position);
        free(phone);
        free(email);
        free(socialLink);
        free(messengerProfile);
    }
    fclose(file);
    nextId = maxId + 1;  // Обновляем nextId до следующего доступного значения
    return 0;
}

void freeContact(Contact* contact) {
    free(contact->name.lastName);
    free(contact->name.firstName);
    free(contact->name.middleName);
    free(contact->workplace);
    free(contact->position);
    free(contact->details.phone);
    free(contact->details.email);
    free(contact->details.socialLink);
    free(contact->details.messengerProfile);
}

void printContacts(const AVLNode* root, char* output, unsigned int outputSize) {
    if (!root) return;
    printContacts(root->left, output, outputSize);
    const Contact* c = &root->contact;
    char buffer[1024];
    snprintf(buffer, sizeof(buffer),
             "Контакт ID: %d\n"
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
             c->name.middleName ? c->name.middleName : "(не указано)",
             c->workplace ? c->workplace : "(не указано)",
             c->position ? c->position : "(не указано)",
             c->details.phone ? c->details.phone : "(не указано)",
             c->details.email ? c->details.email : "(не указано)",
             c->details.socialLink ? c->details.socialLink : "(не указано)",
             c->details.messengerProfile ? c->details.messengerProfile : "(не указано)");
    strncat(output, buffer, outputSize - strlen(output) - 1);
    printContacts(root->right, output, outputSize);
}
static void printTreeHelper(const AVLNode* node, const char* prefix, int isRight) {
    if (!node) return;

    char buf[1024];
    snprintf(buf, sizeof(buf), "%s%s", prefix, isRight ? "        " : "│       ");
    printTreeHelper(node->right, buf, 1);

    printf("%s%s%s (ID: %d)\n", prefix, isRight ? "┌────── " : "└────── ",
           node->contact.name.lastName, node->contact.id);

    snprintf(buf, sizeof(buf), "%s%s", prefix, isRight ? "│       " : "        ");
    printTreeHelper(node->left, buf, 0);
}

void printTree(AVLNode* node, int level) {
    (void)level;
    if (!node) {
        printf("(дерево пусто)\n");
        return;
    }
    printTreeHelper(node->right, "", 1);
    printf("%s (ID: %d)\n", node->contact.name.lastName, node->contact.id);
    printTreeHelper(node->left, "", 0);
}