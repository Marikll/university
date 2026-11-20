#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <sys/stat.h>
#include <limits.h>

#define EPS 1e-9

typedef struct Liver {
    char *name;
    char *lastname;
    char *patronymic;
    unsigned int day;
    unsigned int month;
    unsigned int year;
    char gender;
    double ave_income;
    struct Liver *next_liver;
} Liver;

typedef struct{
    Liver *first_liver;
    size_t len_liver;
} List;

typedef enum{ OP_ADD, OP_DELETE, OP_MODIFY } OpType;

/* одна запись истории: для отмены хранятся копии данных */
typedef struct{
    OpType type;
    Liver old_data;    
    Liver new_data;  
} Record;

typedef struct{
    Record *items;
    size_t count;
    size_t capacity;
} History;

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
    STATUS_INVALID_BIRTHDAY,
    STATUS_INVALID_GENDER,
    STATUS_SIGN_ERROR,
    STATUS_NO_DIGITS,
    STATUS_INVALID_NUMBER,
    STATUS_INVALID_ARGUMENTS,
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

Status read_trimmed(char **out){
    if (out == NULL)
        return STATUS_NULL_ARGUMENT;
    *out = NULL;

    while (1){
        char *line = NULL;
        int end = 0;

        Status st = read_line(stdin, &line, &end);
        if (st != STATUS_OK)
            return st;
        if (end)
            return STATUS_INPUT_ERROR;

        size_t b = 0;
        while (is_blank((unsigned char)line[b]))
            b++;
        size_t e = strlen(line);
        while (e > b && is_blank((unsigned char)line[e - 1]))
            e--;

        if (e == b){
            free(line);
            continue;
        }
        memmove(line, line + b, e - b);
        line[e - b] = '\0';
        *out = line;
        return STATUS_OK;
    }
}

