#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <stddef.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_NULL_ARGUMENT,
    STATUS_EMPTY_STRING,
    STATUS_INVALID_FLAG,
    STATUS_INVALID_NUMBER,
    STATUS_NO_DIGITS,
    STATUS_OVERFLOW,
    STATUS_INVALID_VALUE,
    STATUS_NO_RESULT
} Status;

Status check_flag(const char *str, char *flag){
    if (str == NULL || flag == NULL)
        return STATUS_NULL_ARGUMENT;

    if (str[0] == '\0')
        return STATUS_EMPTY_STRING;

    if (str[0] != '-' && str[0] != '/')
        return STATUS_INVALID_FLAG;

    if (str[1] == '\0')
        return STATUS_INVALID_FLAG;

    if (str[2] != '\0')
        return STATUS_INVALID_FLAG;

    switch (str[1]) {
        case 'h':
        case 'p':
        case 's':
        case 'e':
        case 'a':
        case 'f':
            *flag = str[1];
            return STATUS_OK;

        default:
            return STATUS_INVALID_FLAG;
    }
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

Status flag_h(const long long *x, long long numbers[], int *count){
    if (x == NULL || numbers == NULL || count == NULL)
        return STATUS_NULL_ARGUMENT;

    if (*x == 0)
        return STATUS_INVALID_VALUE;

    *count = 0;

    for (long long i = 1; i <= 100; i++) {

        if (i % *x == 0) {
            numbers[*count] = i;
            (*count)++;
        }
    }

    if (*count == 0)
        return STATUS_NO_RESULT;

    return STATUS_OK;
}

Status flag_p(const long long *x, bool *is_prime, bool *is_composite){
    if (x == NULL ||
        is_prime == NULL ||
        is_composite == NULL)
        return STATUS_NULL_ARGUMENT;

    *is_prime = false;
    *is_composite = false;

    if (*x < 2)
        return STATUS_INVALID_VALUE;

    if (*x == 2) {
        *is_prime = true;
        return STATUS_OK;
    }

    if (*x % 2 == 0) {
        *is_composite = true;
        return STATUS_OK;
    }

    for (long long i = 3; i <= *x / i; i += 2) {

        if (*x % i == 0) {
            *is_composite = true;
            return STATUS_OK;
        }
    }

    *is_prime = true;
    return STATUS_OK;
}

Status flag_s(const long long *x, char numbers[], size_t capacity, size_t *length){
    if (x == NULL || numbers == NULL || length == NULL)
        return STATUS_NULL_ARGUMENT;

    if (capacity < 2)
        return STATUS_INVALID_VALUE;

    if (*x < 0)
        return STATUS_INVALID_VALUE;

    unsigned long long value = *x;

    char digits[] = "0123456789ABCDEF";

    size_t pos = capacity - 1;
    numbers[pos] = '\0';

    if (value == 0) {
        if (pos == 0)
            return STATUS_INVALID_VALUE;
        numbers[--pos] = '0';
    } else {
        while (value > 0) {
            if (pos == 0)
                return STATUS_OVERFLOW;

            numbers[--pos] = digits[value % 16];

            value /= 16;
        }
    }

    size_t result_length = capacity - pos - 1;

    for (size_t i = 0; i < result_length; i++)
        numbers[i] = numbers[pos + i];

    numbers[result_length] = '\0';

    *length = result_length;

    return STATUS_OK;
}

Status flag_e(const long long *x, long long table[10][10]){
    if (x == NULL || table == NULL)
        return STATUS_NULL_ARGUMENT;

    if (*x < 1 || *x > 10)
        return STATUS_INVALID_VALUE;

    for (int base = 1; base <= 10; base++) {

        long long result = 1;

        for (int power = 1; power <= *x; power++) {

            result *= base;

            table[base - 1][power - 1] = result;
        }
    }

    return STATUS_OK;
}

Status flag_a(const long long *x, long long *result){
    if (x == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;

    if (*x < 1)
        return STATUS_INVALID_VALUE;

    unsigned long long a = *x;

    unsigned long long b = a + 1;

    if (a % 2 == 0)
        a /= 2;
    else
        b /= 2;

    if (a > LLONG_MAX / b)
        return STATUS_OVERFLOW;
    *result = a * b;

    return STATUS_OK;
}

Status flag_f(const long long *x, long long *result){ 
    if (x == NULL || result == NULL) 
        return STATUS_NULL_ARGUMENT; 
    
    if (*x < 0) 
        return STATUS_INVALID_VALUE; 
    *result = 1; 

    for (long long i = 2; i <= *x; i++){ 
        if (*result > LLONG_MAX / i) 
            return STATUS_OVERFLOW; 
        *result *= i; 
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

void print_flag_error(Status status){
    switch (status) {

        case STATUS_EMPTY_STRING:
            printf("Ошибка: флаг не передан.\n");
            break;

        case STATUS_INVALID_FLAG:
            printf("Ошибка: некорректный флаг. "
                   "Допустимы: h, p, s, e, a.\n");
            break;

        case STATUS_NULL_ARGUMENT:
            printf("Внутренняя ошибка: недопустимый указатель.\n");
            break;

        default:
            printf("Ошибка: некорректный флаг.\n");
            break;
    }
}

int main(int argc, char *argv[]){
    if (argc != 3) {
        printf("Ошибка: необходимо передать число и флаг.\n");
        return 1;
    }

    long long number1 = 0;
    long long number2 = 0;

    char flag1 = '\0';
    char flag2 = '\0';

    Status status_number1 = check_number(argv[1], &number1);

    Status status_number2 = check_number(argv[2], &number2);

    Status status_flag1 = check_flag(argv[1], &flag1);

    Status status_flag2 = check_flag(argv[2], &flag2);

    long long x = 0;
    char flag = '\0';

    if (status_number1 == STATUS_OK && status_flag2 == STATUS_OK) {
        x = number1;
        flag = flag2;
    } else if (status_flag1 == STATUS_OK && status_number2 == STATUS_OK) {
        x = number2;
        flag = flag1;
    } else if (status_number1 == STATUS_OK && status_number2 == STATUS_OK) {

        printf("Ошибка: переданы два числа. Введите число и флаг.\n");
        return 1;
    } else if (status_flag1 == STATUS_OK && status_flag2 == STATUS_OK) {

        printf("Ошибка: переданы два флага. Введите число и флаг.\n");
        return 1;
    } else if (status_number1 == STATUS_OK) {
        print_flag_error(status_flag2);
        return 1;
    } else if (status_number2 == STATUS_OK) {

        print_flag_error(status_flag1);
        return 1;
    } else if (status_flag1 == STATUS_OK) {
        print_number_error(status_number2);
        return 1;
    } else if (status_flag2 == STATUS_OK) {
        print_number_error(status_number1);
        return 1;
    } else {
        printf("Ошибка: введите число и корректный флаг.\n");
        return 1;
    }

    switch (flag) {

        case 'h': {
            long long numbers[100];
            int count = 0;

            Status status = flag_h(&x, numbers, &count);

            if (status == STATUS_INVALID_VALUE) {
                printf("Ошибка: для флага -h число x не должно быть равно нулю.\n");
                return 1;
            }

            if (status == STATUS_NO_RESULT) {
                printf("Числа, кратные %lld, отсутствуют в диапазоне 1..100.\n", x);
                return 0;
            }

            if (status != STATUS_OK) {
                printf("Ошибка при выполнении флага -h.\n");
                return 1;
            }

            for (int i = 0; i < count; i++)
                printf("%lld ", numbers[i]);

            printf("\n");

            break;
        }


        case 'p': {

            bool is_prime = false;
            bool is_composite = false;

            Status status = flag_p(&x, &is_prime, &is_composite);

            if (status == STATUS_INVALID_VALUE) {
                printf("Число %lld не является ни простым, ни составным.\n", x);
                return 0;
            }

            if (status != STATUS_OK) {
                printf("Ошибка при выполнении флага -p.\n");
                return 1;
            }

            if (is_prime)
                printf("Число %lld является простым.\n", x);
            else if (is_composite)
                printf("Число %lld является составным.\n", x);

            break;
        }


        case 's': {
            char numbers[17];
            size_t length = 0;

            Status status = flag_s(&x, numbers, sizeof(numbers), &length);

            if (status == STATUS_INVALID_VALUE) {
                printf("Ошибка: для флага -s подходят только неотрицательные числа.\n");
                return 1;
            }

            if (status != STATUS_OK) {
                printf("Ошибка при выполнении флага -s.\n");
                return 1;
            }

            for (size_t i = 0; i < length; i++) {
                if (i > 0)
                    printf(" ");

                printf("%c", numbers[i]);
            }

            printf("\n");

            break;
        }


        case 'e': {

            long long table[10][10] = {0};

            Status status = flag_e(&x, table);

            if (status == STATUS_INVALID_VALUE) {
                printf("Ошибка: для флага -e x должно быть от 1 до 10.\n");
                return 1;
            }

            if (status != STATUS_OK) {
                printf("Ошибка при выполнении флага -e.\n");
                return 1;
            }

            printf("|  a^n  |");

            for (int power = 1; power <= x; power++) {
                printf("    %d    |", power);
            }

            printf("\n");

            for (int base = 1; base <= 10; base++) {

                printf("|  %d  |", base);

                for (int power = 1; power <= x; power++) {
                    printf("  %lld  |", table[base - 1][power - 1]);
                }
                printf("\n");
            }
            break;
        }


        case 'a': {
            long long result = 0;

            Status status = flag_a(&x, &result);

            if (status == STATUS_INVALID_VALUE) {
                printf("Ошибка: для флага -a x должно быть натуральным числом.\n");
                return 1;
            }

            if (status == STATUS_OVERFLOW) {
                printf("Ошибка: переполнение при вычислении суммы.\n");
                return 1;
            }

            if (status != STATUS_OK) {
                printf("Ошибка при выполнении флага -a.\n");
                return 1;
            }

            printf("%lld\n", result);
            break;
        }

        case 'f':{ 
            long long result = 0; 
            Status status = flag_f(&x, &result); 
            if (status == STATUS_INVALID_VALUE){
                printf("Ошибка: для флага -f x должно быть неотрицательным.\n"); 
                return 1; 
            } 
            if (status == STATUS_OVERFLOW){ 
                printf("Ошибка: переполнение при вычислении факториала.\n"); 
                return 1; 
            } 
            if (status != STATUS_OK){ 
                printf("Ошибка при выполнении флага -f.\n"); 
                return 1; 
            } 
            
            printf("%lld\n", result); 
            break; 
        }


        default:
            printf("Внутренняя ошибка: неизвестный флаг.\n");
            return 1;
    }
    return 0;
}





