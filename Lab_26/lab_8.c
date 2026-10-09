
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

typedef enum {
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_ARGUMENTS,
    STATUS_INVALID_NUMBER,
    STATUS_OVERFLOW,
    STATUS_MEMORY_ERROR,
    STATUS_INPUT_ERROR,
    STATUS_OUTPUT_ERROR,
    STATUS_FILE_ERROR,
    STATUS_EMPTY_STRING
} Status;


/* Преобразует символ в числовое значение цифры */
Status char_to_digit(const char *symbol, int *digit){
    if (symbol == NULL || digit == NULL)
        return STATUS_NULL_ARGUMENT;

    unsigned char ch = (unsigned char)*symbol;

    if (ch >= '0' && ch <= '9') {
        *digit = ch - '0';
    } else if (ch >= 'A' && ch <= 'Z') {
        *digit = ch - 'A' + 10;
    } else if (ch >= 'a' && ch <= 'z') {
        *digit = ch - 'a' + 10;
    } else {
        return STATUS_INVALID_NUMBER;
    }

    return STATUS_OK;
}


/* Убирает ведущие нули */
Status normalize_number(const char *number, char **normalized){
    if (number == NULL || normalized == NULL)
        return STATUS_NULL_ARGUMENT;

    if (number[0] == '\0')
        return STATUS_INVALID_NUMBER;

    const char *start = number;

    while (*start == '0')
        start++;

    if (*start == '\0')
        start = "0";

    size_t length = strlen(start);

    char *result = malloc(length + 1);
    if (result == NULL)
        return STATUS_MEMORY_ERROR;

    memcpy(result, start, length + 1);

    *normalized = result;

    return STATUS_OK;
}


/* Находит минимальное основание системы счисления */
Status find_base(const char *number, int *base) {
    if (number == NULL || base == NULL)
        return STATUS_NULL_ARGUMENT;

    if (number[0] == '\0')
        return STATUS_INVALID_NUMBER;

    int max_digit = 0;

    for (size_t i = 0; number[i] != '\0'; i++) {
        int digit;
        Status status = char_to_digit(&number[i], &digit);

        if (status != STATUS_OK)
            return status;

        if (digit > max_digit)
            max_digit = digit;
    }

    *base = max_digit + 1;

    if (*base < 2)
        *base = 2;

    return STATUS_OK;
}


/* Переводит число в десятичную систему */
Status to_decimal(const char *number, int base, unsigned long long *decimal){
    if (number == NULL || decimal == NULL)
        return STATUS_NULL_ARGUMENT;

    if (base < 2 || base > 36 || number[0] == '\0')
        return STATUS_INVALID_NUMBER;

    unsigned long long value = 0;

    for (size_t i = 0; number[i] != '\0'; i++) {
        int digit;
        Status status = char_to_digit(&number[i], &digit);

        if (status != STATUS_OK)
            return status;

        if (digit >= base)
            return STATUS_INVALID_NUMBER;

        if (value > (ULLONG_MAX - (unsigned long long)digit)
                    / (unsigned long long)base) {
            return STATUS_OVERFLOW;
        }

        value = value * (unsigned long long)base
              + (unsigned long long)digit;
    }

    *decimal = value;

    return STATUS_OK;
}


/* Увеличивает буфер для чтения очередного числа */
Status resize_buffer(char **buffer, size_t *capacity) {
    if (buffer == NULL || capacity == NULL)
        return STATUS_NULL_ARGUMENT;

    if (*capacity > (size_t)-1 / 2)
        return STATUS_MEMORY_ERROR;

    size_t new_capacity = *capacity * 2;

    char *new_buffer = realloc(*buffer, new_capacity);
    if (new_buffer == NULL)
        return STATUS_MEMORY_ERROR;

    *buffer = new_buffer;
    *capacity = new_capacity;

    return STATUS_OK;
}


