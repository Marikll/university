#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdarg.h>
#include <stdlib.h>

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_ARGUMENT,
    STATUS_INVALID_EPSILON,
    STATUS_INVALID_BASE,
    STATUS_OVERFLOW,
    STATUS_NO_ROOT,
    STATUS_INVALID_COUNT,
    STATUS_NONCONVEX,
    STATUS_DEGENERATE,
    STATUS_MAX_ITERATIONS
}Status;

Status function_1(const int count, const double eps, ...){
    if (count < 3)
        return STATUS_INVALID_COUNT;

    if (!isfinite(eps) || eps <= 0)
        return STATUS_INVALID_EPSILON;

    va_list coordinate;
    va_start(coordinate, eps);
    double first_x = va_arg(coordinate, double);
    double first_y = va_arg(coordinate, double);
    double prev_x = first_x;
    double prev_y = first_y;
    double cur_x = va_arg(coordinate, double);
    double cur_y = va_arg(coordinate, double);
    double second_x = cur_x;
    double second_y = cur_y;
    double next_x;
    double next_y;
    int sign_prev;
    int sign_next;

    for (int i = 0; i < count; i++){
        if (i == count - 2){
            next_x = first_x;
            next_y = first_y;
        } else if (i == count - 1){
            next_x = second_x;
            next_y = second_y;
        } else {
            next_x = va_arg(coordinate, double);
            next_y = va_arg(coordinate, double);
        }
        double vec_product = (cur_x - prev_x) * (next_y - cur_y) - (cur_y - prev_y) * (next_x - cur_x);
        if (fabs(vec_product) < eps) {
            va_end(coordinate);
            return STATUS_DEGENERATE;
        }

        if (vec_product > eps)
            sign_next = 1;
        else 
            sign_next = -1;

        if (i == 0)
            sign_prev = sign_next;

        if (sign_next != sign_prev){
            va_end(coordinate);
            return STATUS_NONCONVEX;
        }
        
        sign_prev = sign_next;
        prev_x = cur_x;
        prev_y = cur_y;
        cur_x = next_x;
        cur_y = next_y;
    }
    va_end(coordinate);
    return STATUS_OK;
}

Status function_2(double x, int n, double *res, ...){
    if (res == NULL)
        return STATUS_NULL_ARGUMENT;
    
    if (n < 0)
        return STATUS_INVALID_ARGUMENT;

    va_list coefficients;
    va_start(coefficients, res);

    double coefficient = va_arg(coefficients, double);
    double value = coefficient;

    for (int i = 1; i <= n; i++){
        coefficient = va_arg(coefficients, double);
        if (!isfinite(value)) {
            va_end(coefficients);
            return STATUS_OVERFLOW;
        }
        value = value * x + coefficient;
        if (!isfinite(value)) {
            va_end(coefficients);
            return STATUS_OVERFLOW;
        }
    }
    va_end(coefficients);
    *res = value;
    return STATUS_OK;
}

#include <limits.h>

Status conversion_num(const char *s, int base, int *digits, long long *num)
{
    if (s == NULL || digits == NULL || num == NULL)
        return STATUS_NULL_ARGUMENT;

    if (base < 2 || base > 36)
        return STATUS_INVALID_BASE;

    if (s[0] == '\0')
        return STATUS_INVALID_ARGUMENT;

    long long value = 0;
    int count = 0;

    for (int i = 0; s[i] != '\0'; i++) {

        int digit;

        if (s[i] >= '0' && s[i] <= '9') {
            digit = s[i] - '0';
        }
        else if (s[i] >= 'A' && s[i] <= 'Z') {
            digit = s[i] - 'A' + 10;
        }
        else if (s[i] >= 'a' && s[i] <= 'z') {
            digit = s[i] - 'a' + 10;
        }
        else {
            return STATUS_INVALID_ARGUMENT;
        }

        if (digit >= base)
            return STATUS_INVALID_ARGUMENT;

        if (value > (LLONG_MAX - digit) / base)
            return STATUS_OVERFLOW;

        value = value * base + digit;
        count++;
    }

    *num = value;
    *digits = count;

    return STATUS_OK;
}

