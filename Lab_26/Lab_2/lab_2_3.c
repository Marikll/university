#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_FILE_ERROR,
    STATUS_MEMORY_ERROR,
    STATUS_EMPTY_STRING,
    STATUS_INVALID_ARGUMENTS
} Status;

static size_t string_length(const char *str){
    size_t n = 0;
    while (str[n] != '\0')
        n++;
    return n;
}

static void free_results(char **res, size_t count){
    for (size_t i = 0; i < count; i++)
        free(res[i]);
    free(res);
}

Status prefix_function(const char *s, size_t n, size_t *pi) {
    if (s == NULL || pi == NULL)
        return STATUS_NULL_ARGUMENT;

    pi[0] = 0;

    for (size_t i = 1; i < n; i++) {
        size_t j = pi[i - 1];

        while (j > 0 && s[i] != s[j]) {
            j = pi[j - 1];
        }

        if (s[i] == s[j]) {
            j++;
        }

        pi[i] = j;
    }
    return STATUS_OK;
}

Status kmp_search(const char *text, size_t len_text, FILE *file, const size_t *pi, char ***str_res, size_t *count) {
    if (text == NULL || file == NULL || pi == NULL || str_res == NULL || count == NULL)
        return STATUS_NULL_ARGUMENT;
    if (len_text == 0)
        return STATUS_EMPTY_STRING;

    int column = 1; //счетчик столбцов
    int line = 1; //счетчик строк
    size_t j = 0; //индекс проверяемого элемента в подстроке, которую нужно найти
    size_t found = 0; //счетчик подстрок
    size_t capacity = 100; //размер массива результатов
    size_t pos = 0; //номер текущего символа в файле
    int symbol;

    char **res = malloc(sizeof(*res) * capacity);
    int *lines = malloc(sizeof(int) * len_text);
    int *columns = malloc(sizeof(int) * len_text);
    if (res == NULL || lines == NULL || columns == NULL){
        free(res);
        free(lines);
        free(columns);
        return STATUS_MEMORY_ERROR;
    }

    while ((symbol = fgetc(file)) != EOF){
        lines[pos % len_text] = line;
        columns[pos % len_text] = column;

        while (j > 0 && (unsigned char)text[j] != symbol)
            j = pi[j - 1];

        if ((unsigned char)text[j] == symbol)
            j++;

        if (j == len_text){
            if (found == capacity){
                if (capacity > (size_t)-1 / 2 / sizeof(*res)){
                    free_results(res, found);
                    free(lines);
                    free(columns);
                    return STATUS_MEMORY_ERROR;
                }
                char **tmp = realloc(res, sizeof(*res) * capacity * 2);
                if (tmp == NULL){
                    free_results(res, found);
                    free(lines);
                    free(columns);
                    return STATUS_MEMORY_ERROR;
                }
                res = tmp;
                capacity *= 2;
            }

            size_t start = (pos + 1 - len_text) % len_text; //где лежит позиция начала вхождения
            res[found] = malloc(64);
            if (res[found] == NULL){
                free_results(res, found);
                free(lines);
                free(columns);
                return STATUS_MEMORY_ERROR;
            }
            snprintf(res[found], 64, "line:%d index:%d", lines[start], columns[start]);
            found++;

            j = pi[j - 1];
        }

        if (symbol == '\n'){
            line++;
            column = 1;
        } else if ((symbol & 0xC0) == 0x80){
            column++;
        }
        pos++;
    }

    free(lines);
    free(columns);

    if (ferror(file)){
        free_results(res, found);
        return STATUS_FILE_ERROR;
    }

    *str_res = res;
    *count = found;
    return STATUS_OK;
}

