#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>


int getFilePermission(const char* filename, char* bits);
void printPermissions(const char* bits);
bool isPermissionsNumeric(const char* str);
void bitsToLetters(const char* bits, char* letters);
void bitsToNumbers(const char* bits, char* number);
void numbersToBits(const char* number, char* bits);
void applyModification(const char* command, char* bits);
void letterToBits(const char* letters, char* bits);

#endif