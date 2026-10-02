#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_FLAG,
    STATUS_EMPTY_STRING,

    STATUS_FLAG_ND,
    STATUS_FLAG_NI,
    STATUS_FLAG_NS,
    STATUS_FLAG_NA,

    STATUS_FLAG_D,
    STATUS_FLAG_I,
    STATUS_FLAG_S,
    STATUS_FLAG_A,

    STATUS_INVALID_ARGUMENTS,
    STATUS_INPUT_ERROR,
    STATUS_OUTPUT_ERROR,
    STATUS_MEMORY_ERROR,
    STATUS_FILE_ERROR
} Status;


Status flag_d(FILE * input, FILE* output){
    if (input == NULL || output == NULL)
        return STATUS_NULL_ARGUMENT;

    int symbol;
    while ((symbol = getc(input)) != EOF){
        if (symbol > '9' || symbol < '0'){
            if (fputc(symbol, output) == EOF)
                return STATUS_OUTPUT_ERROR;
        }
    }

    if (ferror(input))
        return STATUS_INPUT_ERROR;

    return STATUS_OK;
}

Status flag_i(FILE * input, FILE * output){
    if (input == NULL || output == NULL)
        return STATUS_NULL_ARGUMENT;

    int symbol;
    int count = 0;
    int flag = 0;
    while ((symbol = fgetc(input)) != EOF){
        if (symbol == '\n'){
            if(fprintf( output, "%d\n", count) < 0)
                return STATUS_OUTPUT_ERROR;
            count = 0;
            flag = 1;
        } else {
            if ((symbol >= 'A' && symbol <= 'Z') || (symbol >= 'a' && symbol <= 'z'))
                count++;
            flag = 0;
        }
    }

    if (ferror(input))
        return STATUS_INPUT_ERROR;
    if (!flag){
        if(fprintf( output, "%d", count) < 0)
            return STATUS_OUTPUT_ERROR;
    }
    return STATUS_OK;
}

Status flag_s(FILE * input, FILE * output){
    if (input == NULL || output == NULL)
        return STATUS_NULL_ARGUMENT;

    int symbol;
    int count = 0;
    while ((symbol = fgetc(input)) != EOF){
        if (symbol == '\n'){
            if(fprintf( output, "%d\n", count) < 0)
                return STATUS_OUTPUT_ERROR;
            count = 0;
        } else { 
            if ((symbol < 'A' || symbol > 'Z') && (symbol < 'a' || symbol > 'z') && (symbol < '0' || symbol > '9') && symbol != ' ')
                count++;
        }
    }

    if (ferror(input))
        return STATUS_INPUT_ERROR;

    if(fprintf( output, "%d", count) < 0)
        return STATUS_OUTPUT_ERROR;

    return STATUS_OK;
}

