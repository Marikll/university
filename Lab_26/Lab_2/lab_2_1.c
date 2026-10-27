#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stddef.h>
#include <limits.h>

typedef enum {
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_EMPTY_STRING,
    STATUS_INVALID_FLAG,
    STATUS_FLAG_L,
    STATUS_FLAG_R,
    STATUS_FLAG_U,
    STATUS_FLAG_N,
    STATUS_FLAG_C,
    STATUS_MEMORY_ERROR,
    STATUS_INVALID_NUMBER,
    STATUS_OVERFLOW
} Status;

static size_t string_length(const char *str){
    size_t n = 0;
    while (str[n] != '\0')
        n++;
    return n;
}

Status flag_l(const char *str, size_t *length){
    if (str == NULL || length == NULL)
        return STATUS_NULL_ARGUMENT;

    *length = string_length(str);
    return STATUS_OK;
}

Status flag_r(const char *str, char **result){
    if (str == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t len = string_length(str);
    char *out = malloc(len + 1);
    if (out == NULL)
        return STATUS_MEMORY_ERROR;

    for (size_t i = 0; i < len; i++)
        out[i] = str[len - 1 - i];
    out[len] = '\0';

    *result = out;
    return STATUS_OK;
}

Status flag_u(const char *str, char **result){
    if (str == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t len = string_length(str);
    char *out = malloc(len + 1);
    if (out == NULL)
        return STATUS_MEMORY_ERROR;

    for (size_t i = 0; i < len; i++) {
        if (i % 2 == 1)
            out[i] = (char)toupper((unsigned char)str[i]);
        else
            out[i] = str[i];
    }
    out[len] = '\0';

    *result = out;
    return STATUS_OK;
}

Status flag_n(const char *str, char **result){
    if (str == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t len = string_length(str);
    char *out = malloc(len + 1);
    if (out == NULL)
        return STATUS_MEMORY_ERROR;

    size_t pos = 0;

    for (size_t i = 0; i < len; i++)          
        if (isdigit((unsigned char)str[i]))
            out[pos++] = str[i];

    for (size_t i = 0; i < len; i++)        
        if (isalpha((unsigned char)str[i]))
            out[pos++] = str[i];

    for (size_t i = 0; i < len; i++)         
        if (!isdigit((unsigned char)str[i]) && !isalpha((unsigned char)str[i]))
            out[pos++] = str[i];

    out[pos] = '\0';

    *result = out;
    return STATUS_OK;
}

/*first - вторая переданная строка (argv[2]), others - строки начиная с argv[4], others_count - их количество*/
Status flag_c(const char *first, unsigned int seed, char *const *others, size_t others_count, char **result)
{
    if (first == NULL || result == NULL || (others == NULL && others_count > 0))
        return STATUS_NULL_ARGUMENT;

    /* Массив указателей на все строки: first + others */
    if (others_count > (size_t)-1 / sizeof(const char *) - 1)
        return STATUS_OVERFLOW;

    size_t count = others_count + 1;

    const char **parts = malloc(count * sizeof(*parts));
    if (parts == NULL)
        return STATUS_MEMORY_ERROR;

    parts[0] = first;
    for (size_t i = 0; i < others_count; i++) {
        if (others[i] == NULL) {
            free(parts);
            return STATUS_NULL_ARGUMENT;
        }
        parts[i + 1] = others[i];
    }

    /* Перемешивание Фишера–Йетса */
    srand(seed);
    for (size_t i = count - 1; i > 0; i--) {
        size_t j = (size_t)rand() % (i + 1);
        const char *tmp = parts[i];
        parts[i] = parts[j];
        parts[j] = tmp;
    }

    /* Общая длина с проверкой переполнения */
    size_t total = 0;
    for (size_t i = 0; i < count; i++) {
        size_t len = string_length(parts[i]);
        if (total > (size_t)-1 - 1 - len) {
            free(parts);
            return STATUS_OVERFLOW;
        }
        total += len;
    }

    char *out = malloc(total + 1);
    if (out == NULL) {
        free(parts);
        return STATUS_MEMORY_ERROR;
    }

    /* Конкатенация */
    size_t pos = 0;
    for (size_t i = 0; i < count; i++) {
        for (size_t k = 0; parts[i][k] != '\0'; k++)
            out[pos++] = parts[i][k];
    }
    out[pos] = '\0';

    free(parts);
    *result = out;
    return STATUS_OK;
}

Status parse_unsigned(const char *str, unsigned int *value){
    if (str == NULL || value == NULL)
        return STATUS_NULL_ARGUMENT;

    if (str[0] == '\0')
        return STATUS_EMPTY_STRING;
    size_t i = 0;

    if (str[0] == '+')
        i++;

    unsigned int result = 0;

    for (; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9')
            return STATUS_INVALID_NUMBER;

        unsigned int digit = (unsigned int)(str[i] - '0');

        if (result > (UINT_MAX - digit) / 10)
            return STATUS_OVERFLOW;

        result = result * 10 + digit;
    }

    *value = result;
    return STATUS_OK;
}

Status check_flag(const char *str){
    if (str == NULL)
        return STATUS_NULL_ARGUMENT;
    if (str[0] == '\0')
        return STATUS_EMPTY_STRING;
    if (str[0] != '-' || str[1] == '\0' || str[2] != '\0')
        return STATUS_INVALID_FLAG;

    switch (str[1]) {
        case 'l': return STATUS_FLAG_L;
        case 'r': return STATUS_FLAG_R;
        case 'u': return STATUS_FLAG_U;
        case 'n': return STATUS_FLAG_N;
        case 'c': return STATUS_FLAG_C;
        default:  return STATUS_INVALID_FLAG;
    }
}

int main(int argc, char *argv[]){
    if (argc < 3) {
        printf("Ошибка: введите флаг и строку; для -c ещё число и строки.\n");
        return 1;
    }

    Status st_flag = check_flag(argv[1]);

    switch (st_flag) {
        case STATUS_FLAG_L: {
            if (argc != 3) {
                printf("Ошибка: для -l нужна ровно одна строка.\n");
                return 1;
            }
            size_t length;
            Status st = flag_l(argv[2], &length);
            if (st != STATUS_OK) {
                printf("Внутренняя ошибка.\n");
                return 1;
            }
            printf("Длина строки: %zu\n", length);
            return 0;
        }

        case STATUS_FLAG_R:
        case STATUS_FLAG_U:
        case STATUS_FLAG_N: {
            if (argc != 3) {
                printf("Ошибка: для этого флага нужна ровно одна строка.\n");
                return 1;
            }
            char *result = NULL;
            Status st;

            if (st_flag == STATUS_FLAG_R)
                st = flag_r(argv[2], &result);
            else if (st_flag == STATUS_FLAG_U)
                st = flag_u(argv[2], &result);
            else
                st = flag_n(argv[2], &result);

            if (st == STATUS_MEMORY_ERROR) {
                printf("Ошибка выделения памяти.\n");
                return 1;
            }
            if (st != STATUS_OK) {
                printf("Внутренняя ошибка.\n");
                return 1;
            }

            printf("%s\n", result);
            free(result);
            return 0;
        }

        case STATUS_FLAG_C: {
            if (argc < 4) {
                printf("Ошибка: для -c нужны строка, число и произвольное количество строк.\n");
                return 1;
            }

            unsigned int seed;
            Status st = parse_unsigned(argv[3], &seed);

            if (st == STATUS_OVERFLOW) {
                printf("Ошибка: число слишком большое для unsigned int.\n");
                return 1;
            }
            if (st != STATUS_OK) {
                printf("Ошибка: третий аргумент должен быть целым неотрицательным числом.\n");
                return 1;
            }

            char *result = NULL;
            st = flag_c(argv[2], seed, argv + 4, (size_t)(argc - 4), &result);

            if (st == STATUS_MEMORY_ERROR) {
                printf("Ошибка выделения памяти.\n");
                return 1;
            }
            if (st != STATUS_OK) {
                printf("Ошибка при объединении строк.\n");
                return 1;
            }

            printf("%s\n", result);
            free(result);
            return 0;
        }

        case STATUS_EMPTY_STRING:
            printf("Ошибка: передана пустая строка вместо флага.\n");
            return 1;

        default:
            printf("Ошибка: некорректный флаг.\n");
            return 1;
    }
}