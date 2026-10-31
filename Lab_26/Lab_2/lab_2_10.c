#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_EPS,
    STATUS_INVALID_DEGREE,
    STATUS_INVALID_NUMBER,
    STATUS_MEMORY_ERROR
} Status;

Status shift_polynomial(double eps, double a, double **result, int degree, ...){
    if (result == NULL)
        return STATUS_NULL_ARGUMENT;
    if (eps <= 0.0)
        return STATUS_INVALID_EPS;
    if (!isfinite(a))
        return STATUS_INVALID_NUMBER;
    if (degree < 0 || (size_t)degree > (size_t)-1 / sizeof(double) - 1)
        return STATUS_INVALID_DEGREE;

    size_t count = (size_t)degree + 1;

    double *c = malloc(count * sizeof(double));
    if (c == NULL)
        return STATUS_MEMORY_ERROR;

    va_list coeffs;
    va_start(coeffs, degree);
    for (size_t i = 0; i < count; i++){
        c[i] = va_arg(coeffs, double);
        if (!isfinite(c[i])){
            va_end(coeffs);
            free(c);
            return STATUS_INVALID_NUMBER;
        }
    }
    va_end(coeffs);

    /* полная схема Горнера */
    for (size_t i = 0; i < count; i++)
        for (size_t j = count - 1; j > i; j--)
            c[j - 1] += a * c[j];

    for (size_t i = 0; i < count; i++){
        if (!isfinite(c[i])){
            free(c);
            return STATUS_INVALID_NUMBER;
        }
        if (fabs(c[i]) <= eps)
            c[i] = 0.0;
    }

    *result = c;
    return STATUS_OK;
}

void print_status_error(Status status){
    switch (status){
        case STATUS_NULL_ARGUMENT:  printf("Внутренняя ошибка: нулевой указатель.\n"); break;
        case STATUS_INVALID_EPS:    printf("Ошибка: эпсилон должен быть положительным.\n"); break;
        case STATUS_INVALID_DEGREE: printf("Ошибка: некорректная степень многочлена.\n"); break;
        case STATUS_INVALID_NUMBER: printf("Ошибка: некорректное или слишком большое число.\n"); break;
        case STATUS_MEMORY_ERROR:   printf("Ошибка: не удалось выделить память.\n"); break;
        default:                    printf("Ошибка.\n"); break;
    }
}

void print_polynomial(const char *name, const double *g, size_t count, double a){
    printf("%s, разложение по степеням (x - (%g)):\n  ", name, a);
    for (size_t i = 0; i < count; i++){
        printf("%+g", g[i]);
        if (i > 0)
            printf("*(x-(%g))^%zu", a, i);
        printf(" ");
    }
    printf("\n");
}

int main(void){
    const double eps = 0.001;
    double *g = NULL;
    Status st;

    /* x^2 по степеням (x - 1): 1 + 2(x-1) + (x-1)^2 */
    st = shift_polynomial(eps, 1.0, &g, 2, 0.0, 0.0, 1.0);
    if (st == STATUS_OK){ 
        print_polynomial("x^2", g, 3, 1.0); 
        free(g); 
    }
    else print_status_error(st);

    /* 1 + 2x + 3x^2 + x^3 по степеням (x - 2): 23 + 23(x-2) + 9(x-2)^2 + (x-2)^3 */
    st = shift_polynomial(eps, 2.0, &g, 3, 1.0, 2.0, 3.0, 1.0);
    if (st == STATUS_OK){ 
        print_polynomial("1+2x+3x^2+x^3", g, 4, 2.0); 
        free(g); 
    }
    else print_status_error(st);

    /* a = 0: многочлен не меняется */
    st = shift_polynomial(eps, 0.0, &g, 2, 5.0, -1.0, 4.0);
    if (st == STATUS_OK){ 
        print_polynomial("5-x+4x^2", g, 3, 0.0); 
        free(g); 
    }
    else print_status_error(st);

    /* константа */
    st = shift_polynomial(eps, 7.0, &g, 0, 3.0);
    if (st == STATUS_OK){ print_polynomial("3", g, 1, 7.0); free(g); }
    else print_status_error(st);

    /* ошибки */
    st = shift_polynomial(eps, 1.0, &g, -1);
    if (st != STATUS_OK) print_status_error(st);

    st = shift_polynomial(0.0, 1.0, &g, 1, 1.0, 1.0);
    if (st != STATUS_OK) print_status_error(st);

    return 0;
}