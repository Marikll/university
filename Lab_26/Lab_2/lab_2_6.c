#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#define TOKEN_START_CAP 16

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_RIM_NUM_LIMIT,
    STATUS_INVALID_NUMBER,
    STATUS_INVALID_FORMAT,
    STATUS_OUTPUT_ERROR,
    STATUS_MEMORY_ERROR,
    STATUS_OVERFLOW,
    STATUS_MISMATCH,
    STATUS_INVALID_ARGUMENTS,
    STATUS_EMPTY_STRING
} Status;

typedef struct{
    FILE *file;         
    const char *str; 
    size_t pos;       
} Source;

int src_get(Source *s){
    if (s->file != NULL)
        return fgetc(s->file);
    if (s->str[s->pos] == '\0')
        return EOF;
    return (unsigned char)s->str[s->pos++];
}

void src_unget(Source *s, int c){
    if (c == EOF)
        return;
    if (s->file != NULL)
        ungetc(c, s->file);
    else
        s->pos--;
}

void skip_spaces(Source *s){
    int c;
    while ((c = src_get(s)) != EOF && isspace(c)){
    }
    src_unget(s, c);
}

int accept_char(int c, int mode){
    if (mode == 0)
        return c == 'I' || c == 'V' || c == 'X' || c == 'L' || c == 'C' || c == 'D' || c == 'M';
    if (c >= '0' && c <= '9')
        return 1;
    if (mode == 1)
        return c >= 'a' && c <= 'z';
    return c >= 'A' && c <= 'Z';
}


Status token_push(char **buf, size_t *len, size_t *cap, char c){
    if (buf == NULL || *buf == NULL || len == NULL || cap == NULL)
        return STATUS_NULL_ARGUMENT;

    if (*len + 1 >= *cap){
        if (*cap > (size_t)-1 / 2 / sizeof(**buf))
            return STATUS_MEMORY_ERROR;
        char *tmp = realloc(*buf, *cap * 2 * sizeof(**buf));
        if (tmp == NULL)
            return STATUS_MEMORY_ERROR;
        *buf = tmp;
        *cap *= 2;
    }
    (*buf)[(*len)++] = c;
    (*buf)[*len] = '\0';
    return STATUS_OK;
}