Status search_str(const char *s, size_t n, ...){
    if (s == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t str_len = string_length(s);
    if (str_len == 0)
        return STATUS_EMPTY_STRING;

    size_t *pi = malloc(sizeof(size_t) * str_len);
    if (pi == NULL)
        return STATUS_MEMORY_ERROR;
    Status st_pi = prefix_function(s, str_len, pi);
    if (st_pi != STATUS_OK){
        free(pi);
        return st_pi;
    }

    Status result = STATUS_OK;

    va_list files;
    va_start(files, n);
    for (size_t i = 0; i < n; i++){
        const char *file_path = va_arg(files, const char *);
        if (file_path == NULL){
            result = STATUS_NULL_ARGUMENT;
            break;
        }

        FILE *file_for_search = fopen(file_path, "r");
        if (file_for_search == NULL){
            result = STATUS_FILE_ERROR;
            break;
        }

        char **str_res = NULL;
        size_t count = 0;
        Status st_search = kmp_search(s, str_len, file_for_search, pi, &str_res, &count);
        fclose(file_for_search);

        if (st_search != STATUS_OK){
            result = st_search;
            break;
        }

        printf("%s: ", file_path);
        if (count == 0)
            printf("вхождений не найдено");
        for (size_t k = 0; k < count; k++)
            printf("(%s); ", str_res[k]);
        printf("\n");

        free_results(str_res, count);
    }
    va_end(files);

    free(pi);
    return result;
}

int extension_equal(const char *first, const char *second){
    while (*first != '\0' && *second != '\0') {
        if (tolower((unsigned char)*first) != tolower((unsigned char)*second)) {
            return 0;
        }

        first++;
        second++;
    }

    return *first == '\0' && *second == '\0';
}

Status check_file_extension(const char *path){
    if (path == NULL)
        return STATUS_NULL_ARGUMENT;

    if (path[0] == '\0')
        return STATUS_EMPTY_STRING;

    const char *name = path;

    for (const char *p = path; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\')
            name = p + 1;
    }

    const char *dot = strrchr(name, '.');

    if (dot == NULL)
        return STATUS_OK;

    const char *forbidden[] = {
        ".png", ".jpg", ".jpeg", ".gif",
        ".bmp", ".webp", ".tif", ".tiff",
        ".ico", ".pdf",
        ".zip", ".rar", ".7z", ".gz", ".tar",
        ".exe", ".dll", ".so",
        ".mp3", ".wav", ".flac",
        ".mp4", ".avi", ".mkv", ".mov",
        ".docx", ".xlsx", ".pptx",
        ".odt", ".ods", ".odp"
    };

    int count = sizeof(forbidden) / sizeof(forbidden[0]);

    for (int i = 0; i < count; i++) {
        if (extension_equal(dot, forbidden[i]))
            return STATUS_INVALID_ARGUMENTS;
    }

    return STATUS_OK;
}

int main(int argc, char *argv[]){
    if (argc < 4){
        printf("Введите подстроку, количество файлов (от 1 до 5) и пути к файлам.\n");
        return 1;
    }

    if (argv[2][0] < '1' || argv[2][0] > '5' || argv[2][1] != '\0'){
        printf("Ошибка: количество файлов должно быть числом от 1 до 5.\n");
        return 1;
    }
    int n = argv[2][0] - '0';

    if (argc != 3 + n){
        printf("Ошибка: количество переданных файлов не соответствует второму аргументу.\n");
        return 1;
    }

    for (int i = 3; i < argc; i++){
        if (check_file_extension(argv[i]) != STATUS_OK){
            printf("Ошибка: запрещённое расширение файла.\n");
            return 1;
        }
    }

    Status st;
    switch (n){
        case 1:
            st = search_str(argv[1], 1, argv[3]);
            break;
        case 2:
            st = search_str(argv[1], 2, argv[3], argv[4]);
            break;
        case 3:
            st = search_str(argv[1], 3, argv[3], argv[4], argv[5]);
            break;
        case 4:
            st = search_str(argv[1], 4, argv[3], argv[4], argv[5], argv[6]);
            break;
        default:
            st = search_str(argv[1], 5, argv[3], argv[4], argv[5], argv[6], argv[7]);
            break;
    }

    if (st != STATUS_OK){
        printf("Ошибка, код %d\n", st);
        return 1;
    }

    return 0;
}