Status flag_a(FILE* input, FILE* output){
    if (input == NULL || output == NULL)
        return STATUS_NULL_ARGUMENT;
    int symbol;
   
    char hex[] = "0123456789ABCDEF";
    while((symbol = fgetc(input)) != EOF){
        if ( symbol < '0' || symbol > '9'){
            int high = symbol/16;
            int low = symbol%16;
            if (fputc(hex[high], output) == EOF)
                return STATUS_OUTPUT_ERROR;
            if (fputc(hex[low], output) == EOF)
                return STATUS_OUTPUT_ERROR;
        } else{
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
    if (str[1] == 'n'){
        if (str[3] != '\0')
            return STATUS_INVALID_FLAG;
        if (str[2] == '\0')
            return STATUS_INVALID_FLAG;

        switch(str[2]){
            case 'd':
                return STATUS_FLAG_ND;
            case 'i':
                return STATUS_FLAG_NI;
            case 's':
                return STATUS_FLAG_NS;
            case 'a':
                return STATUS_FLAG_NA;
            default:
                return STATUS_INVALID_FLAG;
        }
    }
    if (str[2] != '\0')
        return STATUS_INVALID_FLAG;
    if (str[1] == '\0')
        return STATUS_INVALID_FLAG;
    switch (str[1]){
        case 'd':
            return STATUS_FLAG_D;
        case 'i':
            return STATUS_FLAG_I;
        case 's':
            return STATUS_FLAG_S;
        case 'a':
            return STATUS_FLAG_A;
        default:
            return STATUS_INVALID_FLAG;
    }
}

Status generate_output_path(const char *input_path, char **output_path) {
    if (input_path == NULL || output_path == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t length = strlen(input_path);

    if (length == 0)
        return STATUS_EMPTY_STRING;

    const char *last_slash = strrchr(input_path, '/');
    const char *last_backslash = strrchr(input_path, '\\');

    const char *last_separator = last_slash;

    if (last_backslash != NULL && (last_separator == NULL || last_backslash > last_separator)) {
        last_separator = last_backslash;
    }

    size_t prefix_length;

    if (last_separator == NULL) {
        prefix_length = 0;
    } else {
        prefix_length = (size_t)(last_separator - input_path + 1);
    }

    const char *filename = input_path + prefix_length;
    size_t filename_length = strlen(filename);

    if (filename_length == 0)
        return STATUS_INVALID_ARGUMENTS;

    size_t result_length = prefix_length + 4 + filename_length + 1;

    char *result = malloc(result_length);

    if (result == NULL)
        return STATUS_MEMORY_ERROR;

    if (prefix_length > 0)
        memcpy(result, input_path, prefix_length);

    memcpy(result + prefix_length, "out_", 4);

    memcpy(result + prefix_length + 4, filename, filename_length + 1);

    *output_path = result;

    return STATUS_OK;
}


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


int main(int argc, char *argv[]){
    if (argc < 2 || argc > 4){
        printf("Введите флаг и соответствующее ему количество чисел.\n");
        return 1;
    }

    /*прописать функцию для проверки флага, проверить статус, а после смотреть на путь/пути к файлу/файлам.*/
    Status st_flag = check_flag(argv[1]);

    if (st_flag == STATUS_NULL_ARGUMENT || st_flag == STATUS_INVALID_FLAG || st_flag == STATUS_EMPTY_STRING){
        print_status_error(st_flag);
        return 1;
    }
    const char *input_path;
    const char *output_path;
    char * generated_output = NULL;
    if (st_flag == STATUS_FLAG_NA || st_flag == STATUS_FLAG_ND || st_flag == STATUS_FLAG_NI || st_flag == STATUS_FLAG_NS){
        if (argc != 4){
            printf("Ошибка: для флага с n необходимо ввести входной и выходной файлы.\n");
            return 1;
        }
        input_path = argv[2];
        output_path = argv[3];
    } else{
        if (argc != 3){
            printf("Ошибка: для флага необходимо ввести входной файл.\n");
            return 1;
        }
        input_path = argv[2];
        Status st_path = generate_output_path(input_path, &generated_output);

        if (st_path != STATUS_OK) {
            print_status_error(st_path);
            return 1;
        }

        output_path = generated_output;
    }

    if (strcmp(input_path, output_path) == 0) {
            printf("Ошибка: входной и выходной файлы совпадают.\n");
            free(generated_output);
            return 1;
    }

    FILE* input = fopen(argv[2], "r");
    if (input == NULL){
        printf("Ошибка: не удалось открыть входной файл.\n");
        free(generated_output);
        return 1;
    }
    printf("output_path = [%s]\n", output_path);
    FILE* output = fopen(output_path, "w");
    if (output == NULL){
        printf("Ошибка: не удалось открыть выходной файл.\n");
        fclose(input);
        free(generated_output);
        return 1;
    }

    Status operation_status;
    switch (st_flag) {
        case STATUS_FLAG_D:
        case STATUS_FLAG_ND:
            operation_status = flag_d(input, output);
            break;

        case STATUS_FLAG_I:
        case STATUS_FLAG_NI:
            operation_status = flag_i(input, output);
            break;

        case STATUS_FLAG_S:
        case STATUS_FLAG_NS:
            operation_status = flag_s(input, output);
            break;

        case STATUS_FLAG_A:
        case STATUS_FLAG_NA:
            operation_status = flag_a(input, output);
            break;

        default:
            operation_status = STATUS_INVALID_FLAG;
            break;
    }

     int input_close_result = fclose(input);
    int output_close_result = fclose(output);

    if (operation_status != STATUS_OK) {
        print_status_error(operation_status);
        free(generated_output);
        return 1;
    }

    if (input_close_result != 0 ||
        output_close_result != 0) {
        printf("Ошибка закрытия файла.\n");
        free(generated_output);
        return 1;
    }

    free(generated_output);

    return 0;
}
