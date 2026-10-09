
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define ARRAY_SIZE 10

typedef enum {
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_ARGUMENT,
    STATUS_OVERFLOW,
    STATUS_EMPTY_STRING,
    STATUS_NO_DIGITS,
    STATUS_INVALID_NUMBER,
    STATUS_INVALID_RANGE
} Status;

#include <limits.h>
#include <stdlib.h>

#include <limits.h>
#include <stdlib.h>

Status fill_array(long long *array, size_t size, long long a, long long b){
    if (array == NULL)
        return STATUS_NULL_ARGUMENT;

    if (a > b)
        return STATUS_INVALID_RANGE;

    long long max_offset = (long long)RAND_MAX - 1;

    if (a <= LLONG_MAX - max_offset && b > a + max_offset)
        return STATUS_INVALID_RANGE;

    long long range = b - a + 1;

    for (size_t i = 0; i < size; ++i)
        array[i] = a + rand() % range;

    return STATUS_OK;
}

Status swap_min_max(long long *array, size_t size) {
    if (array == NULL)
        return STATUS_NULL_ARGUMENT;

    if (size == 0)
        return STATUS_INVALID_ARGUMENT;

    size_t min_index = 0;
    size_t max_index = 0;

    for (size_t i = 1; i < size; i++) {
        if (array[i] < array[min_index])
            min_index = i;

        if (array[i] > array[max_index])
            max_index = i;
    }

    long long temp = array[min_index];
    array[min_index] = array[max_index];
    array[max_index] = temp;

    return STATUS_OK;
}


Status print_array(const long long *array, size_t size) {
    if (array == NULL)
        return STATUS_NULL_ARGUMENT;

    for (size_t i = 0; i < size; i++)
        printf("%lld ", array[i]);

    printf("\n");

    return STATUS_OK;
}


Status check_number(const char *str, long long *num){
    if (str == NULL || num == NULL)
        return STATUS_NULL_ARGUMENT;

    if (str[0] == '\0')
        return STATUS_EMPTY_STRING;

    int i = 0;
    int sign = 1;

    if (str[i] == '+' || str[i] == '-') {
        if (str[i] == '-')
            sign = -1;

        i++;

        if (str[i] == '\0')
            return STATUS_NO_DIGITS;
    }

    unsigned long long value = 0;

    unsigned long long limit;

    if (sign == -1)
        limit = (unsigned long long)LLONG_MAX + 1;
    else
        limit = LLONG_MAX;

    for (; str[i] != '\0'; i++) {

        if (str[i] < '0' || str[i] > '9')
            return STATUS_INVALID_NUMBER;

        unsigned long long digit = str[i] - '0';

        if (value > (limit - digit) / 10)
            return STATUS_OVERFLOW;

        value = value * 10 + digit;
    }

    if (sign == -1) {
        if (value == (unsigned long long)LLONG_MAX + 1)
            *num = LLONG_MIN;
        else
            *num = -value;
    } else {
        *num = value;
    }

    return STATUS_OK;
}

void print_number_error(Status status){
    switch (status) {

        case STATUS_EMPTY_STRING:
            printf("Ошибка: число не передано.\n");
            break;

        case STATUS_NO_DIGITS:
            printf("Ошибка: после знака нет цифр.\n");
            break;

        case STATUS_INVALID_NUMBER:
            printf("Ошибка: передано некорректное целое число.\n");
            break;

        case STATUS_OVERFLOW:
            printf("Ошибка: число выходит за диапазон long long.\n");
            break;

        case STATUS_NULL_ARGUMENT:
            printf("Внутренняя ошибка: недопустимый указатель.\n");
            break;

        default:
            printf("Ошибка: некорректное число.\n");
            break;
    }
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Ошибка: введите две границы диапазона.\n");
        return 1;
    }

    long long a, b;
    Status st_num_a = check_number(argv[1], &a);
    Status st_num_b = check_number(argv[2], &b);

    if (st_num_a != STATUS_OK){
        print_number_error(st_num_a);
        return 1;
    }

    if (st_num_b != STATUS_OK){
        print_number_error(st_num_b);
        return 1;
    }

    if (a > b){
        printf("Ошибка: левая граница больше правой.\n");
        return 1;
    }

    if ((long long)b - a + 1 > RAND_MAX) {
        printf("Ошибка: слишком большой диапазон.\n");
        return 1;
    }

    long long array[ARRAY_SIZE];

    srand((unsigned int)time(NULL));

    Status status = fill_array(array, ARRAY_SIZE, a, b);

    if (status != STATUS_OK) {
        printf("Ошибка заполнения массива.\n");
        return 1;
    }

    printf("Исходный массив:\n");
    print_array(array, ARRAY_SIZE);

    status = swap_min_max(array, ARRAY_SIZE);

    if (status != STATUS_OK) {
        printf("Ошибка обработки массива.\n");
        return 1;
    }

    printf("После обмена минимума и максимума:\n");
    print_array(array, ARRAY_SIZE);

    return 0;
}