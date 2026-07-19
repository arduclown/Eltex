#include <string.h>
#include "header.h"
#include <ctype.h>
#include <stdlib.h>

int getFilePermission(const char* filename, char* bits) {
    struct stat st;

    // получаем информацию о файле
    if (stat(filename, &st) == -1) 
        return -1;

    //st.mode содержит права доступа и тип файла
    mode_t mode = st.st_mode & 0777;
    bits[0] = (mode & S_IRUSR) ? '1' : '0';
    bits[1] = (mode & S_IWUSR) ? '1' : '0';
    bits[2] = (mode & S_IXUSR) ? '1' : '0';
    bits[3] = (mode & S_IRGRP) ? '1' : '0';
    bits[4] = (mode & S_IWGRP) ? '1' : '0';
    bits[5] = (mode & S_IXGRP) ? '1' : '0';
    bits[6] = (mode & S_IROTH) ? '1' : '0';
    bits[7] = (mode & S_IWOTH) ? '1' : '0';
    bits[8] = (mode & S_IXOTH) ? '1' : '0';
    bits[9] = '\0';
    return 0;
}

void printPermissions(const char* bits) {
    char letters[10], number[4];
    bitsToLetters(bits, letters);
    bitsToNumbers(bits, number);
    printf("Буквенный формат: %s\n", letters);
    printf("Цифровой формат: %s\n", number);
    printf("Битовый формат: %s\n", bits);
}

bool isPermissionsNumeric(const char* str) {
    if (strlen(str) != 3) return false;
    for (int i = 0; i < 3; i++) 
        if (!isdigit(str[i])) 
        return false;
    
    return true;
}

void numbersToBits(const char* number, char* bits) {
    int num = atoi(number);
    int usr = (num / 100) % 10;
    int grp = (num / 10) % 10;
    int oth = num % 10;

    bits[0] = (usr & 4) ? '1' : '0';
    bits[1] = (usr & 2) ? '1' : '0';
    bits[2] = (usr & 1) ? '1' : '0';
    bits[3] = (grp & 4) ? '1' : '0';
    bits[4] = (grp & 2) ? '1' : '0';
    bits[5] = (grp & 1) ? '1' : '0';
    bits[6] = (oth & 4) ? '1' : '0';
    bits[7] = (oth & 2) ? '1' : '0';
    bits[8] = (oth & 1) ? '1' : '0';
    bits[9] = '\0';
}

void bitsToNumbers(const char* bits, char* number) {
    int usr = (bits[0] == '1' ? 4 : 0) + (bits[1] == '1' ? 2 : 0) + (bits[2] == '1' ? 1 : 0);
    int grp = (bits[3] == '1' ? 4 : 0) + (bits[4] == '1' ? 2 : 0) + (bits[5] == '1' ? 1 : 0);
    int oth = (bits[6] == '1' ? 4 : 0) + (bits[7] == '1' ? 2 : 0) + (bits[8] == '1' ? 1 : 0);
    sprintf(number, "%d%d%d", usr, grp, oth);
}

void bitsToLetters(const char* bits, char* letters) {
    letters[0] = bits[0] == '1' ? 'r' : '-';
    letters[1] = bits[1] == '1' ? 'w' : '-';
    letters[2] = bits[2] == '1' ? 'x' : '-';
    letters[3] = bits[3] == '1' ? 'r' : '-';
    letters[4] = bits[4] == '1' ? 'w' : '-';
    letters[5] = bits[5] == '1' ? 'x' : '-';
    letters[6] = bits[6] == '1' ? 'r' : '-';
    letters[7] = bits[7] == '1' ? 'w' : '-';
    letters[8] = bits[8] == '1' ? 'x' : '-';
    letters[9] = '\0';
}

// Преобразование буквенного формата в битовый
void letterToBits(const char* letters, char* bits) {
    for (int i = 0; i < 9; i++) {
        bits[i] = (letters[i] != '-') ? '1' : '0';
    }
    bits[9] = '\0';
}

// [кто][операторы][прав]
// u/g/o/a  +/-/=  r/w/x
void applyModification(const char* command, char* bits) {
    int apply[4] = {0}; //отслеживание, какие категори ментяь (ugoa)
    const char* operator = strpbrk(command, "+=-"); //ищем указатель на оператор
    if (operator == NULL){
        fprintf(stderr, "Error: Оператор не найден.\n");
        return;
    }

    char op = *operator;
    const char* permissions = operator + 1; //указател на часть с правами

    // категории не указаны, применяется ко всем
    if (operator == command) 
        apply[3] = 1;
    else {
        for(const char* p = command; p < operator; p++){
            switch (*p)
            {
            case 'u':
                apply[0] = 1;
                break;
            case 'g':
                apply[1] = 1;
                break;
            case 'o':
                apply[2] = 1;
                break;
            case 'a':
                apply[3] = 1;
                break;
            default:
                fprintf(stderr, "Error: Неверно указана категория.");
                break;
            }
        }
    }

    if (apply[3] == 1) 
        apply[0] = apply[1] = apply[2] = 1;
    
    // диапозоны для групп 
    int range[3][2] = {{0, 3}, {3, 6}, {6, 9}};

    if (op == '=') {
        for (int i = 0; i < 3; i++) {
            if (apply[i]) {
                for (int j = range[i][0]; j < range[i][1]; i++)
                    bits[j] = '0';
            }
        }
    }

    for (const char* p = permissions; *p != '\0'; p++) {
        int bitOffset;
        switch (*p){
            case 'r': 
                bitOffset = 0;
                break;
            case 'w': 
                bitOffset = 1;
                break;
            case 'x': 
                bitOffset = 2;
                break;
            default:
                fprintf(stderr, "Error: неверное право.\n");
                continue;
        }

        // Применяем к выбранным категориям
        for (int i = 0; i < 3; i++) {
            if (apply[i]) {
                int bit_pos = range[i][0] + bitOffset;
                if (op == '+') bits[bit_pos] = '1';
                else if (op == '-') bits[bit_pos] = '0';
                else if (op == '=') bits[bit_pos] = '1';
            }
        }
    }

}