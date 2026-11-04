#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_BASE,
    STATUS_MEMORY_ERROR,
    STATUS_OVERFLOW,
    STATUS_INVALID_NUMBER,
    STATUS_INPUT_ERROR
} Status;

static size_t add_one(size_t x){
    size_t carry = 1;
    while (carry != 0){
        size_t sum = x ^ carry;
        carry = (x & carry) << 1;
        x = sum;
    }
    return x;
}

static Status push_char(char **buf, size_t *count, size_t *capacity, char c){
    if (add_one(*count) >= *capacity){
        if (*capacity > ((size_t)-1 >> 1))
            return STATUS_OVERFLOW;
        char *tmp = realloc(*buf, (*capacity << 1) * sizeof(**buf));
        if (tmp == NULL)
            return STATUS_MEMORY_ERROR;
        *buf = tmp;
        *capacity <<= 1;
    }
    (*buf)[*count] = c;
    *count = add_one(*count);
    return STATUS_OK;
}

Status translation(long long num, int base, char **res){
    if (res == NULL)
        return STATUS_NULL_ARGUMENT;
    *res = NULL;

    int bits = 0;
    switch (base){
        case 2:  bits = 1; break;
        case 4:  bits = 2; break;
        case 8:  bits = 3; break;
        case 16: bits = 4; break;
        case 32: bits = 5; break;
        default: return STATUS_INVALID_BASE;
    }

    unsigned int mask = 0;                    
    for (int i = 0; i < bits; i = (int)add_one((size_t)i))
        mask = (mask << 1) | 1u;

    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUV";

    int negative = num < 0;
    unsigned long long value = (unsigned long long)num;
    if (negative){
        value = ~value;                         
        unsigned long long carry = 1;
        while (carry != 0){
            unsigned long long sum = value ^ carry;
            carry = (value & carry) << 1;
            value = sum;
        }
    }

    size_t capacity = 16;
    size_t count = 0;
    char *reverse = malloc(capacity * sizeof(*reverse));
    if (reverse == NULL)
        return STATUS_MEMORY_ERROR;

    Status st = STATUS_OK;

    if (value == 0)
        st = push_char(&reverse, &count, &capacity, '0');

    while (st == STATUS_OK && value != 0){
        st = push_char(&reverse, &count, &capacity, digits[value & mask]);
        value >>= bits;
    }
    if (st == STATUS_OK && negative)
        st = push_char(&reverse, &count, &capacity, '-');

    if (st != STATUS_OK){
        free(reverse);
        return st;
    }

    char *result = malloc(add_one(count) * sizeof(*result));
    if (result == NULL){
        free(reverse);
        return STATUS_MEMORY_ERROR;
    }

    size_t pos = 0;
    size_t i = count;
    while (i != 0){
        size_t j = i;
        size_t borrow = 1;                    
        while (borrow != 0){
            size_t d = j ^ borrow;
            borrow = (~j & borrow) << 1;
            j = d;
        }
        i = j;
        result[pos] = reverse[i];
        pos = add_one(pos);
    }
    result[pos] = '\0';

    free(reverse);
    *res = result;
    return STATUS_OK;
}

void print_status_error(Status status){
    switch (status){
        case STATUS_NULL_ARGUMENT: printf("Внутренняя ошибка: нулевой указатель.\n"); break;
        case STATUS_INVALID_BASE:  printf("Ошибка: основание должно быть 2, 4, 8, 16 или 32.\n"); break;
        case STATUS_MEMORY_ERROR:  printf("Ошибка: не удалось выделить память.\n"); break;
        case STATUS_OVERFLOW:      printf("Ошибка: переполнение.\n"); break;
        case STATUS_INVALID_NUMBER:printf("Введено некорректное число.\n"); break;
        case STATUS_INPUT_ERROR:   printf("Ошибка: неудалось сяитать число и основание.\n");
        case STATUS_OK:            break;
        default:                   printf("Ошибка.\n"); break;
    }
}

