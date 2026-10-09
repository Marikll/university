#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_FLAG,
    STATUS_EMPTY_STRING,
    STATUS_FLAG_R,
    STATUS_FLAG_A,
    STATUS_INVALID_ARGUMENTS,
    STATUS_INPUT_ERROR,
    STATUS_OUTPUT_ERROR,
    STATUS_MEMORY_ERROR,
    STATUS_FILE_ERROR
} Status;

void print_status_error(Status status) {
    switch (status) {
        case STATUS_NULL_ARGUMENT:
            printf("Внутренняя ошибка: введите данные еще раз.\n");
            break;

        case STATUS_INVALID_FLAG:
            printf("Ошибка: неизвестный флаг.\n");
            break;

        case STATUS_EMPTY_STRING:
            printf("Ошибка: передана пустая строка.\n");
            break;

        case STATUS_INVALID_ARGUMENTS:
            printf("Ошибка: неверное количество или значение аргументов.\n");
            break;

        case STATUS_INPUT_ERROR:
            printf("Ошибка чтения входного файла.\n");
            break;

        case STATUS_OUTPUT_ERROR:
            printf("Ошибка записи в выходной файл.\n");
            break;

        case STATUS_MEMORY_ERROR:
            printf("Ошибка: невозможно выделить память.\n");
            break;

        case STATUS_FILE_ERROR:
            printf("Ошибка открытия файла.\n");
            break;

        default:
            printf("Ошибка.\n");
            break;
    }
}

Status flag_r(FILE *file1, FILE *file2, FILE *output){
    if (file1 == NULL || file2 == NULL || output == NULL)
        return STATUS_NULL_ARGUMENT;

    int symbol1;
    int symbol2;

    symbol1 = getc(file1);
    symbol2 = getc(file2);
    while (symbol1 != EOF && symbol2 != EOF){
        while (symbol1 == ' ' || symbol1 == '\t' || symbol1 == '\n' || symbol1 == '\r')
            symbol1 = getc(file1);

        while (symbol1 != EOF && symbol1 != ' ' && symbol1 != '\t' && symbol1 != '\n' && symbol1 != '\r'){
            if (fputc(symbol1, output) == EOF)
                return STATUS_OUTPUT_ERROR;
            symbol1 = getc(file1);
        }

        if (fputc(' ', output) == EOF)
            return STATUS_OUTPUT_ERROR;

        while (symbol2 == ' ' || symbol2 == '\t' || symbol2 == '\n' || symbol2 == '\r')
            symbol2 = getc(file2);

        while (symbol2 != EOF && symbol2 != ' ' && symbol2 != '\t' && symbol2 != '\n' && symbol2 != '\r'){
            if (fputc(symbol2, output) == EOF)
                return STATUS_OUTPUT_ERROR;
            symbol2 = getc(file2);
        }

        if (fputc(' ', output) == EOF)
            return STATUS_OUTPUT_ERROR;
    }

    if (symbol1 != EOF){
        while (symbol1 != EOF){
            while (symbol1 == ' ' || symbol1 == '\t' || symbol1 == '\n' || symbol1 == '\r')
                symbol1 = getc(file1);

            while (symbol1 != EOF && symbol1 != ' ' && symbol1 != '\t' && symbol1 != '\n' && symbol1 != '\r'){
                if (fputc(symbol1, output) == EOF)
                    return STATUS_OUTPUT_ERROR;
                symbol1 = getc(file1);
            }
            if (fputc(' ', output) == EOF)
                return STATUS_OUTPUT_ERROR;
        }
    } else if (symbol2 != EOF){
        while (symbol2 != EOF){
            while (symbol2 == ' ' || symbol2 == '\t' || symbol2 == '\n' || symbol2 == '\r')
                symbol2 = getc(file2);

            while (symbol2 != EOF && symbol2 != ' ' && symbol2 != '\t' && symbol2 != '\n' && symbol2 != '\r'){
                if (fputc(symbol2, output) == EOF)
                    return STATUS_OUTPUT_ERROR;
                symbol2 = getc(file2);
            }
            if (fputc(' ', output) == EOF)
                return STATUS_OUTPUT_ERROR;
        }
    }

    if (ferror(file1))
        return STATUS_INPUT_ERROR;

    if (ferror(file2))
        return STATUS_INPUT_ERROR;

    return STATUS_OK;
}

int is_separator(int symbol){
    return symbol == ' ' || symbol == '\t' || symbol == '\n' || symbol == '\r';
}

int to_lower_symbol(int symbol){
    if (symbol >= 'A' && symbol <= 'Z')
        return symbol - 'A' + 'a';
    return symbol;
}

Status write_base(FILE *output, unsigned int value, int base, int *output_started){
    if (output == NULL || output_started == NULL)
        return STATUS_NULL_ARGUMENT;

    char digits[] = "0123456789ABCDEF";
    char reversed[16];
    int count = 0;

    do {
        reversed[count] = digits[value % base];
        count++;
        value /= base;
    } while (value > 0);

    if (*output_started) {
        if (fputc(' ', output) == EOF)
            return STATUS_OUTPUT_ERROR;
    }

    for (int i = count - 1; i >= 0; i--) {
        if (fputc(reversed[i], output) == EOF)
            return STATUS_OUTPUT_ERROR;
    }

    *output_started = 1;

    return STATUS_OK;
}

