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
	FullName name;
	char* workplace;
	char* position;
	ContactInfo details;
} Contact;

void freeContact(Contact* contact);
int addContact(Contact** contacts, unsigned int* count, char* lastName, char* firstName,
		char* surname, char* workplace, char* position, char* phone, char* email,
		char* socialLink, char* mesProfile);
int editeContact(Contact** contacts, unsigned int* count, unsigned int id,
		char* lastName, char* firstName,
		char* surname, char* workplace, char* position,
		char* phone, char* email,
		char* socialLink, char* mesProfile);
int deleteContact(Contact** contacts, unsigned int* count, unsigned int id);

int saveContactBook(const Contact* contacts, int count, const char* filename);
int readContactBook(Contact** contacts, unsigned int* count, const char* filename);
void printContactBook(const Contact* contacts, int count, char* output, int outputSize);

#endif
