#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_BASE,
    STATUS_INVALID_NUMBER,
    STATUS_MEMORY_ERROR,
    STATUS_OVERFLOW
} Status;

size_t string_length(const char *str){
    size_t n = 0;
    while (str[n] != '\0')
        n++;
    return n;
}

Status digit_value(char c, int base, int *value){
    if (value == NULL)
        return STATUS_NULL_ARGUMENT;

    int d;
    if (c >= '0' && c <= '9')
        d = c - '0';
    else if (c >= 'A' && c <= 'Z')
        d = c - 'A' + 10;
    else if (c >= 'a' && c <= 'z')
        d = c - 'a' + 10;
    else
        return STATUS_INVALID_NUMBER;

    if (d >= base)
        return STATUS_INVALID_NUMBER;

    *value = d;
    return STATUS_OK;
}

Status check_number(const char *str, int base){
    if (str == NULL)
        return STATUS_NULL_ARGUMENT;
    if (str[0] == '\0')
        return STATUS_INVALID_NUMBER;

    for (size_t i = 0; str[i] != '\0'; i++){
        int d;
        Status st = digit_value(str[i], base, &d);
        if (st != STATUS_OK)
            return st;
    }
    return STATUS_OK;
}

Status add_columns(const char *a, const char *b, int base, char **result){
    if (a == NULL || b == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;
    if (base < 2 || base > 36)
        return STATUS_INVALID_BASE;

    const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    size_t len_a = string_length(a);
    size_t len_b = string_length(b);
    size_t max = len_a > len_b ? len_a : len_b;

    if (max > (size_t)-1 - 2)
        return STATUS_OVERFLOW;

    char *reversed = malloc((max + 2) * sizeof(*reversed));     
    if (reversed == NULL)
        return STATUS_MEMORY_ERROR;

    size_t pos = 0;
    int carry = 0;

    for (size_t k = 0; k < max; k++){
        int da = 0, db = 0;
        Status st;

        if (k < len_a){
            st = digit_value(a[len_a - 1 - k], base, &da);
            if (st != STATUS_OK){
                free(reversed);
                return st;
            }
        }
        if (k < len_b){
            st = digit_value(b[len_b - 1 - k], base, &db);
            if (st != STATUS_OK){
                free(reversed);
                return st;
            }
        }

        int sum = da + db + carry;
        reversed[pos++] = digits[sum % base];  
        carry = sum / base;                 
    }

    if (carry > 0)
        reversed[pos++] = digits[carry];

    while (pos > 1 && reversed[pos - 1] == '0')
        pos--;

    char *out = malloc((pos + 1) * sizeof(*out));
    if (out == NULL){
        free(reversed);
        return STATUS_MEMORY_ERROR;
    }
    for (size_t i = 0; i < pos; i++)
        out[i] = reversed[pos - 1 - i];
    out[pos] = '\0';

    free(reversed);
    *result = out;
    return STATUS_OK;
}

Status sum_numbers(int base, size_t count, char **result, ...){
    if (result == NULL)
        return STATUS_NULL_ARGUMENT;
    if (base < 2 || base > 36)
        return STATUS_INVALID_BASE;

    char *sum = malloc(2 * sizeof(*sum));
    if (sum == NULL)
        return STATUS_MEMORY_ERROR;
    sum[0] = '0';
    sum[1] = '\0';

    va_list numbers;
    va_start(numbers, result);

    Status st = STATUS_OK;
    for (size_t i = 0; i < count; i++){
        const char *num = va_arg(numbers, const char *);

        st = check_number(num, base);
        if (st != STATUS_OK)
            break;

        char *new_sum = NULL;
        st = add_columns(sum, num, base, &new_sum);
        if (st != STATUS_OK)
            break;

        free(sum);
        sum = new_sum;
    }
    va_end(numbers);

    if (st != STATUS_OK){
        free(sum);
        return st;
    }

    *result = sum;
    return STATUS_OK;
}

void print_status_error(Status status){
    switch (status){
        case STATUS_NULL_ARGUMENT:  printf("Внутренняя ошибка: нулевой указатель.\n"); break;
        case STATUS_INVALID_BASE:   printf("Ошибка: основание должно быть в диапазоне [2, 36].\n"); break;
        case STATUS_INVALID_NUMBER: printf("Ошибка: некорректное число для этой системы счисления.\n"); break;
        case STATUS_MEMORY_ERROR:   printf("Ошибка: не удалось выделить память.\n"); break;
        case STATUS_OVERFLOW:       printf("Ошибка: переполнение.\n"); break;
        default:                    printf("Ошибка.\n"); break;
    }
}

int main(void){
    char *res = NULL;
    Status st;

    st = sum_numbers(10, 3, &res, "999", "1", "0005");
    if (st == STATUS_OK){ printf("base 10: %s\n", res); free(res); }   // 1005
    else print_status_error(st);

    st = sum_numbers(36, 2, &res, "Z", "1");
    if (st == STATUS_OK){ printf("base 36: %s\n", res); free(res); }   // 10
    else print_status_error(st);

    st = sum_numbers(16, 3, &res, "FFFFFFFFFFFFFFFFFFFFFFFF", "1", "a");
    if (st == STATUS_OK){ printf("base 16: %s\n", res); free(res); }   // 1000000000000000000000009
    else print_status_error(st);

    st = sum_numbers(2, 2, &res, "1011", "0110");
    if (st == STATUS_OK){ printf("base 2: %s\n", res); free(res); }    // 10001
    else print_status_error(st);

    st = sum_numbers(36, 2, &res, "12", "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz");                         // ошибка: A не цифра в base 10
    if (st == STATUS_OK){ printf("%s\n", res); free(res); }
    else print_status_error(st);

    st = sum_numbers(10, 0, &res);                                     // пустая сумма
    if (st == STATUS_OK){ printf("пустая сумма: %s\n", res); free(res); }

    return 0;
}