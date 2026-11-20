#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

typedef struct Node {
    char *value;
    struct Node *parent;
    struct Node **children;
    size_t count;
    size_t capacity;
    size_t next;              
} Node;

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_MEMORY_ERROR,
    STATUS_FILE_ERROR,
    STATUS_INPUT_ERROR,
    STATUS_OUTPUT_ERROR,
    STATUS_EMPTY_STRING,
    STATUS_INVALID_ARGUMENTS,
    STATUS_INVALID_EXPRESSION,
    STATUS_SAME_FILE
} Status;

static int is_blank(int c){
    return c == ' ' || c == '\t' || c == '\r';
}

static int is_token_char(int c){
    return c != '\0' && !is_blank(c) && c != '(' && c != ')' && c != ',';
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

Status create_node(const char *s, size_t n, Node **out){
    if (s == NULL || out == NULL)
        return STATUS_NULL_ARGUMENT;
    if (n == 0)
        return STATUS_EMPTY_STRING;

    Node *node = malloc(sizeof(*node));
    if (node == NULL)
        return STATUS_MEMORY_ERROR;

    node->value = malloc((n + 1) * sizeof(*node->value));
    if (node->value == NULL){
        free(node);
        return STATUS_MEMORY_ERROR;
    }
    memcpy(node->value, s, n);
    node->value[n] = '\0';
    node->parent = NULL;
    node->children = NULL;
    node->count = 0;
    node->capacity = 0;
    node->next = 0;

    *out = node;
    return STATUS_OK;
}

Status add_child(Node *parent, Node *child){
    if (parent == NULL || child == NULL)
        return STATUS_NULL_ARGUMENT;

    if (parent->count == parent->capacity){
        if (parent->capacity > (size_t)-1 / 2 / sizeof(*parent->children))
            return STATUS_MEMORY_ERROR;
        size_t new_cap = (parent->capacity == 0) ? 4 : parent->capacity * 2;
        Node **tmp = realloc(parent->children, new_cap * sizeof(*tmp));
        if (tmp == NULL)
            return STATUS_MEMORY_ERROR;
        parent->children = tmp;
        parent->capacity = new_cap;
    }
    parent->children[parent->count++] = child;
    child->parent = parent;
    return STATUS_OK;
}

void free_node(Node *node){
    if (node == NULL)
        return;
    free(node->value);
    free(node->children);
    free(node);
}

void free_tree(Node *root){
    Node *cur = root;
    while (cur != NULL){
        if (cur->count > 0){
            cur = cur->children[--cur->count];
        } else {
            Node *parent = cur->parent;
            free_node(cur);
            cur = parent;
        }
    }
}

Status parse_expression(const char *line, Node **root){
    if (line == NULL || root == NULL)
        return STATUS_NULL_ARGUMENT;
    *root = NULL;

    Node *rt = NULL;
    Node *current = NULL;    
    size_t i = 0;
    int expect_node = 1;
    Status st = STATUS_OK;

    while (1){
        while (is_blank((unsigned char)line[i]))
            i++;

        if (expect_node){
            size_t n = 0;
            while (is_token_char((unsigned char)line[i + n]))
                n++;
            if (n == 0){
                st = STATUS_INVALID_EXPRESSION;
                break;
            }

            Node *node = NULL;
            st = create_node(line + i, n, &node);
            if (st != STATUS_OK)
                break;

            if (current == NULL){
                if (rt != NULL){              
                    free_node(node);
                    st = STATUS_INVALID_EXPRESSION;
                    break;
                }
                rt = node;
            } else {
                st = add_child(current, node);
                if (st != STATUS_OK){
                    free_node(node);
                    break;
                }
            }

            i += n;
            while (is_blank((unsigned char)line[i]))
                i++;

            if (line[i] == '('){
                current = node;
                i++;
            } else {
                expect_node = 0;
            }
        } else {
            char c = line[i];
            if (c == '\0'){
                if (current != NULL)         
                    st = STATUS_INVALID_EXPRESSION;
                break;
            }
            if (c == ',' && current != NULL){
                expect_node = 1;
                i++;
            } else if (c == ')' && current != NULL){
                current = current->parent;
                i++;
            } else {
                st = STATUS_INVALID_EXPRESSION;
                break;
            }
        }
    }

    if (st == STATUS_OK && rt == NULL)
        st = STATUS_INVALID_EXPRESSION;

    if (st != STATUS_OK){
        free_tree(rt);
        return st;
    }
    *root = rt;
    return STATUS_OK;
}

Status put_text(FILE *out, const char *s, size_t n){
    if (n > 0 && fwrite(s, sizeof(*s), n, out) != n)
        return STATUS_OUTPUT_ERROR;
    return STATUS_OK;
}

Status write_tree(FILE *out, Node *root){
    if (out == NULL || root == NULL)
        return STATUS_NULL_ARGUMENT;

    if (fprintf(out, "%s\n", root->value) < 0)
        return STATUS_OUTPUT_ERROR;

    size_t capacity = 64;
    size_t len = 0;
    char *prefix = malloc(capacity * sizeof(*prefix));
    if (prefix == NULL)
        return STATUS_MEMORY_ERROR;

    Status st = STATUS_OK;
    Node *cur = root;

    while (cur != NULL && st == STATUS_OK){
        if (cur->next < cur->count){
            Node *child = cur->children[cur->next++];
            int last = (cur->next == cur->count);

            st = put_text(out, prefix, len);
            if (st == STATUS_OK && fprintf(out, "%s%s\n", last ? "`-- " : "|-- ", child->value) < 0)
                st = STATUS_OUTPUT_ERROR;
            if (st != STATUS_OK)
                break;

            if (len + 4 > capacity){
                if (capacity > (size_t)-1 / 2 / sizeof(*prefix)){
                    st = STATUS_MEMORY_ERROR;
                    break;
                }
                char *tmp = realloc(prefix, capacity * 2 * sizeof(*tmp));
                if (tmp == NULL){
                    st = STATUS_MEMORY_ERROR;
                    break;
                }
                prefix = tmp;
                capacity *= 2;
            }
            memcpy(prefix + len, last ? "    " : "|   ", 4);
            len += 4;
            cur = child;
        } else {
            if (cur->parent != NULL)
                len -= 4;
            cur = cur->parent;
        }
    }

    free(prefix);
    return st;
}

int same_files(const char *path1, const char *path2){
    struct stat s1, s2;

    if (path1 == NULL || path2 == NULL)
        return 0;

    if (stat(path1, &s1) != 0 || stat(path2, &s2) != 0)
        return 0;

    return s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino;
}

int extension_equal(const char *first, const char *second){
    while (*first != '\0' && *second != '\0') {
        if (tolower((unsigned char)*first) != tolower((unsigned char)*second))
            return 0;
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

Status process_file(FILE *input, FILE *output, size_t *line_no, size_t *count){
    if (input == NULL || output == NULL || line_no == NULL || count == NULL)
        return STATUS_NULL_ARGUMENT;

    *line_no = 0;
    *count = 0;

    while (1){
        char *line = NULL;
        int end = 0;

        Status st = read_line(input, &line, &end);
        if (st != STATUS_OK)
            return st;
        if (end)
            break;

        (*line_no)++;

        size_t k = 0;
        while (is_blank((unsigned char)line[k]))
            k++;
        if (line[k] == '\0'){
            free(line);
            continue;
        }

        Node *root = NULL;
        st = parse_expression(line, &root);
        if (st != STATUS_OK){
            free(line);
            return st;
        }

        if (fprintf(output, "Выражение %zu: %s\n", *count + 1, line) < 0)
            st = STATUS_OUTPUT_ERROR;
        free(line);

        if (st == STATUS_OK)
            st = write_tree(output, root);
        if (st == STATUS_OK && fprintf(output, "\n") < 0)
            st = STATUS_OUTPUT_ERROR;

        free_tree(root);
        if (st != STATUS_OK)
            return st;

        (*count)++;
    }
    return STATUS_OK;
}

void print_error(Status status){
    switch (status){
        case STATUS_NULL_ARGUMENT:      printf("Внутренняя ошибка: нулевой указатель.\n"); break;
        case STATUS_MEMORY_ERROR:       printf("Ошибка: не удалось выделить память.\n"); break;
        case STATUS_FILE_ERROR:         printf("Ошибка работы с файлом.\n"); break;
        case STATUS_INPUT_ERROR:        printf("Ошибка чтения входного файла.\n"); break;
        case STATUS_OUTPUT_ERROR:       printf("Ошибка записи в выходной файл.\n"); break;
        case STATUS_EMPTY_STRING:       printf("Ошибка: передана пустая строка.\n"); break;
        case STATUS_INVALID_ARGUMENTS:  printf("Ошибка: запрещённое расширение файла.\n"); break;
        case STATUS_INVALID_EXPRESSION: printf("Ошибка: некорректное скобочное выражение.\n"); break;
        case STATUS_SAME_FILE:          printf("Ошибка: входной и выходной файлы совпадают.\n"); break;
        default:                        printf("Ошибка.\n"); break;
    }
}

int main(int argc, char *argv[]){
    if (argc != 3){
        printf("Введите путь к входному файлу и путь к выходному файлу.\n");
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
        print_error(STATUS_SAME_FILE);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (input == NULL){
        printf("Ошибка: не удалось открыть входной файл.\n");
        return 1;
    }

    FILE *output = fopen(argv[2], "w");
    if (output == NULL){
        printf("Ошибка: не удалось открыть выходной файл.\n");
        fclose(input);
        return 1;
    }

    size_t line_no = 0;
    size_t count = 0;
    st = process_file(input, output, &line_no, &count);

    int input_close = fclose(input);
    int output_close = fclose(output);

    if (st != STATUS_OK){
        print_error(st);
        if (st == STATUS_INVALID_EXPRESSION)
            printf("Строка файла: %zu\n", line_no);
        return 1;
    }
    if (input_close != 0 || output_close != 0){
        printf("Ошибка закрытия файла.\n");
        return 1;
    }

    if (count == 0)
        printf("В файле нет выражений.\n");
    else
        printf("Обработано выражений: %zu\n", count);
    return 0;
}