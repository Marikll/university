#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <sys/stat.h>

#define SALARY_EPS 1e-9

typedef struct {
    unsigned long long id;
    char *name;
    char *lastname;
    double zp;
} Employee;

typedef enum {
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_MEMORY_ERROR,
    STATUS_FILE_ERROR,
    STATUS_EMPTY_STRING,
    STATUS_INVALID_FLAG,
    STATUS_FLAG_A,
    STATUS_FLAG_D,
    STATUS_INVALID_ARGUMENTS,
    STATUS_INVALID_STRING,
    STATUS_INVALID_ID,
    STATUS_INVALID_NAME,
    STATUS_INVALID_SALARY,
    STATUS_OVERFLOW,
    STATUS_SAME_FILE
} Status;

void print_error(Status status){
    switch (status) {
        case STATUS_NULL_ARGUMENT:   printf("Внутренняя ошибка: нулевой указатель.\n"); break;
        case STATUS_MEMORY_ERROR:    printf("Ошибка: не удалось выделить память.\n"); break;
        case STATUS_FILE_ERROR:      printf("Ошибка работы с файлом.\n"); break;
        case STATUS_EMPTY_STRING:    printf("Ошибка: передана пустая строка.\n"); break;
        case STATUS_INVALID_FLAG:    printf("Ошибка: некорректный флаг (допустимо -a, -d, /a, /d).\n"); break;
        case STATUS_INVALID_ARGUMENTS: printf("Ошибка: запрещённое расширение файла.\n"); break;
        case STATUS_INVALID_STRING:  printf("Ошибка: в строке файла должно быть ровно 4 поля.\n"); break;
        case STATUS_INVALID_ID:      printf("Ошибка: некорректный id.\n"); break;
        case STATUS_INVALID_NAME:    printf("Ошибка: имя и фамилия должны состоять только из латинских букв.\n"); break;
        case STATUS_INVALID_SALARY:  printf("Ошибка: некорректная зарплата.\n"); break;
        case STATUS_OVERFLOW:        printf("Ошибка: переполнение.\n"); break;
        default:                     printf("Ошибка.\n"); break;
    }
}

static int is_blank(int c){
    return c == ' ' || c == '\t' || c == '\r';
}

