#include "header.h"
#include <math.h>

void division(double* result, double a, double b) {
    if (b == 0) {
        *result = NAN;
        return;
    }
    *result = a / b;
}