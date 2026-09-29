#include <stdio.h>
#include <stdbool.h>

/* Функция для нахождения чисел, кратных x, в пределах 100 (включительно).*/
/* Если числа отсутствуют, возвращается 0. Иначе 1.*/
/* С помощью *k можно вывести числа из массива.*/
int flag_h (long long x, long long numbers[], int *k){
    if (x < -100 || x == 0 || x > 100){
        printf("Для флага -h подходят только числа от -100 до -1 и от 1 до 100.");
        return 0;
    }
    *k = 0;
    for (long long i = x; i <= 100; i++){
        if (i%x == 0){
            numbers[*k] = i;
            (*k)++;
        }
    }
    if (*k > 0){
        return 1;
    } else{
        printf("Числа кратные %lld отсутствуют.", x);
        return 0;
    }
}

/* Функция, определяющая является ли x простым, составным.*/
/* Функция возвращает 0, если число — составное, 1 — простое.*/
int flag_p(long long x){
    if (x < 2){
        printf("Для этого флага подходят только натуральные числа. %lld \n", x);
        return 0;
    }
    int k = 0;
    for (long long i = 1; i <= x; i++){
        if (x%i == 0)
            k++;
        if (k > 2){
            printf("Число %lld является составным.\n", x);
            return 0;
        }
    }
    printf("Число %lld является простым.\n", x);
    return 1;
}

/* Функция, которая делит x на отдельные цифры в 16 СС, и выводит их от старших разрядов к младшим.*/
/* Ставим указатель на нулевой элемент массива, 
в качестве последнего элемента массива ставим 0, 
и начинаем заполнять массив с конца/
Сначала k - указатель на длину числа, затем указатель на первый перед нашим числом свободный элемент массива.*/
int flag_s(long long x, char numbers[], int *k){
    if (x < 0){
        printf("Для этого флага подходят только 0 и натуральные числа.");
        return 0;
    }
    int d, base = 16, count = 0;
    char *pres = numbers;
    pres[*k - 1] = '\0';
    pres += *k - 2;
    while(x){
        if ((d = x%base) > 9)
            *pres-- = d - 10 + 'A';
        else *pres-- = d + '0';
        x /= base;
        count++;
    }
    *k = (numbers + *k - 1) - pres - 1;
    return 1;
}

/* Функция, которая выводит таблицу степеней для показателей от 1 до x и для оснований от 1 до 10*/
void flag_e(long long x){
    if (x >= 1 && x <= 10){
        printf("|  a^n  |");
        for (int i = 1; i <= x; i++)
            printf("    %d    |", i);
        printf("\n");
        for (int k = 1; k <= 10; k++){
            printf("|  %d  |", k);
            long long res = 1;
            for (int j = 1; j <= x; j++){
                res *= k;
                printf("  %lld  |", res);
            }
            printf("\n");
        }
    } else printf("Ошибка: введите x не больше 10.\n");
}

/* Функция, вычисляющая сумму всех чисел от 1 до x*/
int flag_a(long long x){
    if (x < 1){
        printf("Для этого флага подходят только натуральные числа.\n");
        return 0;
    }
    long long max_num = (1ULL << (sizeof(long long) * 8 - 1)) - 1;
    long long res = 0;
    for (long long i = 1; i <= x; i++){
        if (res <= max_num - i)
            res += i;
        else {
            printf("Ошибка: переполнение.\n");
            return 0;
        }
    }
    printf("%lld", res);
    return 1;
}

/*Функция для вычисления факториала x*/
int flag_f(long long x){
    if (x < 1){
        printf("Для этого флага подходят только натуральные числа.\n");
        return 0;
    }
    long long max_num = (1ULL << (sizeof(long long) * 8 - 1)) - 1;
    long long res = 1;
    for (long long i = 1; i <= x; i++){
        if (res > (max_num/i)){
            printf("Ошибка: переполнение\n");
            return 0;
        }
        res *= i;
    }
    printf("%lld\n", res);
    return 1;
}

typedef enum {
    STATUS_OK = 0,
    STATUS_NULL_ARGUMENT,
    STATUS_EMPTY_STRING,
    STATUS_INVALID_FLAG,
    STATUS_INVALID_NUMBER,
    STATUS_NO_DIGITS,
    STATUS_OVERFLOW
} Status;

