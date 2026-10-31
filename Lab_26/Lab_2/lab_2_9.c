#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#define MAX_DEN 1000000ULL      /* максимальный знаменатель при подборе дроби */
#define EPS_MAX 1e-12           /* eps больше этого значения даёт ложные дроби */

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_BASE,
    STATUS_INVALID_EPS,
    STATUS_INVALID_COUNT,
    STATUS_INVALID_NUMBER,    /* не число или не из (0; 1) */
    STATUS_NO_FRACTION,       /* не удалось подобрать дробь со знаменателем <= MAX_DEN */
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

/* Одно число: *finite = 1, если x в системе base записывается конечно */
Status is_finite_fraction(double x, int base, double eps, int *finite){
    if (finite == NULL)
        return STATUS_NULL_ARGUMENT;
    if (base < 2 || base > 36)
        return STATUS_INVALID_BASE;
    if (!isfinite(eps) || eps <= 0.0 || eps > EPS_MAX)
        return STATUS_INVALID_EPS;
    if (!isfinite(x) || x <= 0.0 || x >= 1.0)
        return STATUS_INVALID_NUMBER;

    unsigned long long num = 0;
    unsigned long long den = 0;

    /* подбираем p/q, совпадающую с x с точностью eps */
    for (unsigned long long q = 2; q <= MAX_DEN; q++){
        double p = floor(x * (double)q + 0.5);
        if (p >= 1.0 && fabs(x - p / (double)q) <= eps){
            num = (unsigned long long)p;
            den = q;
            break;
        }
    }
    if (den == 0)
        return STATUS_NO_FRACTION;

    den /= gcd(num, den);                      /* сократили дробь, den >= 2 */

    /* убираем из знаменателя все простые множители, общие с base */
    unsigned long long g = gcd(den, (unsigned long long)base);
    while (g > 1){
        while (den % g == 0)
            den /= g;
        g = gcd(den, (unsigned long long)base);
    }

    *finite = (den == 1);
    return STATUS_OK;
}

/*
 * base    - основание системы счисления
 * eps     - точность
 * count   - сколько чисел типа double передано после results
 * results - массив из count значений (1 - конечное, 0 - бесконечное),
 *           память выделяется внутри, освобождает вызывающий код
 */
Status finite_representation(int base, double eps, size_t count, int **results, ...){
    if (results == NULL)
        return STATUS_NULL_ARGUMENT;
    if (base < 2 || base > 36)
        return STATUS_INVALID_BASE;
    if (!isfinite(eps) || eps <= 0.0 || eps > EPS_MAX)
        return STATUS_INVALID_EPS;
    if (count == 0 || count > (size_t)-1 / sizeof(**results))
        return STATUS_INVALID_COUNT;

    int *res = malloc(count * sizeof(*res));
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
        case STATUS_INVALID_EPS:    printf("Ошибка: эпсилон должен быть конечным числом из (0; 1e-12].\n"); break;
        case STATUS_INVALID_COUNT:  printf("Ошибка: некорректное количество чисел.\n"); break;
        case STATUS_INVALID_NUMBER: printf("Ошибка: число должно быть вещественным из интервала (0; 1).\n"); break;
        case STATUS_NO_FRACTION:    printf("Ошибка: не удалось представить число обыкновенной дробью с малым знаменателем.\n"); break;
        case STATUS_MEMORY_ERROR:   printf("Ошибка: не удалось выделить память.\n"); break;
        case STATUS_OK:             break;
        default:                    printf("Ошибка.\n"); break;
    }
}

/* вывод отделён от вычислений */
void print_results(int base, const double *values, const int *results, size_t count){
    printf("Основание %d:\n", base);
    for (size_t i = 0; i < count; i++)
        printf("  %.6g: %s\n", values[i], results[i] ? "конечное" : "бесконечное");
}

int main(void){
    const double eps = 1e-12;
    int *res = NULL;
    Status st;

    const double v1[] = {0.5, 0.25, 0.1, 1.0 / 3.0};
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

    /* ожидаемые ошибки: должен напечататься текст каждой */
    st = finite_representation(1, eps, 1, &res, 0.5);
    if (st != STATUS_OK) print_status_error(st);

    st = finite_representation(10, eps, 2, &res, 0.5, 1.5);
    if (st != STATUS_OK) print_status_error(st);

    st = finite_representation(10, eps, 1, &res, NAN);
    if (st != STATUS_OK) print_status_error(st);

    st = finite_representation(10, eps, 1, &res, 0.0);
    if (st != STATUS_OK) print_status_error(st);

    st = finite_representation(10, 0.0, 1, &res, 0.5);
    if (st != STATUS_OK) print_status_error(st);

    st = finite_representation(10, 0.1, 1, &res, 0.5);
    if (st != STATUS_OK) print_status_error(st);

    st = finite_representation(10, eps, 0, &res);
    if (st != STATUS_OK) print_status_error(st);

    st = finite_representation(10, eps, 1, &res, 0.1234567891234);
    if (st != STATUS_OK) print_status_error(st);

    st = finite_representation(10, eps, 1, (int **)NULL, 0.5);
    if (st != STATUS_OK) print_status_error(st);

    return 0;
}

