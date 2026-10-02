#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_MACHINE_LIMIT,
    STATUS_MAX_ITERATIONS,
    STATUS_EMPTY_STRING,
    STATUS_NO_DIGITS,
    STATUS_INVALID_NUMBER,
    STATUS_OVERFLOW,
    STATUS_SIGNED_NUMBER,
    STATUS_INVALID_OPERATION
} Status;

Status machine_epsilon(double *eps){
    if (eps == NULL)
        return STATUS_NULL_ARGUMENT;
    while ((1.0 + *eps/2.0) != 1.0)
        *eps /= 2.0;
    return STATUS_OK; 
}

Status sum_a(const double eps, double *x, double *res){
    if (x == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double x1 = *x;
    double term = 1;
    int n = 1;
    double sum = term;
    double new_sum;
    int max_iterations = 10000000;
    int iterators = 0;
    while (1){
        iterators++;
        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;
        term = term * (x1/n);
        n++;
        new_sum = sum + term;
        sum = new_sum;
        if (fabs(term) < eps)
            break;
    }

    *res = sum;
    return STATUS_OK;
}

Status sum_b(const double eps, double *x, double *res){
    if (x == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double x1 = *x;
    double term = 1;
    int n = 1;
    double sum = term;
    double new_sum;
    int max_iterations = 10000000;
    int iterators = 0;
    while (1){
        iterators++;
        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;
        term = -1 * term * (x1 * x1/((2 * n - 1)*(2 * n)));
        n++;
        new_sum = sum + term;
        sum = new_sum;
        if (fabs(term) < eps)
            break;
    }

    *res = sum;
    return STATUS_OK;
}

Status sum_c(const double eps, double *x, double *res){
    if (x == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double x1 = *x;
    double term = 1;
    int n = 1;
    double sum = term;
    double new_sum;
    int max_iterations = 10000000;
    int iterators = 0;
    if (fabs(x1 - 1) < eps || x1 > (1 + eps))
        return STATUS_INVALID_OPERATION;
    while (1){
        iterators++;
        if (iterators >= max_iterations)
            return STATUS_MAX_ITERATIONS;
        term = term * 27 * n * n * n * x1 * x1/((3 * n - 2) * (3 * n - 1) * (3 * n));
        n++;
        new_sum = sum + term;
        sum = new_sum;
        if (fabs(term) < eps)
            break;
    }

    *res = sum;
    return STATUS_OK;
}

Status sum_d(const double eps, double *x, double *res){
    if (x == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    double x1 = *x;
    double term = - x1 * x1/2;
    int n = 2;
    double sum = term;
    double new_sum;
    int max_iterations = 10000000;
    int iterators = 0;
    if (x1 > (1 + eps))
        return STATUS_INVALID_OPERATION;
    while (1){
        iterators++;
        if (iterators >= max_iterations)
             return STATUS_MAX_ITERATIONS;
        // printf("%d \n", n);
        term = -1 * term * (2 * n - 1) * x1 * x1/(2 * n);
        n++;
        new_sum = sum + term;
        sum = new_sum;
        if (fabs(term) < eps)
            break;
    }

    *res = sum;
    return STATUS_OK;
}

double function_a(double x){
    if (x == 0)
        return 1.0;
    return log(1 + x)/x;
}

double function_b(double x){
    return exp(-(x * x)/2);
}

double function_c(double x){
    if (x >= 1.0)
        return 0.0;
    
    if (x == 0.0)
        return 0.0;

    return -log(1.0 - x);
}

double function_d(double x){
    if (x == 0)
        return 1.0;

    return exp(x * log(x));
}


Status integral(const double eps, double *res, const int flag){
    if (res == NULL)
        return STATUS_NULL_ARGUMENT;
    int n = 1;
    double h = 1.0/n;
    double sum;
    if (flag == 1)
        sum = (function_a(0) + function_a(1))/2;
    else if (flag == 2)
        sum = (function_b(0) + function_b(1))/2;
    else if (flag == 3)
        sum = (function_c(0) + function_c(1))/2;
    else if (flag == 4)
        sum = (function_d(0) + function_d(1))/2;
    double x;
    double new_sum = h * sum;
    double old_sum = new_sum;
    int max_iterations = 10000000;
    int iterators = 1;
    while(1){
        iterators++;
        if (iterators >= max_iterations)
             return STATUS_MAX_ITERATIONS;
        n++;
        h = 1.0/n;
        if (flag == 1){
            sum = (function_a(0) + function_a(1))/2;
            for (int i = 1; i < n; i++)
                sum += function_a(i * h);
        } else if (flag == 2){
            sum = (function_b(0) + function_b(1))/2;
            for (int i = 1; i < n; i++)
                sum += function_b(i * h);
        } else if(flag == 3){
            sum = (function_c(0) + function_c(1))/2;
            for (int i = 1; i < n; i++)
                sum += function_c(i * h);
        } else if (flag == 4){
            sum = (function_d(0) + function_d(1))/2;
            for (int i = 1; i < n; i++)
                sum += function_d(i * h);
        }
        new_sum = h * sum;
        if (fabs(new_sum - old_sum) < eps)
            break;
        old_sum = new_sum;
    }
    *res = new_sum;
    return STATUS_OK;
}


Status conversion_num(const char *str, double *num, int flag_eps){ 
    if (str == NULL || num == NULL) 
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

    if ((str[i] == 'e' || str[i] == 'E') && flag_eps == 0)
            return STATUS_INVALID_NUMBER;
    else if (flag_eps == 1){
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
    } else{
        if (sign == -1) 
            value = -value; 
    }

    *num = value; 
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
            printf("Ряд расходится. Сумму невозможно посчитать.\n");
            break;

        default:
            printf("неизвестная ошибка.\n");
            break;
    }
}

int main(int argc, char *argv[]){
    if (argc!=3){
        printf("Ошибка: введите эпсилон и число.\n");
        return 1;
    }

    double machine_eps = 1;
    Status status_machine_eps = machine_epsilon(&machine_eps);
    if (status_machine_eps == STATUS_NULL_ARGUMENT){
        printf("Внутренняя ошибка: некорректные данные.\n");
        return 1;
    }

    double cust_eps = 0.0;
    int flag = 1;
    Status status_eps = conversion_num(argv[1], &cust_eps, flag);

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

    double cust_x = 0.0;
    flag = 0;
    Status status_x = conversion_num(argv[2], &cust_x, flag);

    switch(status_x){
        case STATUS_NULL_ARGUMENT:{
            printf("Внутренняя ошибка: некорректные данные.\n");
            return 1;
        }

        case STATUS_EMPTY_STRING:{
            printf("Ошибка: пустая строка. Введите число.\n");
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

    if (cust_eps <= machine_eps) {
        printf("Ошибка: epsilon слишком мало для типа double.\n");
        return 1;
    }

    printf("epsilon = %.17g\n", cust_eps);

    double result;
    Status status;

    status = sum_a(cust_eps, &cust_x, &result);
    print_result("сумма a", status, result);

    status = sum_b(cust_eps, &cust_x, &result);
    print_result("сумма b", status, result);

    status = sum_c(cust_eps, &cust_x, &result);
    print_result("сумма c", status, result);

    status = sum_d(cust_eps, &cust_x, &result);
    print_result("сумма d", status, result);
    
    const char* integral_names[] = {"интеграл a", "интеграл b", "интеграл c", "интеграл d"};
    for (int i = 1; i < 5; i++){
        status = integral(cust_eps, &result, i);
        print_result(integral_names[i-1], status, result);
    }

    return 0;
}