Status flag_a(FILE *input, FILE *output)
{
    if (input == NULL || output == NULL)
        return STATUS_NULL_ARGUMENT;

    int symbol;
    int token_number = 0;
    int in_token = 0;
    int output_started = 0;

    int token_base = 0;
    int lowercase = 0;

    while ((symbol = getc(input)) != EOF) {

        if (is_separator(symbol)) {
            in_token = 0;
            continue;
        }

        if (!in_token) {
            in_token = 1;
            token_number++;

            token_base = 0;
            lowercase = 0;

            if (token_number % 10 == 0) {
                token_base = 4;
                lowercase = 1;
            } else if (token_number % 5 == 0) {
                token_base = 8;
            } else if (token_number % 2 == 0) {
                lowercase = 1;
            }

            if (token_base == 0) {
                if (output_started) {
                    if (fputc(' ', output) == EOF)
                        return STATUS_OUTPUT_ERROR;
                }

                output_started = 1;
            }
        }

        if (lowercase)
            symbol = to_lower_symbol(symbol);

        if (token_base != 0) {
            Status st = write_base(
                output, (unsigned char)symbol,
                token_base, &output_started
            );

            if (st != STATUS_OK)
                return st;
        } else {
            if (fputc(symbol, output) == EOF)
                return STATUS_OUTPUT_ERROR;
        }
    }

    if (ferror(input))
        return STATUS_INPUT_ERROR;

    return STATUS_OK;
}

Status check_flag(const char * str){
    if (str == NULL)
        return STATUS_NULL_ARGUMENT;
    if (str[0] != '-' && str[0] != '/')
        return STATUS_INVALID_FLAG;
    if (str[1] == '\0')
        return STATUS_INVALID_FLAG;
    if (str[2] != '\0')
        return STATUS_INVALID_FLAG;
    switch (str[1]){
        case 'r':
            return STATUS_FLAG_R;
        case 'a':
            return STATUS_FLAG_A;
        default:
            return STATUS_INVALID_FLAG;
    }
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
    if (argc < 4 || argc > 5){
        printf("Введите флаг и соответствующее ему количество чисел.\n");
        return 1;
    }

    /*прописать функцию для проверки флага, проверить статус, а после смотреть на путь/пути к файлу/файлам.*/
    Status st_flag = check_flag(argv[1]);

    if (st_flag == STATUS_NULL_ARGUMENT || st_flag == STATUS_INVALID_FLAG || st_flag == STATUS_EMPTY_STRING){
        print_status_error(st_flag);
        return 1;
    }
    const char *input1_path;
    const char *input2_path;
    const char *output_path;

    if (st_flag == STATUS_FLAG_R){
        if (argc != 5){
            printf("Ошибка: для флага -r необходимо ввести два входных файла и один выходной файл.\n");
            return 1;
        }
        input1_path = argv[2];
        input2_path = argv[3];
        output_path = argv[4];

        if (strcmp(input1_path, output_path) == 0){
            printf("Ошибка: первый входной файл и выходной файл совпадают.\n");
            return 1;
        } 
        if (strcmp(input2_path, output_path) == 0){
            printf("Ошибка: второй входной файл и выходной файл совпадают.\n");
            return 1;
        }

        if (check_file_extension(input1_path) != STATUS_OK || check_file_extension(input2_path) != STATUS_OK || check_file_extension(output_path) != STATUS_OK){
            printf("Ошибка: запрещённое расширение файла.\n");
            return 1;
        }

        FILE* input1 = fopen(argv[2], "r");
        if (input1 == NULL){
            printf("Ошибка: не удалось открыть первый входной файл.\n");
            return 1;
        }
        FILE* input2 = fopen(argv[3], "r");
        if (input2 == NULL){
            printf("Ошибка: не удалось открыть второй входной файл.\n");
            fclose(input1);
            return 1;
        }

        printf("output_path = [%s]\n", output_path);
    
        FILE* output = fopen(output_path, "w");
        if (output == NULL){
            printf("Ошибка: не удалось открыть выходной файл.\n");
            fclose(input1);
            fclose(input2);
            return 1;
        }

        Status st_r = flag_r(input1, input2, output);

        int input1_close_result = fclose(input1);
        int input2_close_result = fclose(input2);
        int output_close_result = fclose(output);

        if (st_r != STATUS_OK) {
            print_status_error(st_r);
            return 1;
        }

        if (input1_close_result != 0 || input2_close_result != 0 || output_close_result != 0){
            printf("Ошибка закрытия файла.\n");
            return 1;
        }

    } else if (st_flag == STATUS_FLAG_A){
        if (argc != 4){
            printf("Ошибка: для флага -a необходимо ввести один входной и один выходной файлы.\n");
            return 1;
        }
        input1_path = argv[2];
        output_path = argv[3];
        if (strcmp(input1_path, output_path) == 0){
            printf("Ошибка: входной и выходной файлы совпадают.\n");
            return 1;
        }

        if (check_file_extension(input1_path) != STATUS_OK || check_file_extension(output_path) != STATUS_OK){
            printf("Ошибка: запрещённое расширение файла.\n");
            return 1;
        }

        FILE* input = fopen(argv[2], "r");
        if (input == NULL){
            printf("Ошибка: не удалось открыть входной файл.\n");
            return 1;
        }

        printf("output_path = [%s]\n", output_path);
    
        FILE* output = fopen(output_path, "w");
        if (output == NULL){
            printf("Ошибка: не удалось открыть выходной файл.\n");
            fclose(input);
            return 1;
        }

        Status st_a = flag_a(input, output);

        int input_close_result = fclose(input);
        int output_close_result = fclose(output);

        if (st_a != STATUS_OK) {
            print_status_error(st_a);
            return 1;
        }

        if (input_close_result != 0 || output_close_result != 0) {
            printf("Ошибка закрытия файла.\n");
            return 1;
        }

    }

    return 0;
}