Status read_word(FILE *stream, char **word, int *found){
    if (stream == NULL || word == NULL || found == NULL)
        return STATUS_NULL_ARGUMENT;

    *word = NULL;
    *found = 0;

    int c;
    do {
        c = fgetc(stream);
    } while (c != EOF && isspace((unsigned char)c));

    if (c == EOF)
        return ferror(stream) ? STATUS_INPUT_ERROR : STATUS_OK;

    size_t capacity = 16;
    size_t length = 0;
    char *buffer = malloc(capacity * sizeof(*buffer));
    if (buffer == NULL)
        return STATUS_MEMORY_ERROR;

    while (c != EOF && !isspace((unsigned char)c)){
        if (length + 1 >= capacity){
            if (capacity > (size_t)-1 / 2 / sizeof(*buffer)){
                free(buffer);
                return STATUS_OVERFLOW;
            }
            char *tmp = realloc(buffer, capacity * 2 * sizeof(*buffer));
            if (tmp == NULL){
                free(buffer);
                return STATUS_MEMORY_ERROR;
            }
            buffer = tmp;
            capacity *= 2;
        }
        buffer[length++] = (char)c;
        c = fgetc(stream);
    }

    if (c == EOF && ferror(stream)){
        free(buffer);
        return STATUS_INPUT_ERROR;
    }

    buffer[length] = '\0';
    *word = buffer;
    *found = 1;
    return STATUS_OK;
}

Status check_num(const char *str, long long *num){
    if (str == NULL || num == NULL)
        return STATUS_NULL_ARGUMENT;
    if (str[0] == '\0')
        return STATUS_INVALID_NUMBER;

    size_t i = 0;
    int negative = 0;
    if (str[i] == '+' || str[i] == '-'){
        negative = (str[i] == '-');
        i++;
    }
    if (str[i] == '\0')
        return STATUS_INVALID_NUMBER;

    unsigned long long limit = negative ? (unsigned long long)LLONG_MAX + 1ULL : (unsigned long long)LLONG_MAX;
    unsigned long long value = 0;
    for (; str[i] != '\0'; i++){
        if (str[i] < '0' || str[i] > '9')
            return STATUS_INVALID_NUMBER;
        unsigned long long d = (unsigned long long)(str[i] - '0');
        if (value > (limit - d) / 10ULL)
            return STATUS_OVERFLOW;
        value = value * 10ULL + d;
    }

    *num = negative ? (long long)(0ULL - value) : (long long)value;
    return STATUS_OK;
}

Status parse_base(const char *str, int *base){
    if (str == NULL || base == NULL)
        return STATUS_NULL_ARGUMENT;
    if (str[0] == '\0')
        return STATUS_INVALID_NUMBER;

    int value = 0;
    for (size_t i = 0; str[i] != '\0'; i++){
        if (str[i] < '0' || str[i] > '9')
            return STATUS_INVALID_NUMBER;
        value = value * 10 + (str[i] - '0');
        if (value > 32)
            return STATUS_INVALID_BASE;    
    }
    *base = value;
    return STATUS_OK;
}

int main(void){
    char *word_num = NULL, *word_base = NULL;
    int found_num = 0, found_base = 0;
    long long num = 0;
    int base = 0;
    char *res_num = NULL;

    printf("Введите число и основание (2, 4, 8, 16, 32):\n");

    Status st = read_word(stdin, &word_num, &found_num);
    if (st == STATUS_OK)
        st = read_word(stdin, &word_base, &found_base);
    if (st == STATUS_OK && (!found_num || !found_base))
        st = STATUS_INPUT_ERROR;
    if (st == STATUS_OK)
        st = check_num(word_num, &num);
    if (st == STATUS_OK)
        st = parse_base(word_base, &base);
    if (st == STATUS_OK)
        st = translation(num, base, &res_num);

    free(word_num);
    free(word_base);

    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }

    printf("%s\n", res_num);
    free(res_num);
    return 0;
}