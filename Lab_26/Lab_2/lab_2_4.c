#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

typedef struct{
    char * name_file;
    size_t size_file;
    unsigned long long importance;
} Input_file;

typedef struct{
    Input_file *files;
    size_t count;
    size_t total_size;
    unsigned long long total_importance;
} Backup_result;

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_MEMORY_ERROR,
    STATUS_SIGN_ERROR,
    STATUS_NO_DIGITS,
    STATUS_OVERFLOW,
    STATUS_INVALID_NUMBER,
    STATUS_EMPTY_STRING,
    STATUS_INPUT_ERROR
} Status;

static void free_dp(unsigned long long **dp, size_t rows){
    if (dp == NULL)
        return;
    for (size_t i = 0; i < rows; i++)
        free(dp[i]);
    free(dp);
}

static void free_files(Input_file *files, size_t n){
    if (files == NULL)
        return;
    for (size_t i = 0; i < n; i++)
        free(files[i].name_file);
    free(files);
}

Status search(const Input_file *files, size_t n, size_t capasity, Backup_result *res){
    if (files == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    if (capasity > (size_t)-1 / sizeof(unsigned long long) - 1)
        return STATUS_OVERFLOW;

    Input_file *result = malloc(n * sizeof(Input_file));
    if (result == NULL)
        return STATUS_MEMORY_ERROR;

    unsigned long long **dp = calloc(n + 1, sizeof(unsigned long long*));
    if (dp == NULL){
        free(result);
        return STATUS_MEMORY_ERROR;
    }

    for (size_t i = 0; i < n + 1; i++){
        dp[i] = calloc(capasity + 1, sizeof(unsigned long long));
        if (dp[i] == NULL){
            free_dp(dp, n + 1);
            free(result);
            return STATUS_MEMORY_ERROR;
        }
    }

    for (size_t l = 1; l < n + 1; l++){
        for (size_t w = 1; w < capasity + 1; w++){
            Input_file curr_file = files[l - 1];
            dp[l][w] = dp[l - 1][w];                       // по умолчанию файл не берем
            if (curr_file.size_file <= w){
                if (curr_file.importance + dp[l - 1][w - curr_file.size_file] > dp[l - 1][w])
                    dp[l][w] = curr_file.importance + dp[l - 1][w - curr_file.size_file];
            }
        }
    }

    size_t num_file = n;
    size_t files_size = capasity;
    size_t count_files = 0;
    size_t total_size = 0;
    unsigned long long total_importance = 0;

    while (num_file > 0){
        if (dp[num_file][files_size] != dp[num_file - 1][files_size]){
            result[count_files++] = files[num_file - 1];
            files_size -= files[num_file - 1].size_file;
            total_size += files[num_file - 1].size_file;
            total_importance += files[num_file - 1].importance;
        }
        num_file--;
    }

    for (size_t i = 0; i < count_files / 2; i++){
        Input_file tmp = result[i];
        result[i] = result[count_files - 1 - i];
        result[count_files - 1 - i] = tmp;
    }

    free_dp(dp, n + 1);

    res->files = result;
    res->count = count_files;
    res->total_size = total_size;
    res->total_importance = total_importance;
    return STATUS_OK;
}

Status check_number(const char *str, unsigned long long *num){
    if (str == NULL || num == NULL)
        return STATUS_NULL_ARGUMENT;

    if (str[0] == '\0')
        return STATUS_EMPTY_STRING;

    size_t i = 0;

    if (str[i] == '+' || str[i] == '-') {
        if (str[i] == '-')
            return STATUS_SIGN_ERROR;
        i++;
    }

    if (str[i] == '\0')
        return STATUS_NO_DIGITS;

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
    
    *num = value;
    return STATUS_OK;
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
            printf("Ошибка: некорректное число.\n");
            break;

        case STATUS_SIGN_ERROR:
            printf("Ошибка: число должно быть неотрицательным.\n");
            break;

        case STATUS_MEMORY_ERROR:
            printf("Ошибка: невозможно выделить память.\n");
            break;

        case STATUS_INPUT_ERROR:
            printf("Ошибка ввода.\n");
            break;

        default:
            printf("Ошибка.\n");
            break;
    }
}