/* Читает одно число из файла */
Status read_number(FILE *input, char **number, int *found) {
    if (input == NULL || number == NULL || found == NULL)
        return STATUS_NULL_ARGUMENT;

    *number = NULL;
    *found = 0;

    size_t capacity = 16;
    size_t length = 0;

    char *buffer = malloc(capacity);
    if (buffer == NULL)
        return STATUS_MEMORY_ERROR;

    int ch;

    /* Пропускаем разделители */
    do {
        ch = fgetc(input);

        if (ch == EOF) {
            if (ferror(input)) {
                free(buffer);
                return STATUS_INPUT_ERROR;
            }

            free(buffer);
            return STATUS_OK;
        }
    } while (isspace((unsigned char)ch));

    /* Собираем символы числа */
    while (ch != EOF && !isspace((unsigned char)ch)) {
        if (length + 1 >= capacity) {
            Status status = resize_buffer(&buffer, &capacity);

            if (status != STATUS_OK) {
                free(buffer);
                return status;
            }
        }

        buffer[length++] = (char)ch;
        ch = fgetc(input);
    }

    if (ch == EOF && ferror(input)) {
        free(buffer);
        return STATUS_INPUT_ERROR;
    }

    buffer[length] = '\0';
    *number = buffer;
    *found = 1;

    return STATUS_OK;
}


/* Обрабатывает файл и записывает результаты */
Status process_file(FILE *input, FILE *output) {
    if (input == NULL || output == NULL)
        return STATUS_NULL_ARGUMENT;

    while (1) {
        char *number = NULL;
        char *normalized = NULL;
        int found = 0;

        Status status = read_number(input, &number, &found);

        if (status != STATUS_OK)
            return status;

        if (!found)
            break;

        int base;
        unsigned long long decimal;

        status = find_base(number, &base);

        if (status == STATUS_OK)
            status = normalize_number(number, &normalized);

        if (status == STATUS_OK)
            status = to_decimal(normalized, base, &decimal);

        if (status == STATUS_OK) {
            if (fprintf(output, "%s %d %llu\n", normalized, base, decimal) < 0){
                status = STATUS_OUTPUT_ERROR;
            }
        }

        free(number);
        free(normalized);

        if (status != STATUS_OK)
            return status;
    }

    if (ferror(output))
        return STATUS_OUTPUT_ERROR;

    return STATUS_OK;
}

void print_status_error(Status status) {
    switch (status) {
        case STATUS_NULL_ARGUMENT:
            printf("Ошибка: нулевой указатель.\n");
            break;

        case STATUS_INVALID_ARGUMENTS:
            printf("Ошибка: неверное количество аргументов.\n");
            break;

        case STATUS_INVALID_NUMBER:
            printf("Ошибка: некорректное число.\n");
            break;

        case STATUS_OVERFLOW:
            printf("Ошибка: число слишком большое.\n");
            break;

        case STATUS_MEMORY_ERROR:
            printf("Ошибка: не удалось выделить память.\n");
            break;

        case STATUS_INPUT_ERROR:
            printf("Ошибка чтения входного файла.\n");
            break;

        case STATUS_OUTPUT_ERROR:
            printf("Ошибка записи выходного файла.\n");
            break;

        case STATUS_FILE_ERROR:
            printf("Ошибка открытия файла.\n");
            break;

        case STATUS_OK:
            break;
    }
}


int extension_equal(const char *first, const char *second)
{
    while (*first != '\0' && *second != '\0') {
        if (tolower((unsigned char)*first) != tolower((unsigned char)*second)) {
            return 0;
        }

        first++;
        second++;
    }

    return *first == '\0' && *second == '\0';
}

Status check_file_extension(const char *path)
{
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

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_status_error(STATUS_INVALID_ARGUMENTS);
        printf("Пример допустимого ввода: %s input.txt output.txt\n", argv[0]);
        return 1;
    }

    if (check_file_extension(argv[1]) != STATUS_OK || check_file_extension(argv[2]) != STATUS_OK){
            printf("Ошибка: запрещённое расширение файла.\n");
            return 1;
    }

    FILE *input = fopen(argv[1], "r");

    if (input == NULL) {
        print_status_error(STATUS_FILE_ERROR);
        return 1;
    }

    FILE *output = fopen(argv[2], "w");

    if (output == NULL) {
        print_status_error(STATUS_FILE_ERROR);
        fclose(input);
        return 1;
    }

    Status status = process_file(input, output);

    if (fclose(input) != 0 && status == STATUS_OK)
        status = STATUS_INPUT_ERROR;

    if (fclose(output) != 0 && status == STATUS_OK)
        status = STATUS_OUTPUT_ERROR;

    if (status != STATUS_OK) {
        print_status_error(status);
        return 1;
    }

    return 0;
}