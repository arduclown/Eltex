#ifndef CALCULATOR
#define CALCULATOR


#define MAX_OPERATIONS 10

typedef void (*action) (double*, double, double);

typedef struct {
    action func;
    const char* name;
    void* handle; // дескриптор библиотеки
} Operation;

void add(double* result, double a, double b);
void subtract(double* result, double a, double b);
void multiply(double* result, double a, double b);
void division(double* result, double a, double b);



#endif