#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#define MAX_DIGITS 9   

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_BASE,
    STATUS_INVALID_EPS,
    STATUS_INVALID_COUNT,
    STATUS_INVALID_NUMBER,  
    STATUS_NOT_DECIMAL,       
    STATUS_MEMORY_ERROR
} Status;

unsigned long long gcd(unsigned long long a, unsigned long long b){
    while (b != 0){
        unsigned long long t = a % b;
        a = b;
        b = t;
    }
    return a;
}

Status is_finite_fraction(double x, int base, double eps, int *finite){
    if (finite == NULL)
        return STATUS_NULL_ARGUMENT;
    if (base < 2 || base > 36)
        return STATUS_INVALID_BASE;
    if (eps <= 0.0)
        return STATUS_INVALID_EPS;
    if (x <= eps || x >= 1.0 - eps)
        return STATUS_INVALID_NUMBER;

    unsigned long long den = 1;
    double scaled = x;
    int found = 0;

    for (int k = 0; k < MAX_DIGITS; k++){
        scaled *= 10.0;
        den *= 10;
        if (fabs(scaled - floor(scaled + 0.5)) <= eps){
            found = 1;
            break;
        }
    }
    if (!found)
        return STATUS_NOT_DECIMAL;

    unsigned long long num = (unsigned long long)floor(scaled + 0.5);
    if (num == 0)
        return STATUS_INVALID_NUMBER;

    den /= gcd(num, den);                 

    unsigned long long g = gcd(den, (unsigned long long)base);
    while (g > 1){
        while (den % g == 0)
            den /= g;
        g = gcd(den, (unsigned long long)base);
    }

    *finite = (den == 1);
    return STATUS_OK;
}

Status finite_representation(int base, double eps, size_t count, int **results, ...){
    if (results == NULL)
        return STATUS_NULL_ARGUMENT;
    if (base < 2 || base > 36)
        return STATUS_INVALID_BASE;
    if (eps <= 0.0)
        return STATUS_INVALID_EPS;
    if (count == 0 || count > (size_t)-1 / sizeof(int))
        return STATUS_INVALID_COUNT;

    int *res = malloc(count * sizeof(int));
    if (res == NULL)
        return STATUS_MEMORY_ERROR;

    va_list numbers;
    va_start(numbers, results);

    Status st = STATUS_OK;
    for (size_t i = 0; i < count && st == STATUS_OK; i++){
        double x = va_arg(numbers, double);
        st = is_finite_fraction(x, base, eps, &res[i]);
    }
    va_end(numbers);

    if (st != STATUS_OK){
        free(res);
        return st;
    }

    *results = res;
    return STATUS_OK;
}

void print_status_error(Status status){
    switch (status){
        case STATUS_NULL_ARGUMENT:  printf("Внутренняя ошибка: нулевой указатель.\n"); break;
        case STATUS_INVALID_BASE:   printf("Ошибка: основание должно быть в диапазоне [2, 36].\n"); break;
        case STATUS_INVALID_EPS:    printf("Ошибка: эпсилон должен быть положительным.\n"); break;
        case STATUS_INVALID_COUNT:  printf("Ошибка: некорректное количество чисел.\n"); break;
        case STATUS_INVALID_NUMBER: printf("Ошибка: число должно быть из интервала (0; 1).\n"); break;
        case STATUS_NOT_DECIMAL:    printf("Ошибка: число не является конечной десятичной дробью.\n"); break;
        case STATUS_MEMORY_ERROR:   printf("Ошибка: не удалось выделить память.\n"); break;
        default:                    printf("Ошибка.\n"); break;
    }
}

void print_results(int base, const double *values, const int *results, size_t count){
    printf("Основание %d:\n", base);
    for (size_t i = 0; i < count; i++)
        printf("  %.6g: %s\n", values[i], results[i] ? "конечное" : "бесконечное");
}

int main(void){
    const double eps = 1e-9;
    int *res = NULL;
    Status st;

    const double v1[] = {0.5, 0.25, 0.1, 0.2};
    st = finite_representation(10, eps, 4, &res, v1[0], v1[1], v1[2], v1[3]);
    if (st == STATUS_OK){ print_results(10, v1, res, 4); free(res); }
    else print_status_error(st);

    st = finite_representation(3, eps, 4, &res, v1[0], v1[1], v1[2], v1[3]);
    if (st == STATUS_OK){ print_results(3, v1, res, 4); free(res); }
    else print_status_error(st);

    st = finite_representation(2, eps, 4, &res, v1[0], v1[1], v1[2], v1[3]);
    if (st == STATUS_OK){ print_results(2, v1, res, 4); free(res); }
    else print_status_error(st);

    const double v2[] = {0.5, 0.2, 0.125};
    st = finite_representation(6, eps, 3, &res, v2[0], v2[1], v2[2]);
    if (st == STATUS_OK){ print_results(6, v2, res, 3); free(res); }
    else print_status_error(st);

    st = finite_representation(15, eps, 3, &res, v2[0], v2[1], v2[2]);
    if (st == STATUS_OK){ print_results(15, v2, res, 3); free(res); }
    else print_status_error(st);

    st = finite_representation(1, eps, 1, &res, 0.5);
    if (st != STATUS_OK) print_status_error(st);

    st = finite_representation(10, eps, 2, &res, 0.5, 1.5);
    if (st != STATUS_OK) print_status_error(st);

    return 0;
}