Status function_3(const int *count, int *res, const int *base, ...){
    if (count == NULL || res == NULL || base == NULL)
        return STATUS_NULL_ARGUMENT;

    if (*count < 1)
        return STATUS_INVALID_COUNT;

    if (*base < 2 || *base > 36)
        return STATUS_INVALID_BASE;

    va_list numbers;
    va_start(numbers, base);

    for (int i = 0; i < *count; i++) {

        const char *str = va_arg(numbers, const char *);

        long long num = 0;
        int digits = 0;

        Status status = conversion_num(str, *base, &digits, &num);

        if (status != STATUS_OK) {
            va_end(numbers);
            return status;
        }

        if (num != 0 && num > LLONG_MAX / num) {
            va_end(numbers);
            return STATUS_OVERFLOW;
        }

        long long square = num * num;

        long long divider = 1;
        int divider_overflow = 0;

        for (int j = 0; j < digits; j++) {
            if (divider > LLONG_MAX / *base) {
                divider_overflow = 1;
                break;
            }
            divider *= *base;
        }

        long long left;
        long long right;

        if (divider_overflow) {
            left = 0;
            right = square;
        } else {
            left = square / divider;
            right = square % divider;
        }

        if (left + right == num)
            res[i] = 1;
        else
            res[i] = 0;
    }

    va_end(numbers);
    return STATUS_OK;
}

Status function_4(double *res, const int count, ...){
    if (res == NULL)
        return STATUS_NULL_ARGUMENT;

    va_list numbers;
    va_start(numbers, count);

     if (count <= 0){
        va_end(numbers);
        return STATUS_INVALID_COUNT;
     }

    double log_sum = 0.0;

    for (int i = 0; i < count; i++){
        double num = va_arg(numbers, double);

        if (!isfinite(num)){
            va_end(numbers);
            return STATUS_OVERFLOW;
        }

        if (num < 0.0){
            va_end(numbers);
            return STATUS_INVALID_ARGUMENT;
        }

        if (num == 0.0){
            *res = 0.0;
            va_end(numbers);
            return STATUS_OK;
        }

        log_sum += log(num);
    }

    va_end(numbers);
    *res = exp(log_sum/count);

    return STATUS_OK;
}

Status function_5(const double x, const long long n, double *res){
    if (res == NULL)
        return STATUS_NULL_ARGUMENT;

    if (!isfinite(x))
        return STATUS_INVALID_ARGUMENT;

    if (n == 0) {
        *res = 1.0;
        return STATUS_OK;
    }

    if (x == 0.0 && n < 0)
        return STATUS_INVALID_ARGUMENT;

    if (n < 0) {
        if (n == LLONG_MIN)
            return STATUS_OVERFLOW;
        double temp;
        Status status;

        unsigned long long power = -(unsigned long long)n;

        status = function_5(x, (long long)power, &temp);

        if (status != STATUS_OK)
            return status;

        if (temp == 0.0)
            return STATUS_OVERFLOW;

        *res = 1.0 / temp;

        if (!isfinite(*res))
            return STATUS_OVERFLOW;

        return STATUS_OK;
    }

    if (n == 1) {
        *res = x;
        return STATUS_OK;
    }

    if (n % 2 == 0) {
        double temp;
        Status status = function_5(x, n / 2, &temp);

        if (status != STATUS_OK)
            return status;

        *res = temp * temp;
    }
    else {
        double temp;
        Status status = function_5(x, (n - 1)/2, &temp);

        if (status != STATUS_OK)
            return status;

        *res = temp * temp;

        if (!isfinite(*res))
            return STATUS_OVERFLOW;
        *res *= x;
    }

    if (!isfinite(*res))
        return STATUS_OVERFLOW;

    return STATUS_OK;
}

