#ifndef HEADER_H
#define HEADER_H
#include <stddef.h>

typedef struct {
    char* phone;
    char* email;
    char* socialLink;
    char* messengerProfile;
} ContactDetails;

typedef struct {
    char* lastName;
    char* firstName;
    char* middleName;
} FullName;

typedef struct {
    int id;  
    FullName name;
    char* workplace;
    char* position;
    ContactDetails details;
} Contact;

// Структура узла AVL-дерева
typedef struct AVLNode {
    Contact contact;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;

// Функции для работы с AVL-деревом
int getHeight(AVLNode* node);
int getBalance(AVLNode* node);
int chooseDirection(const char* newLastName, const char* newFirstName, 
                     const char* currentLastName, const char* currentFirstName);
AVLNode* rotateRight(AVLNode* y);
AVLNode* rotateLeft(AVLNode* x);
AVLNode* createNode(const Contact* contact);
AVLNode* insert(AVLNode* node, const Contact* contact);
AVLNode* deleteNode(AVLNode* root, const char* lastName, const char* firstName);
AVLNode* findNode(AVLNode* node, const char* lastName, const char* firstName);
AVLNode* findNodeById(AVLNode* node, int id);
void freeTree(AVLNode* node);

// Функции для работы с контактами
int addContact(AVLNode** root, const char* lastName, const char* firstName, const char* middleName,
                const char* workplace, const char* position, const char* phone, const char* email,
                const char* socialLink, const char* messengerProfile);
int editContact(AVLNode** root, int id, const char* newLastName,
                 const char* newFirstName, const char* newMiddleName, const char* newWorkplace,
                 const char* newPosition, const char* newPhone, const char* newEmail,
                 const char* newSocialLink, const char* newMessengerProfile);
int deleteContact(AVLNode** root, int id);
int saveToFile(const AVLNode* root, const char* filename);
int loadFromFile(AVLNode** root, const char* filename);
void freeContact(Contact* contact);
void printContacts(const AVLNode* root, char* output, unsigned int outputSize);
void printTree(AVLNode* node, int level);

#endif // HEADER_H