Status read_token(Source *s, int mode, int allow_sign, char **token, size_t *len){
    if (s == NULL || token == NULL || len == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t cap = TOKEN_START_CAP;
    *len = 0;
    *token = malloc(cap * sizeof(**token));
    if (*token == NULL)
        return STATUS_MEMORY_ERROR;
    (*token)[0] = '\0';

    Status st = STATUS_OK;

    skip_spaces(s);
    int c = src_get(s);

    if (allow_sign && (c == '-' || c == '+')){
        st = token_push(token, len, &cap, (char)c);
        c = src_get(s);
    }
    while (st == STATUS_OK && c != EOF && accept_char(c, mode)){
        st = token_push(token, len, &cap, (char)c);
        if (st == STATUS_OK)
            c = src_get(s);
    }

    if (st != STATUS_OK){
        free(*token);
        *token = NULL;
        return st;
    }

    src_unget(s, c);
    return STATUS_OK;
}

int roman_digit(char c){
    switch (c){
        case 'I': return 1;
        case 'V': return 5;
        case 'X': return 10;
        case 'L': return 50;
        case 'C': return 100;
        case 'D': return 500;
        case 'M': return 1000;
        default:  return 0;      
    }
}

Status flag_Ro(Source *s, int *res){
    if (s == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    char *buf = NULL;
    size_t len = 0;
    Status st = read_token(s, 0, 0, &buf, &len);
    if (st != STATUS_OK)
        return st;

    if (len == 0){
        free(buf);
        return STATUS_INVALID_NUMBER;
    }

    long long total = 0;
    for (size_t i = 0; buf[i] != '\0'; i++){
        int cur = roman_digit(buf[i]);
        int next = roman_digit(buf[i + 1]);
        if (cur < next)
            total -= cur;
        else
            total += cur;

        if (total > INT_MAX || total < INT_MIN){
            st = STATUS_OVERFLOW;
            break;
        }
    }

    if (st == STATUS_OK)
        *res = (int)total;

    free(buf);
    return st;
}

Status flag_Zr(Source *s, unsigned int *res){
    if (s == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t cap = TOKEN_START_CAP;
    size_t len = 0;
    char *buf = malloc(cap * sizeof(*buf));
    if (buf == NULL)
        return STATUS_MEMORY_ERROR;
    buf[0] = '\0';

    Status st = STATUS_OK;
    char prev = 0;
    int c;

    skip_spaces(s);
    while ((c = src_get(s)) != EOF){
        if (c != '0' && c != '1'){
            src_unget(s, c);
            break;
        }
        st = token_push(&buf, &len, &cap, (char)c);
        if (st != STATUS_OK)
            break;
        if (c == '1' && prev == '1')
            break;
        prev = (char)c;
    }

    if (st == STATUS_OK && len < 2)
        st = STATUS_INVALID_NUMBER;

    unsigned long long result = 0;
    unsigned long long a = 1, b = 2;
    for (size_t i = 0; st == STATUS_OK && i + 1 < len; i++){
        if (buf[i] == '1'){
            if (a > UINT_MAX - result){
                st = STATUS_OVERFLOW;
                break;
            }
            result += a;
        }
        if (a <= UINT_MAX){               
            unsigned long long t = a + b;
            a = b;
            b = t;
        }
    }

    if (st == STATUS_OK)
        *res = (unsigned int)result;

    free(buf);
    return st;
}

Status flag_Cv(Source *s, int *res, int base, int upper){
    if (s == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    if (base < 2 || base > 36)
        base = 10;

    char *buf = NULL;
    size_t len = 0;
    Status st = read_token(s, upper ? 2 : 1, 1, &buf, &len);
    if (st != STATUS_OK)
        return st;

    size_t i = 0;
    int negative = 0;
    if (buf[0] == '-'){
        negative = 1;
        i++;
    } else if (buf[0] == '+'){
        i++;
    }

    if (buf[i] == '\0'){                    
        free(buf);
        return STATUS_INVALID_NUMBER;
    }

    unsigned long long limit = negative ? (unsigned long long)INT_MAX + 1ULL : (unsigned long long)INT_MAX;
    unsigned long long value = 0;
    for (; buf[i] != '\0'; i++){
        int digit;
        if (buf[i] <= '9')
            digit = buf[i] - '0';
        else if (buf[i] >= 'a')
            digit = buf[i] - 'a' + 10;
        else
            digit = buf[i] - 'A' + 10;

        if (digit >= base){
            st = STATUS_INVALID_NUMBER;
            break;
        }
        if (value > (limit - (unsigned long long)digit) / (unsigned long long)base){
            st = STATUS_OVERFLOW;
            break;
        }
        value = value * (unsigned long long)base + (unsigned long long)digit;
    }

    if (st == STATUS_OK)
        *res = negative ? (int)(-(long long)value) : (int)value;

    free(buf);
    return st;
}

Status std_scan(Source *s, const char *spec, void *ptr){
    size_t n = strlen(spec);
    char *spec2 = malloc((n + 3) * sizeof(*spec2));
    if (spec2 == NULL)
        return STATUS_MEMORY_ERROR;

    size_t k = 0;
    for (; spec[k] != '\0'; k++)
        spec2[k] = spec[k];
    spec2[k++] = '%';                   
    spec2[k++] = 'n';
    spec2[k] = '\0';

    int used = -1;                       

    if (s->file != NULL){
        if (ptr != NULL)
            fscanf(s->file, spec2, ptr, &used);
        else
            fscanf(s->file, spec2, &used);
    } else {
        if (ptr != NULL)
            sscanf(s->str + s->pos, spec2, ptr, &used);
        else
            sscanf(s->str + s->pos, spec2, &used);
        if (used >= 0)
            s->pos += (size_t)used;
    }

    free(spec2);
    return (used >= 0) ? STATUS_OK : STATUS_INVALID_NUMBER;
}

Status scan_core(Source *s, const char *format, va_list *values){
    size_t i = 0;

    while (format[i] != '\0'){

        if (isspace((unsigned char)format[i])){
            while (isspace((unsigned char)format[i]))
                i++;
            skip_spaces(s);
            continue;
        }

        if (format[i] != '%'){
            int c = src_get(s);
            if (c != (unsigned char)format[i]){
                src_unget(s, c);
                return STATUS_MISMATCH;
            }
            i++;
            continue;
        }

        size_t j = i + 1;                  
        int suppress = 0;
        if (format[j] == '*'){
            suppress = 1;
            j++;
        }

        char a = format[j];
        char b = (a != '\0') ? format[j + 1] : '\0';
        Status st;

        if (a == 'R' && b == 'o'){
            int tmp = 0;
            st = flag_Ro(s, &tmp);
            if (!suppress){
                int *p = va_arg(*values, int *);
                if (st == STATUS_OK)
                    *p = tmp;
            }
            i = j + 2;
        } else if (a == 'Z' && b == 'r'){
            unsigned int tmp = 0;
            st = flag_Zr(s, &tmp);
            if (!suppress){
                unsigned int *p = va_arg(*values, unsigned int *);
                if (st == STATUS_OK)
                    *p = tmp;
            }
            i = j + 2;
        } else if (a == 'C' && (b == 'v' || b == 'V')){
            int *p = NULL;
            if (!suppress)
                p = va_arg(*values, int *);
            int base = va_arg(*values, int);     
            int tmp = 0;
            st = flag_Cv(s, &tmp, base, b == 'V');
            if (p != NULL && st == STATUS_OK)
                *p = tmp;
            i = j + 2;
        } else if (a == '%'){
            skip_spaces(s);
            int c = src_get(s);
            if (c != '%'){
                src_unget(s, c);
                return STATUS_MISMATCH;
            }
            st = STATUS_OK;
            i = j + 1;
                } else {
            const char *allowed = "0123456789hlLm";     

            size_t spec_cap = strlen(format + j) + 4;   
            char *spec = malloc(spec_cap * sizeof(*spec));
            if (spec == NULL)
                return STATUS_MEMORY_ERROR;

            size_t len = 0;
            spec[len++] = '%';
            if (suppress)
                spec[len++] = '*';
            while (format[j] != '\0'){
                int ok = 0;
                for (size_t k = 0; allowed[k] != '\0'; k++)
                    if (format[j] == allowed[k])
                        ok = 1;
                if (!ok)
                    break;
                spec[len++] = format[j++];
            }

            char conv = format[j];
            const char *convs = "diuoxXfeEgGaAcsp";
            int known = 0;
            for (size_t k = 0; convs[k] != '\0'; k++)
                if (conv == convs[k])
                    known = 1;
            if (!known){
                free(spec);
                return STATUS_INVALID_FORMAT;
            }

            spec[len++] = conv;
            spec[len] = '\0';

            void *p = NULL;
            if (!suppress)
                p = va_arg(*values, void *);
            st = std_scan(s, spec, p);
            free(spec);
            i = j + 1;
        }

        if (st != STATUS_OK)
            return st;
    }
    return STATUS_OK;
}

Status overfscanf(FILE *stream, const char *format, ...){
    if (stream == NULL || format == NULL)
        return STATUS_NULL_ARGUMENT;

    Source src = {stream, NULL, 0};

    va_list values;
    va_start(values, format);
    Status st = scan_core(&src, format, &values);
    va_end(values);
    return st;
}

Status oversscanf(const char *str, const char *format, ...){
    if (str == NULL || format == NULL)
        return STATUS_NULL_ARGUMENT;

    Source src = {NULL, str, 0};

    va_list values;
    va_start(values, format);
    Status st = scan_core(&src, format, &values);
    va_end(values);
    return st;
}

const char *get_status_message(Status st) {
    switch (st) {
        case STATUS_OK:
            return "Операция выполнена успешно.";

        case STATUS_NULL_ARGUMENT:
            return "Ошибка: передан нулевой указатель.";

        case STATUS_RIM_NUM_LIMIT:
            return "Ошибка: римское число выходит за допустимые пределы.";

        case STATUS_INVALID_NUMBER:
            return "Ошибка: не удалось считать число.";

        case STATUS_INVALID_FORMAT:
            return "Ошибка: некорректный формат спецификатора.";

        case STATUS_OUTPUT_ERROR:
            return "Ошибка записи в поток вывода.";

        case STATUS_MEMORY_ERROR:
            return "Ошибка выделения динамической памяти.";

        case STATUS_OVERFLOW:
            return "Ошибка: переполнение числового типа.";

        case STATUS_MISMATCH:
            return "Ошибка: входные данные не соответствуют формату.";

        case STATUS_EMPTY_STRING:
            return "Передана пустая строка.";
        
        case STATUS_INVALID_ARGUMENTS:
            return "Ошибка: переданы некорректные аргументы.";

        default:
            return "Неизвестная ошибка.";
    }
}

void print_status_error(Status st){
    printf("%s\n", get_status_message(st));
}

int main(void){
    int roman = 0, num = 0, plain = 0;
    unsigned int zeck = 0;
    double real = 0.0;
    char *word = NULL;
    Status st;

    // st = oversscanf("MCMXCIV 1011 ff 255 3.5 hello", "%Ro %Zr %Cv %d %lf %ms", &roman, &zeck, &num, 16, &plain, &real, &word);
    st = oversscanf("1111111111 1 1", "%ms  %d", &word, &num);
    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }
    printf("ms=%s d=%d\n", word, num);
    // printf("Ro=%d Zr=%u Cv=%d d=%d f=%.1f ms=%s\n", roman, zeck, num, plain, real, word);
    free(word);
    word = NULL;

    FILE *f = tmpfile();
    if (f == NULL){
        printf("Не удалось создать файл.\n");
        return 1;
    }
    fputs("XIV 100011 -FF 42 world", f);
    rewind(f);

    st = overfscanf(f, "%Ro %Zr %CV %d %ms", &roman, &zeck, &num, 16, &plain, &word);
    fclose(f);
    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }
    printf("Ro=%d Zr=%u CV=%d d=%d ms=%s\n", roman, zeck, num, plain, word);
    free(word);
    word = NULL;

    st = oversscanf("ffffffff", "%Cv", &num, 16);
    if (st != STATUS_OK)
        print_status_error(st);

    st = oversscanf("abc", "%Ro", &roman);
    if (st != STATUS_OK)
        print_status_error(st);

    st = oversscanf("5", "%y", &plain);
    if (st != STATUS_OK)
        print_status_error(st);

    return 0;
}