#include "header.h"
#include <math.h>

void add(double* result, double a, double b) {
    *result = a + b;
}

void subtract(double* result, double a, double b) {
    *result = a - b;
}

void multiply(double* result, double a, double b) {
    *result = a * b;
}

void division(double* result, double a, double b) {
    if (b == 0) {
        *result = NAN;
        return;
    }
    *result = a / b;
}
