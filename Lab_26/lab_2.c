#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_NUMBER,
    STATUS_NO_DIGITS,
    STATUS_OVERFLOW,
    STATUS_SIGNED_NUMBER,
    STATUS_EMPTY_STRING,
    STATUS_MACHINE_LIMIT,
    STATUS_MAX_ITERATIONS,
    STATUS_INVALID_OPERATION,
    STATUS_SMALL_NUMBER
} Status;

Status machine_epsilon(double *eps){
    if (eps == NULL)
        return STATUS_NULL_ARGUMENT;
    while ((1.0 + *eps/2.0) != 1.0)
        *eps /= 2.0;
    return STATUS_OK; 
}

double module_num(double num){
    if (num < 0)
        num = -num;
    return num;
}

Status e_series(const double *eps, double *res){
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double sum = 1;
    double term = 1;
    int n = 1;
    int iterators = 0;
    const int max_iterations = 1000000;

    while (module_num(term) > *eps){
        if (iterators > max_iterations)
            return STATUS_MAX_ITERATIONS;
        term /= n;
        if (sum == sum + term)
            return STATUS_MACHINE_LIMIT;
        sum += term;
        n++;
    }
    *res = sum;
    return STATUS_OK;
}

Status e_equation(const double *eps, double *res){
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;
    
    double current = 2.0;
    int iterators = 0;
    const int max_iterations = 1000000;
    double next;

    while (1){
        next = current * (2.0 - log(current));
        if (module_num(next - current) <= *eps){
            *res = next;
            return STATUS_OK;
        }
        if (current == next)
            return STATUS_MACHINE_LIMIT;
        current = next;
        iterators++;
        if (iterators > max_iterations)
            return STATUS_MAX_ITERATIONS;
    }
}

Status e_limit(const double *eps, double *res){
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double x_cur = 2.0;
    double x_next;
    int n = 2;
    int iterators = 0;
    const int max_iterations = 1000000;

    while (1){
        x_next = pow(1.0 + 1.0/n, n);
        if (module_num(x_next - x_cur) <= *eps){
            *res = x_next;
            return STATUS_OK;
        }
        if (x_next == x_cur)
            return STATUS_MACHINE_LIMIT;
        x_cur = x_next;
        n++;
        iterators++;
        if (iterators > max_iterations)
            return STATUS_MAX_ITERATIONS;
    }
}

Status pi_series(const double *eps, double *res){
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double sum = 0.0;
    double term = 4.0;
    long long n = 1;
    int sign = 1;
    int iterators = 0;
    const int max_iterations = 1000000;

    while (module_num(term) > *eps) {

        sum += term;
        n++;
        iterators++;

        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;

        sign = -sign;

        term = 4.0 * sign / (2.0 * n - 1.0);

        if (sum + term == sum && module_num(term) > *eps)
            return STATUS_MACHINE_LIMIT;
    }
    *res = sum;
    return STATUS_OK;
}

Status pi_equation(const double *eps, double *res){
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double current = 3.0;
    double next;
    int iterators = 0;
    const int max_iterations = 1000000;

    while (1) {
        double del = sin(current);
        if (del == 0.0)
            return STATUS_INVALID_OPERATION;

        next = current + (cos(current) + 1.0) / del;

        if (module_num(next - current) <= *eps) {
            *res = next;
            return STATUS_OK;
        }

        if (next == current)
            return STATUS_MACHINE_LIMIT;
        iterators++;
        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;
        current = next;
    }
}

Status pi_limit(const double *eps, double *res){
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;
    double current = 4.0;
    double next;
    long long n = 1;
    int iterators = 0;
    const int max_iterations = 1000000;

    while (1) {
        next = current * (4.0 * n * (n + 1.0)) / ((2.0 * n + 1.0) * (2.0 * n + 1.0));

        if (module_num(next - current) <= *eps) {
            *res = next;
            return STATUS_OK;
        }

        if (next == current)
            return STATUS_MACHINE_LIMIT;

        iterators++;

        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;

        current = next;
        n++;
    }
}

Status ln2_series(const double *eps, double *res){
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double sum = 0.0;
    double term = 1.0;
    long long n = 1;
    int sign = 1;
    int iterators = 0;
    const int max_iterations = 1000000;

    while (module_num(term) > *eps) {
        sum += term;
        n++;
        iterators++;

        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;
        sign = -sign;
        term = (double)sign / n;
        if (sum + term == sum && module_num(term) > *eps)
            return STATUS_MACHINE_LIMIT;
    }
    *res = sum;
    return STATUS_OK;
}