Status check_flag(const char * str, char * flag){
    if (str == NULL || flag == NULL)
        return STATUS_NULL_ARGUMENT;
    if (str[0] != '-' && str[0] != '/')
        return STATUS_INVALID_FLAG;
    if (str[2] != '\0')
        return STATUS_INVALID_FLAG;
    if (str[1] == '\0')
        return STATUS_INVALID_FLAG;
    if (str[0] == '\0')
        return STATUS_EMPTY_STRING;
    switch (str[1]){
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

Status check_number(const char * str, long long * num){
    if (str == NULL || num == NULL)
        return STATUS_NULL_ARGUMENT;
    if (str[0] == '\0')
        return STATUS_EMPTY_STRING;
    int i = 0;
    int sign = 1;

    if (str[i] == '+' || str[i] == '-'){
        if (str[i] == '-')
            sign = -1;
        i++;
        if (str[i] == '\0')
            return STATUS_NO_DIGITS;
    }

    long long value = 0;
    long long lim_ll;
    if (sign == -1){
        lim_ll = 1ULL << (sizeof(long long)*8 - 1);
    } else {
        lim_ll = (1ULL << (sizeof(long long))*8 - 1) - 1;
    }
    
    for (; str[i] != '\0'; i++){
        if (str[i] == '.' || str[i] == ',')
            return STATUS_INVALID_NUMBER;
        if (str[i] < '0' || str[i] > '9')
            return STATUS_NO_DIGITS;

        int digit = str[i] - '0';

        if (value > (lim_ll - digit) / 10)
            return STATUS_OVERFLOW;
        
        value = value*10 + digit;
    }
    if (sign == -1)
        *num = -value;
    else
        *num = value;
    return STATUS_OK;
}

int main(int argc, char *argv[]){

    if (argc != 3){
        printf("Ошибка: необходимо передать число и флаг.\n");
        return 1;
    }
    long long x1 = 0;
    long long x2 = 0;
    long long x;

    char flag1 = '\0';
    char flag2 = '\0';
    char flag;

    Status status_num1 = check_number(argv[1], &x1);
    Status status_num2 = check_number(argv[2], &x2);

    Status status_flag1 = check_flag(argv[1], &flag1);
    Status status_flag2 = check_flag(argv[2], &flag2);

    if (status_num1 == STATUS_OK && status_flag2 == STATUS_OK){
        x = x1;
        flag = flag2;
    } else if (status_num2 == STATUS_OK && status_flag1 == STATUS_OK){
        x = x2;
        flag = flag1;
    } else if (status_num1 == STATUS_OK && status_num2 == STATUS_OK){
        printf("Ошибка: переданы два числа. Введите число и флаг.\n");
        return 1;
    } else if (status_flag1 == STATUS_OK && status_flag2 == STATUS_OK){
        printf("Ошибка: переданы два флага. Введите число и флаг.\n");
        return 1;
    } else if (status_num1 == STATUS_OK){
        printf("Ошибка: ");
        switch (status_flag2){
            case STATUS_EMPTY_STRING:
                printf("не передан флаг.\n");
                break;
            case STATUS_INVALID_FLAG:
                printf("передан некорректный флаг.\n");
                break;
            case STATUS_NULL_ARGUMENT:
                printf("передан недопустимый указатель (внутренняя ошибка).\n");
                break;
        }
        return 1;
    } else if (status_num2 == STATUS_OK){
        printf("Ошибка: ");
        switch (status_flag1){
            case STATUS_EMPTY_STRING:
                printf("не передан флаг. Введите число и флаг.\n");
                break;
            case STATUS_INVALID_FLAG:
                printf("передан некорректный флаг. Допустимые флаги: h, p, a, s, e, f.\n");
                break;
            case STATUS_NULL_ARGUMENT:
                printf("передан недопустимый указатель (внутренняя ошибка).\n");
                break;
            default:
                printf("некорректное значение.\n");
                break;
        }
        return 1;
    } else if (status_flag1 == STATUS_OK){
        printf("Ошибка: ");
        switch (status_num2){
            case STATUS_NULL_ARGUMENT:
                printf("передан недопустимый указатель (внутренняя ошибка).\n");
                break;
            case STATUS_NO_DIGITS:
                printf("передано не число. Введите число и флаг.\n");
                break;
            case STATUS_EMPTY_STRING:
                printf("не передано число. Введите число и флаг.\n");
                break;
            case STATUS_INVALID_NUMBER:
                printf("вещественные числа не подходят. Введите целое число.\n");
                break;
            case STATUS_OVERFLOW:
                printf("переполнение. Введите число меньше.\n");
                break;
            default:
                printf("некорректное значение.\n");
                break;
        }
        return 1;
    } else if (status_flag2 == STATUS_OK){
        printf("Ошибка: ");
        switch (status_num1){
            case STATUS_NULL_ARGUMENT:
                printf("передан недопустимый указатель (внутренняя ошибка).\n");
                break;
            case STATUS_NO_DIGITS:
                printf("передано не число. Введите число и флаг.\n");
                break;
            case STATUS_EMPTY_STRING:
                printf("не передано число. Введите число и флаг.\n");
                break;
            case STATUS_INVALID_NUMBER:
                printf("вещественные числа не подходят. Введите целое число.\n");
                break;
            case STATUS_OVERFLOW:
                printf("переполнение. Введите число меньше.\n");
                break;
            default:
                printf("некорректное значение.\n");
                break;
        }
        return 1;
    } else {
        printf("Ошибка: введите число и корректный флаг.\n");
        return 1;
    }

    switch (flag) {

        case 'a': {
            flag_a(x);
            break;
        }

        case 'e': {
            flag_e(x);
            break;
        }

        case 'f': {
            flag_f(x);
            break;
        }

        case 's': {
            int len_num = (int)(sizeof(long long)/4) + 1;
            char numbers[len_num];
            int s = flag_s(x, numbers, &len_num);
            if (s == 1){
                for (int i = 0; i <= len_num; i++)
                    printf("%c", numbers[i]);
            }
            printf("\n");
            break;
        }

        case 'h': {
            long long numbers[201];
            int k;
            int h = flag_h(x, numbers, &k);
            if (h == 1){
                for (int i = 0; i <= k-1; i++)
                    printf("%lld ", numbers[i]);
            }
            printf("\n");
            break;
        }

        case 'p': {
            flag_p(x);
            break;
        }

        default:
            printf("Ошибка.\n");
            return 1;
    }

    return 0;

}