Status function_6(const double *left, const double *right, const double *eps, double (*function)(double), double *res)
{
    if (left == NULL || right == NULL || eps == NULL || function == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    if (!isfinite(*eps) || *eps <= 0)
        return STATUS_INVALID_EPSILON;

    if (*left >= *right)
        return STATUS_INVALID_ARGUMENT;

    double a = *left;
    double b = *right;

    double fa = function(a);
    double fb = function(b);

    if (!isfinite(fa) || !isfinite(fb))
        return STATUS_INVALID_ARGUMENT;

    if (fabs(fa) < *eps) {
        *res = a;
        return STATUS_OK;
    }

    if (fabs(fb) < *eps) {
        *res = b;
        return STATUS_OK;
    }

    if ((fa > 0 && fb > 0) || (fa < 0 && fb < 0))
        return STATUS_NO_ROOT;

    int max_iterations = 10000000;
    int iterations = 0;

    while (fabs(b - a) > *eps) {
        iterations++;
        if (iterations >= max_iterations)
            return STATUS_MAX_ITERATIONS;

        double c = a + (b - a) / 2.0;
        double fc = function(c);

        if (!isfinite(fc))
            return STATUS_INVALID_ARGUMENT;

        if (fabs(fc) < *eps) {
            *res = c;
            return STATUS_OK;
        }

        if ((fa < 0 && fc > 0) || (fa > 0 && fc < 0)) {
            b = c;
            fb = fc;
        } else {
            a = c;
            fa = fc;
        }
    }

    *res = (a + b) / 2.0;
    return STATUS_OK;
}


double equation_1(const double x){
    return x * x - 2.0;
}

double equation_2(const double x){
    return x * x * x - x - 2.0;
}

double equation_3(const double x){
    return sin(x) - 0.5;
}

int main(int argc, char *argv[]){
    if (argc != 2) {
        printf("Использование: ./a.out <номер функции 1-6>\n");
        return 1;
    }

    int choice;

    if (argv[1][0] < '1' || argv[1][0] > '6' || argv[1][1] != '\0') {
        printf("Ошибка: номер функции должен быть от 1 до 6.\n");
        return 1;
    }

    choice = argv[1][0] - '0';

    switch (choice) {

        case 1: {
            double eps = 1e-9;
            int count = 4;

            Status status = function_1(count, eps, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0);
            // Status status = function_1(count, eps, 0.0, 0.0, 2.0, 0.0, 1.0, 1.0, 2.0, 2.0, 0.0, 2.0);

            printf("Функция 1: ");

            if (status == STATUS_OK)
                printf("многоугольник выпуклый.\n");
            else if (status == STATUS_NONCONVEX)
                printf("многоугольник невыпуклый.\n");
            else if (status == STATUS_DEGENERATE)
                printf("многоугольник вырожден.\n");
            else
                printf("ошибка.\n");

            break;
        }

        case 2: {
            double x = 2.0;
            int n = 3;
            double result;

            Status status = function_2(x, n, &result, 5.0, 4.0, 3.0, 2.0);

            printf("Функция 2: ");

            if (status == STATUS_OK)
                printf("P(%.2f) = %.15g\n", x, result);
            else
                printf("ошибка.\n");

            break;
        }

        case 3: {
            int base = 10;
            int count = 4;
            int *result = malloc(count * sizeof(*result));

            if (result == NULL) {
                printf("Ошибка выделения памяти.\n");
                return 1;
            }

            Status status = function_3(&count, result, &base, "9", "10", "45", "297");

            if (status == STATUS_OK) {
                printf("Функция 3:\n");

                for (int i = 0; i < count; i++)
                    printf("%s\n", result[i] ? "Капрекара" : "не Капрекара");
            }
            else {
                printf("Функция 3: ошибка, статус %d.\n", status);
            }

            free(result);

            break;
        }

        case 4: {
            int count = 3;
            double result;

            Status status = function_4(&result, count, 2.0, 8.0, 4.0);

            printf("Функция 4: ");

            if (status == STATUS_OK)
                printf("среднее геометрическое = %.15g\n", result);
            else
                printf("ошибка.\n");

            break;
        }

        case 5: {
            double x = 2.0;
            long long n = -3;
            double result;

            Status status = function_5(x, n, &result);

            printf("Функция 5: ");

            if (status == STATUS_OK)
                printf("%.2f^%lld = %.15g\n", x, n, result);
            else
                printf("ошибка.\n");

            break;
        }

        case 6: {
            double eps = 1e-8;
            double left;
            double right;
            double result;
            Status status;

            left = 1.0;
            right = 2.0;

            status = function_6(&left, &right, &eps, equation_1, &result);

            printf("Функция 6, x^2 - 2 = 0:\n");

            if (status == STATUS_OK)
                printf("корень = %.15g\n", result);
            else
                printf("ошибка, статус %d.\n", status);


            left = 1.0;
            right = 2.0;

            status = function_6(&left, &right, &eps, equation_2, &result);

            printf("Функция 6, x^3 - x - 2 = 0:\n");

            if (status == STATUS_OK)
                printf("корень = %.15g\n", result);
            else
                printf("ошибка, статус %d.\n", status);


            left = 0.0;
            right = 1.0;

            status = function_6(&left, &right, &eps, equation_3, &result);

            printf("Функция 6, sin(x) - 0.5 = 0:\n");

            if (status == STATUS_OK)
                printf("корень = %.15g\n", result);
            else
                printf("ошибка, статус %d.\n", status);

            break;
        }
    }

    return 0;
}