Status ln2_equation(const double *eps, double *res){
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double current = 0.5;
    double exp_current;
    double next;
    int iterators = 0;
    const int max_iterations = 1000000;

    while (1) {
        if (current > log(DBL_MAX))
            return STATUS_OVERFLOW;

        exp_current = exp(current);
        if (exp_current == 0.0)
            return STATUS_INVALID_OPERATION;

        next = current - (exp_current - 2.0) / exp_current;

        if (module_num(next - current) <= *eps) {
            *res = next;
            return STATUS_OK;
        }

        if (next == current)
            return STATUS_MACHINE_LIMIT;

        iterators++;

        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;

        current = next;
    }
}

Status ln2_limit(const double *eps, double *res){
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double current = 1.0;
    double next;
    long long n = 2;
    int iterators = 0;
    const int max_iterations = 1000000;

    while (1) {
        next = n * (pow(2.0, 1.0 / n) - 1.0);
        if (module_num(next - current) <= *eps) {
            *res = next;
            return STATUS_OK;
        }

        if (next == current)
            return STATUS_MACHINE_LIMIT;
        iterators++;
        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;
        current = next;
        n++;
    }
}

Status sqrt2_series(const double *eps, double *res){
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double current = 1.0;
    double next;
    long long k = 2;
    int iterators = 0;
    const int max_iterations = 1000000;

    while (1) {
        next = current * pow(2.0, pow(2.0, -(double)k));
        if (module_num(next - current) <= *eps) {
            *res = next;
            return STATUS_OK;
        }

        if (next == current)
            return STATUS_MACHINE_LIMIT;

        iterators++;
        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;

        current = next;
        k++;
    }
}

Status sqrt2_equation(const double *eps, double *res){
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double current = 1.0;
    double next;
    int iterators = 0;
    const int max_iterations = 1000000;

    while (1) {
        if (current == 0.0)
            return STATUS_INVALID_OPERATION;

        next = (current + 2.0 / current) / 2.0;

        if (module_num(next - current) <= *eps) {
            *res = next;
            return STATUS_OK;
        }

        if (next == current)
            return STATUS_MACHINE_LIMIT;

        iterators++;
        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;
        current = next;
    }
}

Status sqrt2_limit(const double *eps, double *res){
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double current = -0.5;
    double next;
    int iterators = 0;
    const int max_iterations = 1000000;

    while (1) {
        next = current - current * current / 2.0 + 1.0;

        if (module_num(next - current) <= *eps) {
            *res = next;
            return STATUS_OK;
        }

        if (next == current)
            return STATUS_MACHINE_LIMIT;

        iterators++;
        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;
        current = next;
    }
}

