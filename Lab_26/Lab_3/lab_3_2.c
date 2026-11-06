#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

typedef struct{
    unsigned long long id;
    unsigned int start_time;
    unsigned int finish_time;
} Conference;

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_MEMORY_ERROR,
    STATUS_INPUT_ERROR,
    STATUS_OVERFLOW,
    STATUS_EMPTY_STRING,
    STATUS_SIGN_ERROR,
    STATUS_NO_DIGITS,
    STATUS_INVALID_NUMBER,
    STATUS_INVALID_HOUR,
    STATUS_INVALID_MINUTS,
    STATUS_INVALID_TIME,
    STATUS_INVALID_ID,
    STATUS_DUPLICATE_ID
} Status;

static void free_conf(Conference *conf){
    free(conf);
}

Status search(const Conference *conf, unsigned int *halls, size_t count_conf, size_t count_halls, size_t *result){
    if (conf == NULL || halls == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t count_result = 0;

    for (size_t i = 0; i < count_conf; i++){
        int found = 0;
        size_t best = 0;

        for (size_t j = 0; j < count_halls; j++){
            if (halls[j] <= conf[i].start_time && (!found || halls[j] > halls[best])){
                best = j;
                found = 1;
            }
        }
        if (!found)
            continue;

        halls[best] = conf[i].finish_time;
        count_result++;
    }

    *result = count_result;
    return STATUS_OK;
}

Status check_number(const char *str, int flag, unsigned long long *num){
    if (str == NULL || num == NULL)
        return STATUS_NULL_ARGUMENT;

    if (str[0] == '\0')
        return STATUS_EMPTY_STRING;

    size_t i = 0;

    if (str[i] == '+' || str[i] == '-') {
        if (flag != 0)
            return STATUS_INVALID_NUMBER;
        if (str[i] == '-')
            return STATUS_SIGN_ERROR;
        i++;
    }

    if (str[i] == '\0')
        return STATUS_NO_DIGITS;

    size_t first_digit = i;
    unsigned long long value = 0;
    unsigned long long limit = LLONG_MAX;

    for (; str[i] != '\0'; i++) {

        if (str[i] < '0' || str[i] > '9')
            return STATUS_INVALID_NUMBER;

        unsigned long long digit = str[i] - '0';

        if (value > (limit - digit) / 10)
            return STATUS_OVERFLOW;

        value = value * 10 + digit;
    }

    if (flag != 0 && i - first_digit != 2)    
        return STATUS_INVALID_NUMBER;

    if (flag == 1){
        if (value > 23)
            return STATUS_INVALID_HOUR;
        value *= 60;
    }
    if (flag == 2){
        if (value > 59)
            return STATUS_INVALID_MINUTS;
    }
    *num = value;
    return STATUS_OK;
}

Status read_word(FILE *stream, char **word, int stop){
    if (stream == NULL || word == NULL)
        return STATUS_NULL_ARGUMENT;

    *word = NULL;

    int c;
    do {
        c = fgetc(stream);
    } while (c != EOF && isspace((unsigned char)c));

    if (c == EOF)
        return STATUS_INPUT_ERROR;

    size_t capacity = 16;
    size_t length = 0;
    char *buffer = malloc(capacity * sizeof(*buffer));
    if (buffer == NULL)
        return STATUS_MEMORY_ERROR;

    while (c != EOF && !isspace((unsigned char)c) && c != stop){
        if (length + 1 >= capacity){
            if (capacity > (size_t)-1 / 2 / sizeof(*buffer)){
                free(buffer);
                return STATUS_MEMORY_ERROR;
            }
            char *tmp = realloc(buffer, capacity * 2 * sizeof(*buffer));
            if (tmp == NULL){
                free(buffer);
                return STATUS_MEMORY_ERROR;
            }
            buffer = tmp;
            capacity *= 2;
        }
        buffer[length++] = (char)c;
        c = fgetc(stream);
    }

    if (c == EOF && ferror(stream)){
        free(buffer);
        return STATUS_INPUT_ERROR;
    }

    if (stop != 0 && c != stop){              
        free(buffer);
        return STATUS_INVALID_NUMBER;
    }

    buffer[length] = '\0';
    *word = buffer;
    return STATUS_OK;
}

int compare(const void *a, const void *b){
    const Conference *ca = (const Conference *)a;
    const Conference *cb = (const Conference *)b;

    if (ca->finish_time < cb->finish_time)
        return -1;
    if (ca->finish_time > cb->finish_time)
        return 1;
    return 0;
}

int compare_id(const void *a, const void *b){
    const Conference *ca = (const Conference *)a;
    const Conference *cb = (const Conference *)b;

    if (ca->id < cb->id)
        return -1;
    if (ca->id > cb->id)
        return 1;
    return 0;
}

void print_status_error(Status status) {
    switch (status) {
        case STATUS_NULL_ARGUMENT:
            printf("Внутренняя ошибка: введите данные еще раз.\n");
            break;

        case STATUS_OVERFLOW:
            printf("Ошибка: переполнение.\n");
            break;

        case STATUS_EMPTY_STRING:
            printf("Ошибка: передана пустая строка.\n");
            break;

        case STATUS_INVALID_NUMBER:
        case STATUS_NO_DIGITS:
            printf("Ошибка: некорректное число или формат времени (нужно HH:MM).\n");
            break;

        case STATUS_SIGN_ERROR:
            printf("Ошибка: число должно быть неотрицательным.\n");
            break;

        case STATUS_MEMORY_ERROR:
            printf("Ошибка: невозможно выделить память.\n");
            break;

        case STATUS_INPUT_ERROR:
            printf("Ошибка ввода: данных меньше, чем нужно.\n");
            break;

        case STATUS_INVALID_HOUR:
            printf("Ошибка: часы должны быть от 00 до 23.\n");
            break;

        case STATUS_INVALID_MINUTS:
            printf("Ошибка: минуты должны быть от 00 до 59.\n");
            break;

        case STATUS_INVALID_TIME:
            printf("Ошибка: время начала должно быть строго меньше времени окончания.\n");
            break;

        case STATUS_INVALID_ID:
            printf("Ошибка: идентификатор должен быть положительным.\n");
            break;

        case STATUS_DUPLICATE_ID:
            printf("Ошибка: идентификаторы докладов должны быть уникальными.\n");
            break;

        default:
            printf("Ошибка.\n");
            break;
    }
}

int main(void){
    Status st;
    char *w1 = NULL, *w2 = NULL, *w3 = NULL, *w4 = NULL, *w5 = NULL;
    unsigned long long count_conf = 0, count_halls = 0;

    st = read_word(stdin, &w1, 0);
    if (st == STATUS_OK)
        st = read_word(stdin, &w2, 0);
    if (st == STATUS_OK)
        st = check_number(w1, 0, &count_conf);
    if (st == STATUS_OK)
        st = check_number(w2, 0, &count_halls);

    free(w1);
    free(w2);
    w1 = w2 = NULL;

    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }

    if (count_conf == 0 || count_halls == 0){
        printf("Количество конференций и залов должно быть положительным.\n");
        return 1;
    }

    if (count_conf > (size_t)-1 / sizeof(Conference) ||
        count_halls > (size_t)-1 / sizeof(unsigned int)){
        print_status_error(STATUS_OVERFLOW);
        return 1;
    }

    size_t c_conf = (size_t)count_conf;
    size_t c_halls = (size_t)count_halls;

    Conference *conf = malloc(c_conf * sizeof(*conf));
    if (conf == NULL){
        print_status_error(STATUS_MEMORY_ERROR);
        return 1;
    }

    for (size_t i = 0; i < c_conf; i++){
        unsigned long long id = 0;
        unsigned long long hour_start = 0, min_start = 0, hour_finish = 0, min_finish = 0;

                w1 = w2 = w3 = w4 = w5 = NULL;

        st = read_word(stdin, &w1, 0);
        if (st == STATUS_OK) st = check_number(w1, 0, &id);
        if (st == STATUS_OK && id == 0) st = STATUS_INVALID_ID;

        if (st == STATUS_OK) st = read_word(stdin, &w2, ':');
        if (st == STATUS_OK) st = check_number(w2, 1, &hour_start);

        if (st == STATUS_OK) st = read_word(stdin, &w3, 0);
        if (st == STATUS_OK) st = check_number(w3, 2, &min_start);

        if (st == STATUS_OK) st = read_word(stdin, &w4, ':');
        if (st == STATUS_OK) st = check_number(w4, 1, &hour_finish);

        if (st == STATUS_OK) st = read_word(stdin, &w5, 0);
        if (st == STATUS_OK) st = check_number(w5, 2, &min_finish);

        free(w1);
        free(w2);
        free(w3);
        free(w4);
        free(w5);

        if (st == STATUS_OK && id == 0)
            st = STATUS_INVALID_ID;

        if (st != STATUS_OK){
            print_status_error(st);
            free_conf(conf);
            return 1;
        }

        unsigned int s_t = (unsigned int)(hour_start + min_start);
        unsigned int f_t = (unsigned int)(hour_finish + min_finish);
        if (s_t >= f_t){
            print_status_error(STATUS_INVALID_TIME);
            free_conf(conf);
            return 1;
        }

        conf[i].id = id;
        conf[i].start_time = s_t;
        conf[i].finish_time = f_t;
    }

    qsort(conf, c_conf, sizeof(Conference), compare_id);
    for (size_t i = 1; i < c_conf; i++){
        if (conf[i].id == conf[i - 1].id){
            print_status_error(STATUS_DUPLICATE_ID);
            free_conf(conf);
            return 1;
        }
    }

    unsigned int *halls = calloc(c_halls, sizeof(*halls));
    if (halls == NULL){
        print_status_error(STATUS_MEMORY_ERROR);
        free_conf(conf);
        return 1;
    }

    qsort(conf, c_conf, sizeof(Conference), compare);

    size_t result = 0;
    st = search(conf, halls, c_conf, c_halls, &result);
    free(halls);
    free_conf(conf);

    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }

    printf("%zu\n", result);
    return 0;
}