Status split_line(const char *line, const char *start[6], size_t length[6], int *count){
    if (line == NULL || start == NULL || length == NULL || count == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t i = 0;
    int c = 0;

    while (line[i] != '\0') {
        while (is_blank((unsigned char)line[i]))
            i++;
        if (line[i] == '\0')
            break;
        if (c == 6)
            return STATUS_INVALID_STRING;

        start[c] = &line[i];
        size_t n = 0;
        while (line[i] != '\0' && !is_blank((unsigned char)line[i])) {
            i++;
            n++;
        }
        length[c] = n;
        c++;
    }

    if (c != 5 && c != 6)
        return STATUS_INVALID_STRING;
    *count = c;
    return STATUS_OK;
}

Status copy_name(const char *s, size_t n, char **out, int flag){
    if (s == NULL || out == NULL)
        return STATUS_NULL_ARGUMENT;
    if (n == 0 && flag == 0)
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

Status dup_string(const char *s, char **out){
    if (s == NULL || out == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t n = strlen(s);
    char *copy = malloc((n + 1) * sizeof(*copy));
    if (copy == NULL)
        return STATUS_MEMORY_ERROR;
    memcpy(copy, s, n + 1);
    *out = copy;
    return STATUS_OK;
}

Status conversion_age(const char *s, size_t n, unsigned int *day, unsigned int *month, unsigned int *year){
    if (s == NULL || day == NULL || month == NULL || year == NULL)
        return STATUS_NULL_ARGUMENT;

    if (n == 0)
        return STATUS_EMPTY_STRING;

    size_t i = 0;
    if (s[i] == '-')
        return STATUS_SIGN_ERROR;

    if (s[i] < '0' || s[i] > '9')
        return STATUS_INVALID_NUMBER;

    int point = 0;
    unsigned int d = 0;
    unsigned int m = 0;
    unsigned int y = 0;
    unsigned int num = 0;

    for (; i < n; i++){
        if (s[i] == '.'){
            point++;
            if (num != 0 && point == 1)
                d = num;
            else if (num != 0 && point == 2)
                m = num;
            else
                return STATUS_INVALID_BIRTHDAY;
            num = 0;
            continue;                   
        }

        if (s[i] < '0' || s[i] > '9')
            return STATUS_INVALID_NUMBER;

        unsigned int digit = (unsigned int)(s[i] - '0');

        if (num > (UINT_MAX - digit) / 10) 
            return STATUS_OVERFLOW;
        num = num * 10 + digit;
    }

    if (point != 2 || num == 0)        
        return STATUS_INVALID_BIRTHDAY;
    y = num;

    int leap = (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));

    if ((d > 30 && m == 4) || (d > 30 && m == 6) || (d > 30 && m == 9) || (d > 30 && m == 11) ||
        (d > 29 && m == 2 && leap) || (d > 28 && m == 2 && !leap))
        return STATUS_INVALID_BIRTHDAY;
    if (d > 31 || m > 12)
        return STATUS_INVALID_BIRTHDAY;

    *day = d;
    *month = m;
    *year = y;
    return STATUS_OK;
}

Status conversion_income(const char *str, size_t n, double *num){
    if (str == NULL || num == NULL) 
        return STATUS_NULL_ARGUMENT; 

    if (str[0] == '\0') 
        return STATUS_EMPTY_STRING; 
    
    int i = 0;
    
    if (str[i] == '-')
        return STATUS_SIGN_ERROR;

    if (str[i] < '0' || str[i] > '9')
        return STATUS_INVALID_NUMBER;
    
    double value = 0.0; 
    double fraction = 0.0; 
    double divider = 10.0; 
    int point = 0; 
    int digits = 0; 
    
    for (; i < n; i++){ 
        if (str[i] == '.' || str[i] == ','){ 
            if (point != 0) 
                return STATUS_INVALID_NUMBER; 
                
            point = 1;
            continue; 
        } 
        if (str[i] < '0' || str[i] > '9') 
            return STATUS_INVALID_NUMBER; 
        
        int digit = str[i] - '0'; 
        digits++; 

        if (point == 0){ 
            if (value > (DBL_MAX - digit) / 10.0) 
                return STATUS_OVERFLOW; 
            
            value = value * 10.0 + digit; 
        } else{ 
            fraction += digit / divider; 
            divider *= 10.0; 
        } 
    } 
    if (digits == 0) 
        return STATUS_NO_DIGITS; 

    value += fraction; 

    *num = value; 
    return STATUS_OK; 
}
Status parse_gender(const char *s, size_t n, char *gender){
    if (s == NULL || gender == NULL)
        return STATUS_NULL_ARGUMENT;
    if (n != 1 || (s[0] != 'M' && s[0] != 'W'))
        return STATUS_INVALID_GENDER;
    *gender = s[0];
    return STATUS_OK;
}

Status parse_student(const char *line, Liver *liver){
    if (line == NULL || liver == NULL)
        return STATUS_NULL_ARGUMENT;

    const char *start[6];
    size_t length[6];
    int count = 0;

    Status st = split_line(line, start, length, &count);
    if (st != STATUS_OK)
        return st;

    int shift = (count == 6) ? 1 : 0;       

    unsigned int day = 0, month = 0, year = 0;
    st = conversion_age(start[2 + shift], length[2 + shift], &day, &month, &year);
    if (st != STATUS_OK)
        return st;

    char gender = 0;
    st = parse_gender(start[3 + shift], length[3 + shift], &gender);
    if (st != STATUS_OK)
        return st;

    double ave_income = 0.0;
    st = conversion_income(start[4 + shift], length[4 + shift], &ave_income);
    if (st != STATUS_OK)
        return st;

    char *name = NULL;
    char *lastname = NULL;
    char *patronymic = NULL;

    st = copy_name(start[0], length[0], &lastname, 0);
    if (st != STATUS_OK)
        return st;

    st = copy_name(start[1], length[1], &name, 0);
    if (st != STATUS_OK) {
        free(lastname);
        return st;
    }

    if (count == 6)
        st = copy_name(start[2], length[2], &patronymic, 1);
    else
        st = copy_name("", 0, &patronymic, 1);
    if (st != STATUS_OK) {
        free(lastname);
        free(name);
        return st;
    }

    liver->name = name;
    liver->lastname = lastname;
    liver->patronymic = patronymic;
    liver->day = day;
    liver->month = month;
    liver->year = year;
    liver->gender = gender;
    liver->ave_income = ave_income;
    liver->next_liver = NULL;
    return STATUS_OK;
}

void create_list(List *l){
    if (l == NULL)
        return;
    l->first_liver = NULL;
    l->len_liver = 0;
}

void free_liver_data(Liver *l){
    if (l == NULL)
        return;
    free(l->name);
    free(l->lastname);
    free(l->patronymic);
    l->name = NULL;
    l->lastname = NULL;
    l->patronymic = NULL;
    l->next_liver = NULL;
}

void free_liver(Liver *l){
    if (l == NULL)
        return;
    free_liver_data(l);
    free(l);
}

void free_list(List *l){
    if (l == NULL)
        return;
    Liver *liver = l->first_liver;
    while (liver != NULL){
        Liver *next_l = liver->next_liver;
        free_liver(liver);
        liver = next_l;
    }
    l->first_liver = NULL;
    l->len_liver = 0;
}

/* глубокая копия */
Status copy_liver(const Liver *src, Liver *dst){
    if (src == NULL || dst == NULL)
        return STATUS_NULL_ARGUMENT;

    Liver tmp = *src;
    tmp.name = NULL;
    tmp.lastname = NULL;
    tmp.patronymic = NULL;
    tmp.next_liver = NULL;

    Status st = dup_string(src->lastname, &tmp.lastname);
    if (st == STATUS_OK)
        st = dup_string(src->name, &tmp.name);
    if (st == STATUS_OK)
        st = dup_string(src->patronymic, &tmp.patronymic);
    if (st != STATUS_OK){
        free_liver_data(&tmp);
        return st;
    }

    *dst = tmp;
    return STATUS_OK;
}

int liver_equal(const Liver *a, const Liver *b, double eps){
    return strcmp(a->lastname, b->lastname) == 0 &&
           strcmp(a->name, b->name) == 0 &&
           strcmp(a->patronymic, b->patronymic) == 0 &&
           a->day == b->day && a->month == b->month && a->year == b->year &&
           a->gender == b->gender &&
           fabs(a->ave_income - b->ave_income) <= eps;
}

unsigned long date_key(const Liver *l){
    return (unsigned long)l->year * 10000UL + (unsigned long)l->month * 100UL + (unsigned long)l->day;
}

/* вставка узла с сохранением порядка */
void list_insert(List *l, Liver *node){
    Liver *prev = NULL;
    Liver *cur = l->first_liver;
    while (cur != NULL && date_key(cur) >= date_key(node)){
        prev = cur;
        cur = cur->next_liver;
    }

    node->next_liver = cur;
    if (prev == NULL)
        l->first_liver = node;
    else
        prev->next_liver = node;
    l->len_liver++;
}

/* вынимает узел из списка, не освобождая */
Status list_unlink(List *l, Liver *target){
    if (l == NULL || target == NULL)
        return STATUS_NULL_ARGUMENT;

    Liver *prev = NULL;
    Liver *cur = l->first_liver;
    while (cur != NULL && cur != target){
        prev = cur;
        cur = cur->next_liver;
    }
    if (cur == NULL)
        return STATUS_NO_SEARCH;

    if (prev == NULL)
        l->first_liver = cur->next_liver;
    else
        prev->next_liver = cur->next_liver;
    cur->next_liver = NULL;
    l->len_liver--;
    return STATUS_OK;
}

Liver *find_equal(const List *l, const Liver *x, double eps){
    for (Liver *cur = l->first_liver; cur != NULL; cur = cur->next_liver)
        if (liver_equal(cur, x, eps))
            return cur;
    return NULL;
}

Status create_array(FILE *file, List *list){
    if (file == NULL || list == NULL)
        return STATUS_NULL_ARGUMENT;

    while (1) {
        char *line = NULL;
        int end = 0;

        Status st = read_line(file, &line, &end);
        if (st != STATUS_OK) {
            free_list(list);
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

        Liver *liver = malloc(sizeof(*liver));
        if (liver == NULL){
            free(line);
            free_list(list);
            return STATUS_MEMORY_ERROR;
        }

        st = parse_student(line, liver);
        free(line);

        if (st != STATUS_OK) {
            free(liver);
            free_list(list);
            return st;
        }
        list_insert(list, liver);
    }
    return STATUS_OK;
}

/* field: 1 фамилия, 2 имя, 3 отчество, 4 дата, 5 пол, 6 доход */
int liver_matches(const Liver *l, int field, const Liver *key, double eps){
    switch (field){
        case 1: return strcmp(l->lastname, key->lastname) == 0;
        case 2: return strcmp(l->name, key->name) == 0;
        case 3: return strcmp(l->patronymic, key->patronymic) == 0;
        case 4: return l->day == key->day && l->month == key->month && l->year == key->year;
        case 5: return l->gender == key->gender;
        case 6: return fabs(l->ave_income - key->ave_income) <= eps;
        default: return 0;
    }
}

Status search_livers(const List *l, int field, const Liver *key, double eps, Liver ***res, size_t *count){
    if (l == NULL || key == NULL || res == NULL || count == NULL)
        return STATUS_NULL_ARGUMENT;
    if (l->len_liver == 0)
        return STATUS_NO_SEARCH;

    Liver **result = malloc(l->len_liver * sizeof(*result));
    if (result == NULL)
        return STATUS_MEMORY_ERROR;

    size_t c = 0;
    for (Liver *cur = l->first_liver; cur != NULL; cur = cur->next_liver)
        if (liver_matches(cur, field, key, eps))
            result[c++] = cur;

    if (c == 0){
        free(result);
        return STATUS_NO_SEARCH;
    }
    *res = result;
    *count = c;
    return STATUS_OK;
}

/* заполняет поле ключа поиска из введённого слова */
Status build_key(int field, const char *word, Liver *key){
    if (word == NULL || key == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t n = strlen(word);

    switch (field){
        case 1: return copy_name(word, n, &key->lastname, 0);
        case 2: return copy_name(word, n, &key->name, 0);
        case 3:
            if (strcmp(word, "-") == 0)
                return copy_name("", 0, &key->patronymic, 1);
            return copy_name(word, n, &key->patronymic, 0);
        case 4: return conversion_age(word, n, &key->day, &key->month, &key->year);
        case 5: return parse_gender(word, n, &key->gender);
        case 6: return conversion_income(word, n, &key->ave_income);
        default: return STATUS_INVALID_FLAG;
    }
}

Status write_liver(FILE *out, const Liver *l){
    if (out == NULL || l == NULL)
        return STATUS_NULL_ARGUMENT;

    if (fprintf(out, "%s %s ", l->lastname, l->name) < 0)
        return STATUS_FILE_ERROR;
    if (l->patronymic[0] != '\0' && fprintf(out, "%s ", l->patronymic) < 0)
        return STATUS_FILE_ERROR;
    if (fprintf(out, "%02u.%02u.%04u %c %.2f\n", l->day, l->month, l->year, l->gender, l->ave_income) < 0)
        return STATUS_FILE_ERROR;
    return STATUS_OK;
}

Status write_list(FILE *out, const List *l){
    if (out == NULL || l == NULL)
        return STATUS_NULL_ARGUMENT;

    for (const Liver *cur = l->first_liver; cur != NULL; cur = cur->next_liver){
        Status st = write_liver(out, cur);
        if (st != STATUS_OK)
            return st;
    }
    return STATUS_OK;
}

void history_init(History *h){
    h->items = NULL;
    h->count = 0;
    h->capacity = 0;
}

void free_record(Record *r){
    free_liver_data(&r->old_data);
    free_liver_data(&r->new_data);
}

void history_clear(History *h){
    for (size_t i = 0; i < h->count; i++)
        free_record(&h->items[i]);
    h->count = 0;
}

void history_free(History *h){
    history_clear(h);
    free(h->items);
    h->items = NULL;
    h->capacity = 0;
}

Status history_reserve(History *h){
    if (h == NULL)
        return STATUS_NULL_ARGUMENT;
    if (h->count < h->capacity)
        return STATUS_OK;

    if (h->capacity > (size_t)-1 / 2)
        return STATUS_MEMORY_ERROR;
    size_t new_cap = (h->capacity == 0) ? 8 : h->capacity * 2;
    if (new_cap > (size_t)-1 / sizeof(*h->items))
        return STATUS_MEMORY_ERROR;

    Record *tmp = realloc(h->items, new_cap * sizeof(*tmp));
    if (tmp == NULL)
        return STATUS_MEMORY_ERROR;
    h->items = tmp;
    h->capacity = new_cap;
    return STATUS_OK;
}

Status op_add(List *l, History *h, Liver *node){
    if (l == NULL || h == NULL || node == NULL)
        return STATUS_NULL_ARGUMENT;

    Status st = history_reserve(h);
    if (st != STATUS_OK)
        return st;

    Record rec = {0};
    rec.type = OP_ADD;
    st = copy_liver(node, &rec.new_data);
    if (st != STATUS_OK)
        return st;

    list_insert(l, node);
    h->items[h->count++] = rec;
    return STATUS_OK;
}

Status op_delete(List *l, History *h, Liver *target){
    if (l == NULL || h == NULL || target == NULL)
        return STATUS_NULL_ARGUMENT;

    Status st = history_reserve(h);
    if (st != STATUS_OK)
        return st;

    st = list_unlink(l, target);
    if (st != STATUS_OK)
        return st;

    Record rec = {0};
    rec.type = OP_DELETE;
    rec.old_data = *target;
    rec.old_data.next_liver = NULL;
    free(target);

    h->items[h->count++] = rec;
    return STATUS_OK;
}

Status op_modify(List *l, History *h, Liver *target, Liver *new_node){
    if (l == NULL || h == NULL || target == NULL || new_node == NULL)
        return STATUS_NULL_ARGUMENT;

    Status st = history_reserve(h);
    if (st != STATUS_OK)
        return st;

    Record rec = {0};
    rec.type = OP_MODIFY;
    st = copy_liver(new_node, &rec.new_data);
    if (st != STATUS_OK)
        return st;

    st = list_unlink(l, target);
    if (st != STATUS_OK){
        free_record(&rec);
        return st;
    }

    rec.old_data = *target;
    rec.old_data.next_liver = NULL;
    free(target);

    list_insert(l, new_node);
    h->items[h->count++] = rec;
    return STATUS_OK;
}

Status undo(List *l, History *h, size_t *undone){
    if (l == NULL || h == NULL || undone == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t k = h->count / 2;
    *undone = 0;
    if (k == 0)
        return STATUS_OK;

    const Liver empty = {0};

    for (size_t i = 0; i < k; i++){
        Record *r = &h->items[h->count - 1];

        if (r->type == OP_ADD){
            Liver *cur = find_equal(l, &r->new_data, EPS);
            if (cur == NULL)
                return STATUS_NO_SEARCH;
            list_unlink(l, cur);
            free_liver(cur);

        } else if (r->type == OP_DELETE){
            Liver *node = malloc(sizeof(*node));
            if (node == NULL)
                return STATUS_MEMORY_ERROR;
            *node = r->old_data;
            r->old_data = empty;
            list_insert(l, node);

        } else {
            Liver *cur = find_equal(l, &r->new_data, EPS);
            if (cur == NULL)
                return STATUS_NO_SEARCH;
            Liver *node = malloc(sizeof(*node));
            if (node == NULL)
                return STATUS_MEMORY_ERROR;
            list_unlink(l, cur);
            free_liver(cur);
            *node = r->old_data;
            r->old_data = empty;
            list_insert(l, node);
        }

        free_record(r);
        h->count--;
        (*undone)++;
    }

    history_clear(h);
    return STATUS_OK;
}

int same_files(const char *path1, const char *path2){
    struct stat s1, s2;

    if (path1 == NULL || path2 == NULL)
        return 0;

    if (stat(path1, &s1) != 0 || stat(path2, &s2) != 0)
        return 0;

    return s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino;
}

void print_error(Status status){
    switch (status) {
        case STATUS_NULL_ARGUMENT:     printf("Внутренняя ошибка: нулевой указатель.\n"); break;
        case STATUS_NO_SEARCH:         printf("Ничего не найдено.\n"); break;
        case STATUS_MEMORY_ERROR:      printf("Ошибка: не удалось выделить память.\n"); break;
        case STATUS_INPUT_ERROR:       printf("Ошибка ввода.\n"); break;
        case STATUS_OVERFLOW:          printf("Ошибка: переполнение.\n"); break;
        case STATUS_EMPTY_STRING:      printf("Ошибка: передана пустая строка.\n"); break;
        case STATUS_INVALID_STRING:    printf("Ошибка: нужно 5 или 6 слов: фамилия имя [отчество] дд.мм.гггг пол доход.\n"); break;
        case STATUS_INVALID_NAME:      printf("Ошибка: фамилия, имя и отчество должны состоять из латинских букв (фамилия и имя не пустые).\n"); break;
        case STATUS_INVALID_BIRTHDAY:  printf("Ошибка: некорректная дата рождения (формат дд.мм.гггг).\n"); break;
        case STATUS_INVALID_GENDER:    printf("Ошибка: пол должен быть M или W.\n"); break;
        case STATUS_SIGN_ERROR:        printf("Ошибка: число должно быть неотрицательным.\n"); break;
        case STATUS_NO_DIGITS:
        case STATUS_INVALID_NUMBER:    printf("Ошибка: некорректное число.\n"); break;
        case STATUS_INVALID_ARGUMENTS: printf("Ошибка: этот файл совпадает с исходным.\n"); break;
        case STATUS_INVALID_FLAG:      printf("Ошибка: неизвестный пункт.\n"); break;
        case STATUS_FILE_ERROR:        printf("Ошибка работы с файлом.\n"); break;
        default:                       printf("Ошибка.\n"); break;
    }
}

Status parse_size(const char *s, size_t *out){
    if (s == NULL || out == NULL)
        return STATUS_NULL_ARGUMENT;
    if (s[0] == '\0')
        return STATUS_EMPTY_STRING;

    size_t value = 0;
    for (size_t i = 0; s[i] != '\0'; i++){
        if (s[i] < '0' || s[i] > '9')
            return STATUS_INVALID_NUMBER;
        size_t d = (size_t)(s[i] - '0');
        if (value > ((size_t)-1 - d) / 10)
            return STATUS_OVERFLOW;
        value = value * 10 + d;
    }
    *out = value;
    return STATUS_OK;
}

Status read_choice(char *choice){
    if (choice == NULL)
        return STATUS_NULL_ARGUMENT;

    char *w = NULL;
    Status st = read_trimmed(&w);
    if (st != STATUS_OK)
        return st;

    *choice = (w[1] == '\0') ? w[0] : '?';
    free(w);
    return STATUS_OK;
}

Status search_dialog(const List *l, Liver ***res, size_t *count){
    printf("Поиск по: 1 - фамилия, 2 - имя, 3 - отчество (- значит пустое), 4 - дата рождения (дд.мм.гггг), 5 - пол (M/W), 6 - средний доход:\n");

    char choice;
    Status st = read_choice(&choice);
    if (st != STATUS_OK)
        return st;
    if (choice < '1' || choice > '6')
        return STATUS_INVALID_FLAG;
    int field = choice - '0';

    printf("Введите значение:\n");
    char *w = NULL;
    st = read_trimmed(&w);
    if (st != STATUS_OK)
        return st;

    Liver key = {0};
    st = build_key(field, w, &key);
    free(w);
    if (st != STATUS_OK){
        free_liver_data(&key);
        return st;
    }

    st = search_livers(l, field, &key, EPS, res, count);
    free_liver_data(&key);
    return st;
}

/* поиск и выбор одного жителя из найденных */
Status pick_liver(const List *l, Liver **chosen){
    if (l == NULL || chosen == NULL)
        return STATUS_NULL_ARGUMENT;

    Liver **res = NULL;
    size_t count = 0;
    Status st = search_dialog(l, &res, &count);
    if (st != STATUS_OK)
        return st;

    for (size_t i = 0; i < count; i++){
        printf("%zu. ", i + 1);
        write_liver(stdout, res[i]);
    }

    size_t index = 0;
    if (count > 1){
        printf("Введите номер жителя:\n");
        char *w = NULL;
        st = read_trimmed(&w);
        if (st == STATUS_OK){
            size_t number = 0;
            st = parse_size(w, &number);
            free(w);
            if (st == STATUS_OK && (number < 1 || number > count))
                st = STATUS_INVALID_NUMBER;
            if (st == STATUS_OK)
                index = number - 1;
        }
        if (st != STATUS_OK){
            free(res);
            return st;
        }
    }

    *chosen = res[index];
    free(res);
    return STATUS_OK;
}

Status read_new_liver(Liver **node){
    printf("Введите жителя: фамилия имя [отчество] дд.мм.гггг пол(M/W) доход\n");

    char *line = NULL;
    Status st = read_trimmed(&line);
    if (st != STATUS_OK)
        return st;

    Liver *liver = malloc(sizeof(*liver));
    if (liver == NULL){
        free(line);
        return STATUS_MEMORY_ERROR;
    }

    st = parse_student(line, liver);
    free(line);
    if (st != STATUS_OK){
        free(liver);
        return st;
    }
    *node = liver;
    return STATUS_OK;
}

Status menu_search(const List *l){
    Liver **res = NULL;
    size_t count = 0;
    Status st = search_dialog(l, &res, &count);
    if (st != STATUS_OK)
        return st;

    for (size_t i = 0; i < count && st == STATUS_OK; i++)
        st = write_liver(stdout, res[i]);
    free(res);
    return st;
}

Status menu_add(List *l, History *h){
    Liver *node = NULL;
    Status st = read_new_liver(&node);
    if (st != STATUS_OK)
        return st;

    st = op_add(l, h, node);
    if (st != STATUS_OK)
        free_liver(node);
    return st;
}

Status menu_modify(List *l, History *h){
    Liver *target = NULL;
    Status st = pick_liver(l, &target);
    if (st != STATUS_OK)
        return st;

    printf("Введите новые данные.\n");
    Liver *node = NULL;
    st = read_new_liver(&node);
    if (st != STATUS_OK)
        return st;

    st = op_modify(l, h, target, node);
    if (st != STATUS_OK)
        free_liver(node);
    return st;
}

Status menu_delete(List *l, History *h){
    Liver *target = NULL;
    Status st = pick_liver(l, &target);
    if (st != STATUS_OK)
        return st;
    return op_delete(l, h, target);
}

Status menu_export(const List *l, const char *input_path){
    printf("Введите путь к файлу для выгрузки:\n");
    char *path = NULL;
    Status st = read_trimmed(&path);
    if (st != STATUS_OK)
        return st;

    if (same_files(path, input_path)){
        free(path);
        return STATUS_INVALID_ARGUMENTS;
    }

    FILE *out = fopen(path, "w");
    free(path);
    if (out == NULL)
        return STATUS_FILE_ERROR;

    st = write_list(out, l);
    if (fclose(out) != 0 && st == STATUS_OK)
        st = STATUS_FILE_ERROR;
    if (st == STATUS_OK)
        printf("Данные выгружены.\n");
    return st;
}

Status menu_undo(List *l, History *h){
    size_t undone = 0;
    Status st = undo(l, h, &undone);
    if (st != STATUS_OK)
        return st;
    if (undone == 0)
        printf("Отменять нечего: нужно хотя бы 2 модификации.\n");
    else
        printf("Отменено модификаций: %zu\n", undone);
    return STATUS_OK;
}

int main(int argc, char *argv[]){
    if (argc != 2){
        printf("Введите путь к файлу с жителями.\n");
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (input == NULL){
        print_error(STATUS_FILE_ERROR);
        return 1;
    }

    List list;
    create_list(&list);

    Status st = create_array(input, &list);
    int input_close = fclose(input);

    if (st != STATUS_OK){
        print_error(st);
        return 1;
    }
    if (input_close != 0){
        print_error(STATUS_FILE_ERROR);
        free_list(&list);
        return 1;
    }

    History history;
    history_init(&history);
    int result = 0;

    while (1){
        printf("\nЖителей: %zu, модификаций с момента чтения/последней отмены: %zu\n", list.len_liver, history.count);
        printf("1. Показать список.\n2. Поиск.\n3. Добавить.\n4. Изменить.\n5. Удалить.\n6. Выгрузить в файл.\n7. Отменить последние N/2 модификаций.\n0. Выход.\n");

        char choice;
        st = read_choice(&choice);
        if (st != STATUS_OK){
            if (st != STATUS_INPUT_ERROR){
                print_error(st);
                result = 1;
            }
            break;
        }

        if (choice == '0')
            break;

        switch (choice){
            case '1': st = write_list(stdout, &list); break;
            case '2': st = menu_search(&list); break;
            case '3': st = menu_add(&list, &history); break;
            case '4': st = menu_modify(&list, &history); break;
            case '5': st = menu_delete(&list, &history); break;
            case '6': st = menu_export(&list, argv[1]); break;
            case '7': st = menu_undo(&list, &history); break;
            default:  st = STATUS_INVALID_FLAG; break;
        }

        if (st != STATUS_OK){
            print_error(st);
            if (st == STATUS_MEMORY_ERROR){
                result = 1;
                break;
            }
            if (st == STATUS_INPUT_ERROR)
                break;
        }
    }

    history_free(&history);
    free_list(&list);
    return result;
}