static int is_latin(char c){
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

/*чтение строки любой длины */
Status read_line(FILE *file, char **line, int *end){
    if (file == NULL || line == NULL || end == NULL)
        return STATUS_NULL_ARGUMENT;

    *line = NULL;
    *end = 0;

    size_t capacity = 64;
    size_t length = 0;
    char *buffer = malloc(capacity);
    if (buffer == NULL)
        return STATUS_MEMORY_ERROR;

    int c;
    while ((c = fgetc(file)) != EOF && c != '\n') {
        if (length + 1 >= capacity) {
            if (capacity > (size_t)-1 / 2) {
                free(buffer);
                return STATUS_MEMORY_ERROR;
            }
            char *tmp = realloc(buffer, capacity * 2);
            if (tmp == NULL) {
                free(buffer);
                return STATUS_MEMORY_ERROR;
            }
            buffer = tmp;
            capacity *= 2;
        }
        buffer[length++] = (char)c;
    }

    if (c == EOF && ferror(file)) {
        free(buffer);
        return STATUS_FILE_ERROR;
    }

    if (c == EOF && length == 0) {
        free(buffer);
        *end = 1;
        return STATUS_OK;
    }

    buffer[length] = '\0';
    *line = buffer;
    return STATUS_OK;
}

/* делит строку на 4 слова*/
Status split_line(const char *line, const char *start[4], size_t length[4]){
    if (line == NULL || start == NULL || length == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t i = 0;
    int count = 0;

    while (line[i] != '\0') {
        while (is_blank((unsigned char)line[i]))
            i++;
        if (line[i] == '\0')
            break;
        if (count == 4)
            return STATUS_INVALID_STRING;

        start[count] = &line[i];
        size_t n = 0;
        while (line[i] != '\0' && !is_blank((unsigned char)line[i])) {
            i++;
            n++;
        }
        length[count] = n;
        count++;
    }

    if (count != 4)
        return STATUS_INVALID_STRING;
    return STATUS_OK;
}

Status parse_id(const char *s, size_t n, unsigned long long *id){
    if (s == NULL || id == NULL)
        return STATUS_NULL_ARGUMENT;
    if (n == 0)
        return STATUS_INVALID_ID;

    unsigned long long value = 0;
    for (size_t i = 0; i < n; i++) {
        if (s[i] < '0' || s[i] > '9')
            return STATUS_INVALID_ID;
        unsigned long long digit = (unsigned long long)(s[i] - '0');
        if (value > (ULLONG_MAX - digit) / 10)
            return STATUS_OVERFLOW;
        value = value * 10 + digit;
    }
    *id = value;
    return STATUS_OK;
}

Status parse_salary(const char *s, size_t n, double *salary){
    if (s == NULL || salary == NULL)
        return STATUS_NULL_ARGUMENT;

    double value = 0.0, fraction = 0.0, divider = 10.0;
    int point = 0;
    size_t digits = 0;

    for (size_t i = 0; i < n; i++) {
        if (s[i] == '.' || s[i] == ',') {
            if (point)
                return STATUS_INVALID_SALARY;
            point = 1;
            continue;
        }
        if (s[i] < '0' || s[i] > '9')
            return STATUS_INVALID_SALARY;

        int digit = s[i] - '0';
        digits++;

        if (!point) {
            if (value > (DBL_MAX - digit) / 10.0)
                return STATUS_OVERFLOW;
            value = value * 10.0 + digit;
        } else {
            fraction += digit / divider;
            divider *= 10.0;
        }
    }

    if (digits == 0)
        return STATUS_INVALID_SALARY;

    *salary = value + fraction;
    return STATUS_OK;
}

Status copy_name(const char *s, size_t n, char **out){
    if (s == NULL || out == NULL)
        return STATUS_NULL_ARGUMENT;
    if (n == 0)
        return STATUS_INVALID_NAME;
    for (size_t i = 0; i < n; i++)
        if (!is_latin(s[i]))
            return STATUS_INVALID_NAME;

    char *copy = malloc(n + 1);
    if (copy == NULL)
        return STATUS_MEMORY_ERROR;
    memcpy(copy, s, n);
    copy[n] = '\0';

    *out = copy;
    return STATUS_OK;
}

void free_employee(Employee *e){
    if (e == NULL)
        return;
    free(e->name);
    free(e->lastname);
    e->name = NULL;
    e->lastname = NULL;
}

void free_array(Employee *array, size_t count){
    if (array == NULL)
        return;
    for (size_t i = 0; i < count; i++)
        free_employee(&array[i]);
    free(array);
}

/* разбирает одну строку файла в структуру*/
Status parse_employee(const char *line, Employee *emp){
    if (line == NULL || emp == NULL)
        return STATUS_NULL_ARGUMENT;

    const char *start[4];
    size_t length[4];

    Status st = split_line(line, start, length);
    if (st != STATUS_OK)
        return st;

    unsigned long long id;
    double zp;

    st = parse_id(start[0], length[0], &id);
    if (st != STATUS_OK)
        return st;

    st = parse_salary(start[3], length[3], &zp);
    if (st != STATUS_OK)
        return st;

    char *name = NULL;
    char *lastname = NULL;

    st = copy_name(start[1], length[1], &name);
    if (st != STATUS_OK)
        return st;

    st = copy_name(start[2], length[2], &lastname);
    if (st != STATUS_OK) {
        free(name);
        return st;
    }

    emp->id = id;
    emp->name = name;
    emp->lastname = lastname;
    emp->zp = zp;
    return STATUS_OK;
}

/* чтение файла в динамический массив*/
Status create_array(FILE *file, Employee **result, size_t *count){
    if (file == NULL || result == NULL || count == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t capacity = 16;
    size_t size = 0;

    Employee *array = malloc(capacity * sizeof(*array));
    if (array == NULL)
        return STATUS_MEMORY_ERROR;

    while (1) {
        char *line = NULL;
        int end = 0;

        Status st = read_line(file, &line, &end);
        if (st != STATUS_OK) {
            free_array(array, size);
            return st;
        }
        if (end)
            break;

        size_t k = 0;
        while (is_blank((unsigned char)line[k]))
            k++;
        if (line[k] == '\0') {
            free(line);
            continue;
        }

        if (size == capacity) {
            if (capacity > (size_t)-1 / 2 / sizeof(*array)) {
                free(line);
                free_array(array, size);
                return STATUS_MEMORY_ERROR;
            }
            Employee *tmp = realloc(array, capacity * 2 * sizeof(*array));
            if (tmp == NULL) {
                free(line);
                free_array(array, size);
                return STATUS_MEMORY_ERROR;
            }
            array = tmp;
            capacity *= 2;
        }

        st = parse_employee(line, &array[size]);
        free(line);

        if (st != STATUS_OK) {
            free_array(array, size);
            return st;
        }
        size++;
    }

    *result = array;
    *count = size;
    return STATUS_OK;
}

/* сортировка по возрастанию*/
int compare_a(const void *a, const void *b){
    const Employee *ea = (const Employee *)a;
    const Employee *eb = (const Employee *)b;

    double diff = ea->zp - eb->zp;
    if (fabs(diff) > SALARY_EPS)
        return diff < 0 ? -1 : 1;

    int cmp = strcmp(ea->lastname, eb->lastname);
    if (cmp != 0)
        return cmp;

    cmp = strcmp(ea->name, eb->name);
    if (cmp != 0)
        return cmp;

    if (ea->id != eb->id)
        return ea->id < eb->id ? -1 : 1;

    return 0;
}

/* сортировка по убыванию*/
int compare_d(const void *a, const void *b)
{
    return compare_a(b, a);
}

/* запись в выходной файл*/
Status write_array(FILE *output, const Employee *array, size_t count){
    if (output == NULL || array == NULL)
        return STATUS_NULL_ARGUMENT;

    for (size_t i = 0; i < count; i++) {
        if (fprintf(output, "%llu %s %s %.2f\n", array[i].id, array[i].name, array[i].lastname, array[i].zp) < 0)
            return STATUS_FILE_ERROR;
    }
    return STATUS_OK;
}

Status check_flag(const char *str){
    if (str == NULL)
        return STATUS_NULL_ARGUMENT;
    if (str[0] == '\0')
        return STATUS_EMPTY_STRING;
    if ((str[0] != '-' && str[0] != '/') || str[1] == '\0' || str[2] != '\0')
        return STATUS_INVALID_FLAG;

    switch (str[1]) {
        case 'a': return STATUS_FLAG_A;
        case 'd': return STATUS_FLAG_D;
        default:  return STATUS_INVALID_FLAG;
    }
}

int same_files(const char *path1, const char *path2)
{
    struct stat s1, s2;

    if (path1 == NULL || path2 == NULL)
        return 0;

    if (stat(path1, &s1) != 0 || stat(path2, &s2) != 0)
        return 0;

    return s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino;
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
    if (argc != 4) {
        printf("Ошибка: введите входной файл, флаг (-a/-d) и выходной файл.\n");
        return 1;
    }

    Status flag = check_flag(argv[2]);
    if (flag != STATUS_FLAG_A && flag != STATUS_FLAG_D) {
        print_error(flag);
        return 1;
    }

    if (same_files(argv[1], argv[3])) {
        printf("Ошибка: входной и выходной файлы совпадают.\n");
        return 1;
    }

    if (check_file_extension(argv[1]) != STATUS_OK || check_file_extension(argv[3]) != STATUS_OK) {
        printf("Ошибка: запрещённое расширение файла.\n");
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (input == NULL) {
        printf("Ошибка: не удалось открыть входной файл.\n");
        return 1;
    }

    Employee *array = NULL;
    size_t count = 0;

    Status status = create_array(input, &array, &count);
    int input_close = fclose(input);

    if (status != STATUS_OK) {
        print_error(status);
        return 1;
    }
    if (input_close != 0) {
        printf("Ошибка закрытия входного файла.\n");
        free_array(array, count);
        return 1;
    }

    qsort(array, count, sizeof(Employee), flag == STATUS_FLAG_A ? compare_a : compare_d);

    FILE *output = fopen(argv[3], "w");
    if (output == NULL) {
        printf("Ошибка: не удалось открыть выходной файл.\n");
        free_array(array, count);
        return 1;
    }

    status = write_array(output, array, count);
    int output_close = fclose(output);
    free_array(array, count);

    if (status != STATUS_OK) {
        print_error(status);
        return 1;
    }
    if (output_close != 0) {
        printf("Ошибка закрытия выходного файла.\n");
        return 1;
    }

    return 0;
}