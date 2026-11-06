#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>

#define EPS 1e-9

typedef struct{
    unsigned long long id;
    char *name;
    char *lastname;
    char *group;
    unsigned char *scores;
} Student;

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_NO_SEARCH,
    STATUS_MEMORY_ERROR,
    STATUS_INPUT_ERROR,
    STATUS_OVERFLOW,
    STATUS_EMPTY_STRING,
    STATUS_INVALID_STRING,
    STATUS_INVALID_NAME,
    STATUS_INVALID_GROUP,
    STATUS_INVALID_SCORE,
    STATUS_SIGN_ERROR,
    STATUS_NO_DIGITS,
    STATUS_INVALID_ID,
    STATUS_INVALID_NUMBER,
    STATUS_INVALID_ARGUMENTS,
    STATUS_FLAG_A,
    STATUS_FLAG_B,
    STATUS_FLAG_C,
    STATUS_FLAG_D,
    STATUS_FLAG_E,
    STATUS_INVALID_FLAG,
    STATUS_FILE_ERROR
} Status;

static int is_blank(int c){
    return c == ' ' || c == '\t' || c == '\r';
}

static int is_latin(char c){
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

Status read_line(FILE *file, char **line, int *end){
    if (file == NULL || line == NULL || end == NULL)
        return STATUS_NULL_ARGUMENT;

    *line = NULL;
    *end = 0;

    size_t capacity = 64;
    size_t length = 0;
    char *buffer = malloc(capacity * sizeof(*buffer));
    if (buffer == NULL)
        return STATUS_MEMORY_ERROR;

    int c;
    while ((c = fgetc(file)) != EOF && c != '\n') {
        if (length + 1 >= capacity) {
            if (capacity > (size_t)-1 / 2 / sizeof(*buffer)) {
                free(buffer);
                return STATUS_MEMORY_ERROR;
            }
            char *tmp = realloc(buffer, capacity * 2 * sizeof(*buffer));
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

Status split_line(const char *line, const char *start[9], size_t length[9]){
    if (line == NULL || start == NULL || length == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t i = 0;
    int count = 0;

    while (line[i] != '\0') {
        while (is_blank((unsigned char)line[i]))
            i++;
        if (line[i] == '\0')
            break;
        if (count == 9)
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

    if (count != 9)
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

Status copy_name(const char *s, size_t n, char **out){
    if (s == NULL || out == NULL)
        return STATUS_NULL_ARGUMENT;
    if (n == 0)
        return STATUS_INVALID_NAME;
    for (size_t i = 0; i < n; i++)
        if (!is_latin(s[i]))
            return STATUS_INVALID_NAME;

    char *copy = malloc((n + 1) * sizeof(*copy));
    if (copy == NULL)
        return STATUS_MEMORY_ERROR;
    memcpy(copy, s, n);
    copy[n] = '\0';

    *out = copy;
    return STATUS_OK;
}

Status check_group(const char *s, size_t n, char **out){
    if (s == NULL || out == NULL)
        return STATUS_NULL_ARGUMENT;
    if (n == 0)
        return STATUS_INVALID_GROUP;

    char *copy = malloc((n + 1) * sizeof(*copy));
    if (copy == NULL)
        return STATUS_MEMORY_ERROR;
    memcpy(copy, s, n);
    copy[n] = '\0';

    *out = copy;
    return STATUS_OK;
}

void free_studend(Student *e){
    if (e == NULL)
        return;
    free(e->name);
    free(e->lastname);
    free(e->group);
    free(e->scores);
    e->name = NULL;
    e->lastname = NULL;
    e->group = NULL;
    e->scores = NULL;
}

void free_array(Student *array, size_t count){
    if (array == NULL)
        return;
    for (size_t i = 0; i < count; i++)
        free_studend(&array[i]);
    free(array);
}

/* разбирает одну строку файла в структуру*/
Status parse_student(const char *line, Student *stud){
    if (line == NULL || stud == NULL)
        return STATUS_NULL_ARGUMENT;

    const char *start[9];
    size_t length[9];

    Status st = split_line(line, start, length);
    if (st != STATUS_OK)
        return st;

    unsigned long long id;

    st = parse_id(start[0], length[0], &id);
    if (st != STATUS_OK)
        return st;

    char *name = NULL;
    char *lastname = NULL;
    char *group = NULL;

    st = copy_name(start[1], length[1], &name);
    if (st != STATUS_OK)
        return st;

    st = copy_name(start[2], length[2], &lastname);
    if (st != STATUS_OK) {
        free(name);
        return st;
    }

    st = check_group(start[3], length[3], &group);
    if (st != STATUS_OK) {
        free(lastname);
        free(name);
        return st;
    }

    unsigned char *score = malloc(5 * sizeof(*score));
    if (score == NULL){
        free(name);
        free(lastname);
        free(group);
        return STATUS_MEMORY_ERROR;
    }

    for (int i = 0; i < 5; i++){
        unsigned long long value = 0;
        st = parse_id(start[4 + i], length[4 + i], &value);
        if (st != STATUS_OK || value > 100){
            free(name);
            free(lastname);
            free(group);
            free(score);
            return STATUS_INVALID_SCORE;
        }
        score[i] = (unsigned char)value;
    }

    stud->id = id;
    stud->name = name;
    stud->lastname = lastname;
    stud->group = group;
    stud->scores = score;
    return STATUS_OK;
}

Status create_array(FILE *file, Student **result, size_t *count){
    if (file == NULL || result == NULL || count == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t capacity = 16;
    size_t size = 0;

    Student *array = malloc(capacity * sizeof(*array));
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
            Student *tmp = realloc(array, capacity * 2 * sizeof(*array));
            if (tmp == NULL) {
                free(line);
                free_array(array, size);
                return STATUS_MEMORY_ERROR;
            }
            array = tmp;
            capacity *= 2;
        }

        st = parse_student(line, &array[size]);
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

int compare_id(const void *a, const void *b){
    const Student *id_a = (const Student *)a;
    const Student *id_b = (const Student *)b;

    if (id_a->id < id_b->id)
        return -1;
    if (id_a->id > id_b->id)
        return 1;
    return 0;
}

int compare_name(const void *a, const void *b){
    const Student *name_a = (const Student *)a;
    const Student *name_b = (const Student *)b;

    return strcmp(name_a->name, name_b->name);
}

int compare_lastname(const void *a, const void *b){
    const Student *lastname_a = (const Student *)a;
    const Student *lastname_b = (const Student *)b;

    return strcmp(lastname_a->lastname, lastname_b->lastname);
}

int compare_group(const void *a, const void *b){
    const Student *group_a = (const Student *)a;
    const Student *group_b = (const Student *)b;

    return strcmp(group_a->group, group_b->group);
}

Status search_id(const Student *students, size_t count_students, unsigned long long st_id, Student **res, size_t *count_res){
    if (students == NULL || res == NULL || count_res == NULL)
        return STATUS_NULL_ARGUMENT;

    Student *result = malloc(count_students * sizeof(*result));
    if (result == NULL)
        return STATUS_MEMORY_ERROR;

    size_t c = 0;
    for (size_t i = 0; i < count_students; i++)
        if (students[i].id == st_id)
            result[c++] = students[i];

    if (c == 0){
        free(result);
        return STATUS_NO_SEARCH;
    }
    *res = result;
    *count_res = c;
    return STATUS_OK;
}

Status search_lastname(const Student *students, size_t count_students, const char *st_lastname, Student **res, size_t *count_res){
    if (students == NULL || st_lastname == NULL || res == NULL || count_res == NULL)
        return STATUS_NULL_ARGUMENT;

    Student *result = malloc(count_students * sizeof(*result));
    if (result == NULL)
        return STATUS_MEMORY_ERROR;

    size_t c = 0;
    for (size_t i = 0; i < count_students; i++)
        if (strcmp(students[i].lastname, st_lastname) == 0)
            result[c++] = students[i];

    if (c == 0){
        free(result);
        return STATUS_NO_SEARCH;
    }
    *res = result;
    *count_res = c;
    return STATUS_OK;
}

Status search_name(const Student *students, size_t count_students, const char *st_name, Student **res, size_t *count_res){
    if (students == NULL || st_name == NULL || res == NULL || count_res == NULL)
        return STATUS_NULL_ARGUMENT;

    Student *result = malloc(count_students * sizeof(*result));
    if (result == NULL)
        return STATUS_MEMORY_ERROR;

    size_t c = 0;
    for (size_t i = 0; i < count_students; i++)
        if (strcmp(students[i].name, st_name) == 0)
            result[c++] = students[i];

    if (c == 0){
        free(result);
        return STATUS_NO_SEARCH;
    }
    *res = result;
    *count_res = c;
    return STATUS_OK;
}

Status search_group(const Student *students, size_t count_students, const char *st_group, Student **res, size_t *count_res){
    if (students == NULL || st_group == NULL || res == NULL || count_res == NULL)
        return STATUS_NULL_ARGUMENT;

    Student *result = malloc(count_students * sizeof(*result));
    if (result == NULL)
        return STATUS_MEMORY_ERROR;

    size_t c = 0;
    for (size_t i = 0; i < count_students; i++)
        if (strcmp(students[i].group, st_group) == 0)
            result[c++] = students[i];

    if (c == 0){
        free(result);
        return STATUS_NO_SEARCH;
    }
    *res = result;
    *count_res = c;
    return STATUS_OK;
}

static double student_average(const Student *s){
    unsigned int sum = 0;
    for (size_t j = 0; j < 5; j++)
        sum += s->scores[j];
    return (double)sum / 5.0;
}

Status search_score(const Student *students, size_t count_st, double ave_score, double eps, Student **res, size_t *count_res){
    if (students == NULL || res == NULL || count_res == NULL)
        return STATUS_NULL_ARGUMENT;

    Student *result = malloc(count_st * sizeof(*result));
    if (result == NULL)
        return STATUS_MEMORY_ERROR;

    size_t c_st = 0;
    for (size_t i = 0; i < count_st; i++){
        if (student_average(&students[i]) - ave_score > eps)
            result[c_st++] = students[i];
    }

    if (c_st == 0){
        free(result);
        return STATUS_NO_SEARCH;
    }
    *res = result;
    *count_res = c_st;
    return STATUS_OK;
}

/* flag 0 - вся запись, 1 - имя, фамилия, группа, средний балл, 2 - фамилия и имя */
Status write_array(FILE *output, const Student *array, size_t count, int flag){
    if (output == NULL || array == NULL)
        return STATUS_NULL_ARGUMENT;

    for (size_t i = 0; i < count; i++) {
        if (flag == 0){
            if (fprintf(output, "%llu %s %s %s", array[i].id, array[i].name, array[i].lastname, array[i].group) < 0)
                return STATUS_FILE_ERROR;
            for (size_t j = 0; j < 5; j++)
                if (fprintf(output, " %u", (unsigned int)array[i].scores[j]) < 0)
                    return STATUS_FILE_ERROR;
            if (fprintf(output, "\n") < 0)
                return STATUS_FILE_ERROR;
        } else if (flag == 1){
            if (fprintf(output, "%s %s %s %.3f\n", array[i].lastname, array[i].name, array[i].group, student_average(&array[i])) < 0)
                return STATUS_FILE_ERROR;
        } else {
            if (fprintf(output, "%s %s\n", array[i].lastname, array[i].name) < 0)
                return STATUS_FILE_ERROR;
        }
    }
    return STATUS_OK;
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

void print_error(Status status){
    switch (status) {
        case STATUS_NULL_ARGUMENT:     printf("Внутренняя ошибка: нулевой указатель.\n"); break;
        case STATUS_MEMORY_ERROR:      printf("Ошибка: не удалось выделить память.\n"); break;
        case STATUS_FILE_ERROR:        printf("Ошибка работы с файлом.\n"); break;
        case STATUS_EMPTY_STRING:      printf("Ошибка: передана пустая строка.\n"); break;
        case STATUS_INVALID_ARGUMENTS: printf("Ошибка: запрещённое расширение файла.\n"); break;
        case STATUS_INVALID_STRING:    printf("Ошибка: в строке файла должно быть ровно 9 полей (id, имя, фамилия, группа, 5 оценок).\n"); break;
        case STATUS_INVALID_ID:        printf("Ошибка: некорректный id (неотрицательное целое число).\n"); break;
        case STATUS_INVALID_NAME:      printf("Ошибка: имя и фамилия должны состоять только из латинских букв.\n"); break;
        case STATUS_INVALID_GROUP:     printf("Ошибка: группа не должна быть пустой.\n"); break;
        case STATUS_INVALID_SCORE:     printf("Ошибка: оценка должна быть целым числом от 0 до 100.\n"); break;
        case STATUS_INPUT_ERROR:       printf("Ошибка ввода.\n"); break;
        case STATUS_OVERFLOW:          printf("Ошибка: переполнение.\n"); break;
        default:                       printf("Ошибка.\n"); break;
    }
}

Status read_choice(char *choice){
    if (choice == NULL)
        return STATUS_NULL_ARGUMENT;

    char *w = NULL;
    Status st = read_word(stdin, &w, 0);
    if (st != STATUS_OK)
        return st;

    *choice = (w[0] != '\0' && w[1] == '\0') ? w[0] : '?';
    free(w);
    return STATUS_OK;
}

Status menu_sort(Student *students, size_t count){
    printf("Введите цифру соответствующую одному из действий: \n1. Отсортировать список студентов по id.\n2. Отсортировать список студентов по имени.\n3. Отсортировать список студентов по фамилии.\n4. Отсортировать список студентов по группе.\n");

    char choice;
    Status st = read_choice(&choice);
    if (st != STATUS_OK)
        return st;

    int (*cmp)(const void *, const void *) = NULL;
    if (choice == '1')
        cmp = compare_id;
    else if (choice == '2')
        cmp = compare_name;
    else if (choice == '3')
        cmp = compare_lastname;
    else if (choice == '4')
        cmp = compare_group;
    else {
        printf("Неизвестное действие.\n");
        return STATUS_OK;
    }

    qsort(students, count, sizeof(*students), cmp);
    return write_array(stdout, students, count, 0);
}

Status menu_search(const Student *students, size_t count, FILE *trace){
    printf("Введите цифру соответствующую одному из действий: \n1. Поиск студента по id.\n2. Поиск студентов по имени.\n3. Поиск студентов по фамилии.\n4. Поиск студентов по группе.\n5. Поиск студентов, у которых средний балл за все экзамены выше среднего балла за все экзамены у всех студентов из списка.\n");

    char choice;
    Status st = read_choice(&choice);
    if (st != STATUS_OK)
        return st;

    Student *res = NULL;
    size_t c_res = 0;

    if (choice == '1'){
        printf("Введите id: \n");
        char *w = NULL;
        st = read_word(stdin, &w, 0);
        if (st != STATUS_OK)
            return st;

        unsigned long long id = 0;
        st = parse_id(w, strlen(w), &id);
        free(w);
        if (st != STATUS_OK)
            return st;

        st = search_id(students, count, id, &res, &c_res);
        if (st == STATUS_NO_SEARCH){
            printf("Студент не найден.\n");
            return STATUS_OK;
        }
        if (st != STATUS_OK)
            return st;

        st = write_array(stdout, res, c_res, 0);
        if (st == STATUS_OK)
            st = write_array(trace, res, c_res, 1);       
        free(res);
        return st;

    } else if (choice == '2' || choice == '3' || choice == '4'){
        Status (*fn)(const Student *, size_t, const char *, Student **, size_t *) = search_name;
        if (choice == '3')
            fn = search_lastname;
        else if (choice == '4')
            fn = search_group;

        printf("Введите значение для поиска: \n");
        char *w = NULL;
        st = read_word(stdin, &w, 0);
        if (st != STATUS_OK)
            return st;

        st = fn(students, count, w, &res, &c_res);
        free(w);
        if (st == STATUS_NO_SEARCH){
            printf("Студенты не найдены.\n");
            return STATUS_OK;
        }
        if (st != STATUS_OK)
            return st;

        st = write_array(stdout, res, c_res, 0);
        free(res);
        return st;

    } else if (choice == '5'){
        /* средний балл за все экзамены по всем студентам */
        unsigned long long total = 0;
        for (size_t i = 0; i < count; i++)
            for (size_t j = 0; j < 5; j++)
                total += students[i].scores[j];
        double ave_score = (double)total / (5.0 * (double)count);

        st = search_score(students, count, ave_score, EPS, &res, &c_res);
        if (st == STATUS_NO_SEARCH){
            printf("Студентов с баллом выше среднего нет.\n");
            return STATUS_OK;
        }
        if (st != STATUS_OK)
            return st;

        st = write_array(trace, res, c_res, 2);
        free(res);
        if (st == STATUS_OK)
            printf("Записано в трассировочный файл: %zu\n", c_res);
        return st;
    }

    printf("Неизвестное действие.\n");
    return STATUS_OK;
}

int main(int argc, char *argv[]){
    if (argc != 3){
        printf("Введите путь к файлу со студентами и путь к трассировочному файлу.\n");
        return 1;
    }

    Status st = check_file_extension(argv[1]);
    if (st == STATUS_OK)
        st = check_file_extension(argv[2]);
    if (st != STATUS_OK){
        print_error(st);
        return 1;
    }

    if (same_files(argv[1], argv[2])){
        printf("Ошибка: входной и трассировочный файлы совпадают.\n");
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (input == NULL){
        print_error(STATUS_FILE_ERROR);
        return 1;
    }

    Student *students = NULL;
    size_t count = 0;

    st = create_array(input, &students, &count);
    int input_close = fclose(input);

    if (st != STATUS_OK){
        print_error(st);
        return 1;
    }
    if (input_close != 0){
        print_error(STATUS_FILE_ERROR);
        free_array(students, count);
        return 1;
    }
    if (count == 0){
        printf("Ошибка: в файле нет студентов.\n");
        free_array(students, count);
        return 1;
    }

    FILE *trace = fopen(argv[2], "w");
    if (trace == NULL){
        print_error(STATUS_FILE_ERROR);
        free_array(students, count);
        return 1;
    }

    int result = 0;

    while (1){
        printf("Введите цифру соответствующую одному из действий: \n1. Отсортировать список студентов.\n2. Поиск студентов в списке.\n0. Выход.\n");

        char choice;
        st = read_choice(&choice);
        if (st != STATUS_OK)                 
            break;

        if (choice == '0')
            break;

        if (choice == '1')
            st = menu_sort(students, count);
        else if (choice == '2')
            st = menu_search(students, count, trace);
        else {
            printf("Неизвестное действие.\n");
            continue;
        }

        if (st != STATUS_OK){
            print_error(st);
            if (st == STATUS_MEMORY_ERROR || st == STATUS_FILE_ERROR){
                result = 1;
                break;
            }
            if (st == STATUS_INPUT_ERROR)
                break;
        }
    }

    if (fclose(trace) != 0){
        print_error(STATUS_FILE_ERROR);
        result = 1;
    }

    free_array(students, count);
    return result;
}