Status read_word(FILE *stream, char **word, int *found){
    if (stream == NULL || word == NULL || found == NULL)
        return STATUS_NULL_ARGUMENT;

    *word = NULL;
    *found = 0;

    int c;
    do {
        c = fgetc(stream);
    } while (c != EOF && isspace((unsigned char)c));

    if (c == EOF)
        return ferror(stream) ? STATUS_INPUT_ERROR : STATUS_OK;

    size_t capacity = 16;
    size_t length = 0;
    char *buffer = malloc(capacity);
    if (buffer == NULL)
        return STATUS_MEMORY_ERROR;

    while (c != EOF && !isspace((unsigned char)c)){
        if (length + 1 >= capacity){
            if (capacity > (size_t)-1 / 2){
                free(buffer);
                return STATUS_MEMORY_ERROR;
            }
            char *tmp = realloc(buffer, capacity * 2);
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

    buffer[length] = '\0';
    *word = buffer;
    *found = 1;
    return STATUS_OK;
}

int main(void){
    Status st;
    char *w1 = NULL, *w2 = NULL, *w3 = NULL;
    int found1, found2, found3;
    unsigned long long n_value, cap_value;

    st = read_word(stdin, &w1, &found1);
    if (st == STATUS_OK)
        st = read_word(stdin, &w2, &found2);
    if (st == STATUS_OK && (!found1 || !found2))
        st = STATUS_INPUT_ERROR;

    if (st == STATUS_OK)
        st = check_number(w1, &n_value);
    if (st == STATUS_OK)
        st = check_number(w2, &cap_value);

    free(w1);
    free(w2);
    w1 = w2 = NULL;

    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }

    if (n_value == 0){
        printf("Количество файлов должно быть положительным.\n");
        return 1;
    }

    if (n_value > (size_t)-1 / sizeof(Input_file)){
        print_status_error(STATUS_OVERFLOW);
        return 1;
    }

    size_t n = (size_t)n_value;
    size_t capacity = (size_t)cap_value;

    Input_file *input_files = calloc(n, sizeof(Input_file));
    if (input_files == NULL){
        print_status_error(STATUS_MEMORY_ERROR);
        return 1;
    }

    for (size_t i = 0; i < n; i++){
        unsigned long long size_value, importance_value;

        w1 = w2 = w3 = NULL;
        st = read_word(stdin, &w1, &found1);
        if (st == STATUS_OK)
            st = read_word(stdin, &w2, &found2);
        if (st == STATUS_OK)
            st = read_word(stdin, &w3, &found3);
        if (st == STATUS_OK && (!found1 || !found2 || !found3))
            st = STATUS_INPUT_ERROR;

        if (st == STATUS_OK)
            st = check_number(w2, &size_value);
        if (st == STATUS_OK)
            st = check_number(w3, &importance_value);

        free(w2);
        free(w3);

        if (st == STATUS_OK && size_value == 0){
            printf("Размер файла должен быть положительным.\n");
            free(w1);
            free_files(input_files, n);
            return 1;
        }

        if (st != STATUS_OK){
            print_status_error(st);
            free(w1);
            free_files(input_files, n);
            return 1;
        }

        input_files[i].name_file = w1;         
        input_files[i].size_file = (size_t)size_value;
        input_files[i].importance = importance_value;
    }

    Backup_result res;

    st = search(input_files, n, capacity, &res);
    if (st != STATUS_OK){
        print_status_error(st);
        free_files(input_files, n);
        return 1;
    }

    printf("TOTAL_SIZE %zu\n", res.total_size);
    printf("TOTAL_IMPORTANCE %llu\n", res.total_importance);
    printf("FILES %zu\n", res.count);
    for (size_t i = 0; i < res.count; i++)
        printf("%s\n", res.files[i].name_file);

    free(res.files);
    free_files(input_files, n);
    return 0;
}
