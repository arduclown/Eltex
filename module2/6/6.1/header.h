#ifndef CONTACT_BOOK
#define CONTACT_BOOK

typedef struct {
	char* phone;
	char* email;
	char* socialLink;
	char* mesProfile;
} ContactInfo;

typedef struct {
	char* lastName;
	char* firstName;
	char* surname;
} FullName;

typedef struct {
	unsigned int id;
	FullName name;
	char* workplace;
	char* position;
	ContactInfo details;
} Contact;

// структура для двухсвязного спискаa
typedef struct Node {
	Contact contact;
	struct Node* next;
	struct Node* prev;
} Node;

void freeList(Node* head);
void freeContact(Contact* contact);
int addContact(Node** head, char* lastName, char* firstName,
		char* surname, char* workplace, char* position, char* phone, char* email,
		char* socialLink, char* mesProfile);
int editeContact(Node** head, unsigned int id,
		char* lastName, char* firstName,
		char* surname, char* workplace, char* position,
		char* phone, char* email,
		char* socialLink, char* mesProfile);
int deleteContact(Node** head, unsigned int id);

int saveContactBook(const Node* head, const char* filename);
int readContactBook(Node** head, const char* filename); 
void printContactBook(const Node* head, char* output, int outputSize ); 

#endif