Status check_prime(const long long *num, int *result)
{
    if (num == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;

    if (*num < 2) {
        *result = 0;
        return STATUS_OK;
    }

    if (*num == 2) {
        *result = 1;
        return STATUS_OK;
    }

    if (*num % 2 == 0) {
        *result = 0;
        return STATUS_OK;
    }

    for (long long i = 3; i <= *num/i; i += 2) {

        if (*num % i == 0) {
            *result = 0;
            return STATUS_OK;
        }
    }

    *result = 1;

    return STATUS_OK;
}


Status gamma_series(const double *eps, double *res)
{
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    if (*eps <= 0)
        return STATUS_SMALL_NUMBER;

    double pi;
    Status status;

    status = pi_series(eps, &pi);

    if (status != STATUS_OK)
        return status;

    double sum = -pi * pi / 6.0;
    double previous_sum = sum;
    double term;

    long long k = 2;

    int iterators = 0;
    const int max_iterations = 1000000;

    while (1) {

        long long root = (long long)sqrt((double)k);

        term = (1.0 / ((double)root * root)) - 1.0 / k;

        if (sum + term == sum && module_num(term) > *eps)
            return STATUS_MACHINE_LIMIT;

        previous_sum = sum;
        sum += term;

        if (!isfinite(sum))
            return STATUS_OVERFLOW;

        if (module_num(sum - previous_sum) <= *eps) {
            *res = sum;
            return STATUS_OK;
        }

        k++;
        iterators++;

        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;
    }
}

Status gamma_equation(const double *eps, double *res)
{
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    if (*eps <= 0)
        return STATUS_SMALL_NUMBER;

    double product = 1.0;

    long long t = 2;

    int prime;
    Status status;

    product *= (2.0 - 1.0) / 2.0;

    double current_argument = log(2.0) * product;

    if (current_argument <= 0.0)
        return STATUS_INVALID_OPERATION;

    double current = -log(current_argument);
    double next;

    int iterators = 0;
    const int max_iterations = 1000000;

    while (1) {

        t++;

        status = check_prime(&t, &prime);

        if (status != STATUS_OK)
            return status;

        if (prime)
            product *= ((double)t - 1.0) / t;

        current_argument = log((double)t) * product;

        if (current_argument <= 0.0)
            return STATUS_INVALID_OPERATION;

        next = -log(current_argument);

        if (module_num(next - current) <= *eps) {
            *res = next;
            return STATUS_OK;
        }

        if (next == current)
            return STATUS_MACHINE_LIMIT;

        iterators++;

        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;

        current = next;
    }
}

Status gamma_limit(const double *eps, double *res)
{
    if (eps == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    if (*eps <= 0)
        return STATUS_SMALL_NUMBER;

    double current = 0.0;
    double next;

    long long m = 1;

    int iterators = 0;
    const int max_iterations = 1000000;

    while (1) {

        m++;

        double sum = 0.0;
        double combination = 1.0;
        double log_factorial = 0.0;

        for (long long k = 1; k <= m; k++) {

            combination *=
                (double)(m - k + 1) / k;

            log_factorial += log((double)k);

            double term =
                combination *
                log_factorial /
                k;

            if (!isfinite(combination) ||
                !isfinite(term))
                return STATUS_OVERFLOW;

            if (k % 2 == 0)
                sum += term;
            else
                sum -= term;
        }

        next = sum;

        if (module_num(next - current) <= *eps) {
            *res = next;
            return STATUS_OK;
        }

        if (next == current)
            return STATUS_MACHINE_LIMIT;

        current = next;

        iterators++;

        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;
    }
}

Status conversion_num(const char *str, double *custom_eps){ 
    if (str == NULL || custom_eps == NULL) 
        return STATUS_NULL_ARGUMENT; 

    if (str[0] == '\0') 
        return STATUS_EMPTY_STRING; 
    
    int i = 0;
    int sign = 1; 
    
    if (str[i] == '-' || str[i] == '+'){ 
        if (str[i] == '-') 
            sign = -1; i++; 
        if (str[i] == '\0') 
            return STATUS_NO_DIGITS; 
    } 
    if (str[i] < '0' || str[i] > '9')
        return STATUS_INVALID_NUMBER;
    
    double value = 0.0; 
    double fraction = 0.0; 
    double divider = 10.0; 
    int point = 0; 
    int digits = 0; 
    
    for (; str[i] != '\0' && str[i] != 'e' && str[i] != 'E'; i++){ 

        if (str[i] == '.' || str[i] == ','){ 
            if (point != 0) 
                return STATUS_INVALID_NUMBER; 
                
            point = 1;
            continue; 
        } 
        if (str[i] < '0' || str[i] > '9') 
            return STATUS_INVALID_NUMBER; 
        
        int digit = str[i] - '0'; 
        digits++; 

        if (point == 0){ 
            if (value > (DBL_MAX - digit) / 10.0) 
                return STATUS_OVERFLOW; 
            
            value = value * 10.0 + digit; 
        } else{ 
            fraction += digit / divider; 
            divider *= 10.0; 
        } 
    } 
    if (digits == 0) 
        return STATUS_NO_DIGITS; 

    value += fraction; 

    if (str[i] == 'e' || str[i] == 'E'){ 
        i++; 
        int exponent_sign = 1; 
        if (str[i] == '+' || str[i] == '-'){ 
            if (str[i] == '-') 
                exponent_sign = -1; 
            i++; 
        } 
        if (str[i] == '\0') 
            return STATUS_NO_DIGITS; 
        
        int exponent = 0; 
        int exponent_digits = 0; 
        while (str[i] != '\0'){ 
            if (str[i] < '0' || str[i] > '9') 
                return STATUS_INVALID_NUMBER; 
            
            int digit = str[i] - '0'; 
            exponent_digits++;  
            
            if (exponent > (INT_MAX - digit) / 10) 
                return STATUS_OVERFLOW; 
                
            exponent = exponent * 10 + digit; 
            i++; 
        } 
        
        if (exponent_digits == 0) 
            return STATUS_NO_DIGITS; 
        
        exponent *= exponent_sign; 
        double multiplier = pow(10.0, exponent); 
        
        if (!isfinite(multiplier)) 
            return STATUS_OVERFLOW; 
            
        value *= multiplier; 
        
        if (!isfinite(value)) 
            return STATUS_OVERFLOW; 
    } 
    
    if (sign == -1) 
        return STATUS_SIGNED_NUMBER; 
    
    *custom_eps = value; 
    return STATUS_OK; 
}

void print_result(const char *name, Status status, double result){
    printf("%s: ", name);

    switch (status) {
        case STATUS_OK:
            printf("%.15g\n", result);
            break;

        case STATUS_NULL_ARGUMENT:
            printf("внутренняя ошибка: некорректный указатель.\n");
            break;

        case STATUS_MACHINE_LIMIT:
            printf("достигнут предел точности double.\n");
            break;

        case STATUS_MAX_ITERATIONS:
            printf("достигнуто максимальное количество итераций.\n");
            break;

        case STATUS_OVERFLOW:
            printf("переполнение.\n");
            break;

        case STATUS_INVALID_OPERATION:
            printf("невозможная математическая операция.\n");
            break;

        default:
            printf("неизвестная ошибка.\n");
            break;
    }
}

int main(int argc, char *argv[]){
    if (argc!=2){
        printf("Ошибка: введите эпсилон.\n");
        return 1;
    }

    double machine_eps = 1;
    Status status_machine_eps = machine_epsilon(&machine_eps);
    if (status_machine_eps == STATUS_NULL_ARGUMENT){
        printf("Внутренняя ошибка: некорректные данные.\n");
        return 1;
    }

    double cust_eps = 0.0;
    Status status_eps = conversion_num(argv[1], &cust_eps);

    switch(status_eps){
        case STATUS_NULL_ARGUMENT:{
            printf("Внутренняя ошибка: некорректные данные.\n");
            return 1;
        }

        case STATUS_EMPTY_STRING:{
            printf("Ошибка: пустая строка. Введите эпсилон.\n");
            return 1;
        }

        case STATUS_INVALID_NUMBER:{
            printf("Ошибка: введено некорректное число. Ведите положительное вещественное число.\n");
            return 1;
        }

        case STATUS_NO_DIGITS:{
            printf("Ошибка: введено не число. Введите положительное вещественное число.\n");
            return 1;
        }

        case STATUS_SIGNED_NUMBER:{
            printf("Ошибка: введено отрицательное число. Введите положительное вещественное число.\n");
            return 1;
        }

        case STATUS_OVERFLOW:{
            printf("Ошибка: переполнение. Попробуйте другое число.\n");
            return 1;
        }

        case STATUS_OK:
            break;

        default: {
            printf("Ошибка: неизвестный статус.\n");
            return 1;
        }
    }

    if (cust_eps > 0 && cust_eps <= machine_eps) {
        printf("Ошибка: epsilon слишком мало для типа double.\n");
        return 1;
    } else if (cust_eps <= 0){
        printf("Ошибка: epsilon должно быть положительным.\n");
        return 1;
    }

    printf("epsilon = %.17g\n", cust_eps);

    double result;
    Status status;

    status = e_series(&cust_eps, &result);
    print_result("e (ряд)", status, result);

    status = e_equation(&cust_eps, &result);
    print_result("e (уравнение)", status, result);

    status = e_limit(&cust_eps, &result);
    print_result("e (предел)", status, result);

    status = pi_series(&cust_eps, &result);
    print_result("pi (ряд)", status, result);

    status = pi_equation(&cust_eps, &result);
    print_result("pi (уравнение)", status, result);

    status = pi_limit(&cust_eps, &result);
    print_result("pi (предел)", status, result);


    status = ln2_series(&cust_eps, &result);
    print_result("ln 2 (ряд)", status, result);

    status = ln2_equation(&cust_eps, &result);
    print_result("ln 2 (уравнение)", status, result);

    status = ln2_limit(&cust_eps, &result);
    print_result("ln 2 (предел)", status, result);


    status = sqrt2_series(&cust_eps, &result);
    print_result("sqrt 2 (произведение)", status, result);

    status = sqrt2_equation(&cust_eps, &result);
    print_result("sqrt 2 (уравнение)", status, result);

    status = sqrt2_limit(&cust_eps, &result);
    print_result("sqrt 2 (предел)", status, result);


    status = gamma_series(&cust_eps, &result);
    print_result("gamma (ряд)", status, result);

    status = gamma_equation(&cust_eps, &result);
    print_result("gamma (уравнение)", status, result);

    status = gamma_limit(&cust_eps, &result);
    print_result("gamma (предел)", status, result);

    return 0;
}