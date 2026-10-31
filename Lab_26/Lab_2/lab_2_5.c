#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#define RESULT_CAP 256

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_RIM_NUM_LIMIT,
    STATUS_INVALID_NUMBER,
    STATUS_INVALID_FORMAT,
    STATUS_OUTPUT_ERROR,
    STATUS_MEMORY_ERROR,
    STATUS_OVERFLOW
} Status;

typedef struct{
    char *data;
    size_t len;
    size_t cap;
} Buffer;

Status buffer_init(Buffer *b){
    if (b == NULL)
        return STATUS_NULL_ARGUMENT;
    b->cap = 64;
    b->len = 0;
    b->data = malloc(b->cap);
    if (b->data == NULL)
        return STATUS_MEMORY_ERROR;
    b->data[0] = '\0';
    return STATUS_OK;
}

Status buffer_push(Buffer *b, char c){
    if (b == NULL)
        return STATUS_NULL_ARGUMENT;
    if (b->len + 1 >= b->cap){
        if (b->cap > (size_t)-1 / 2)
            return STATUS_MEMORY_ERROR;
        char *tmp = realloc(b->data, b->cap * 2);
        if (tmp == NULL)
            return STATUS_MEMORY_ERROR;
        b->data = tmp;
        b->cap *= 2;
    }
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
    return STATUS_OK;
}

Status buffer_append(Buffer *b, const char *s){
    if (b == NULL || s == NULL)
        return STATUS_NULL_ARGUMENT;
    for (size_t i = 0; s[i] != '\0'; i++){
        Status st = buffer_push(b, s[i]);
        if (st != STATUS_OK)
            return st;
    }
    return STATUS_OK;
}

size_t string_length(const char *str){
    size_t n = 0;
    while (str[n] != '\0')
        n++;
    return n;
}

Status flag_Ro(int val, Buffer *out){
    if (out == NULL)
        return STATUS_NULL_ARGUMENT;

    if (val <= 0 || val > 3999)
        return STATUS_RIM_NUM_LIMIT;

    static const int values[] = {1000,900,500,400,100,90,50,40,10,9,5,4,1};
    static const char *symbols[] = {"M","CM","D","CD","C","XC","L","XL","X","IX","V","IV","I"};

    for (int i = 0; i < 13; i++) {
        while (val >= values[i]) {
            Status st = buffer_append(out, symbols[i]);
            if (st != STATUS_OK)
                return st;
            val -= values[i];
        }
    }
    return STATUS_OK;
}

Status flag_Zr(unsigned int val, Buffer *out){
    if (out == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t max_count = 2 * sizeof(val) * CHAR_BIT + 2;

    unsigned long long *fib = malloc(max_count * sizeof(*fib));
    char *bits = malloc(max_count * sizeof(*bits));
    if (fib == NULL || bits == NULL){
        free(fib);
        free(bits);
        return STATUS_MEMORY_ERROR;
    }

    size_t n = 0;
    if (val >= 1)
        fib[n++] = 1;
    if (val >= 2)
        fib[n++] = 2;
    while (n >= 2 && n < max_count && fib[n - 1] + fib[n - 2] <= val){
        fib[n] = fib[n - 1] + fib[n - 2];
        n++;
    }

    unsigned long long rest = val;
    for (size_t i = n; i > 0; i--){
        if (fib[i - 1] <= rest){
            bits[i - 1] = '1';
            rest -= fib[i - 1];
        } else {
            bits[i - 1] = '0';
        }
    }

    Status st = STATUS_OK;
    for (size_t i = 0; i < n && st == STATUS_OK; i++)
        st = buffer_push(out, bits[i]);
    if (st == STATUS_OK)
        st = buffer_push(out, '1');

    free(fib);
    free(bits);
    return st;
}

Status flag_Cv(int val, int base, int upper, Buffer *out){
    if (out == NULL)
        return STATUS_NULL_ARGUMENT;

    if (base < 2 || base > 36)
        base = 10;

    const char *digits = upper ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" : "0123456789abcdefghijklmnopqrstuvwxyz";

    size_t max_digits = sizeof(val) * CHAR_BIT;
    char *res = malloc(max_digits * sizeof(*res));
    if (res == NULL)
        return STATUS_MEMORY_ERROR;

    int count = 0;
    int v = val;

    if (v == 0)
        res[count++] = '0';
    while (v != 0){
        int d = v % base;
        if (d < 0)
            d = -d;
        res[count++] = digits[d];
        v /= base;
    }

    Status st = STATUS_OK;
    if (val < 0)
        st = buffer_push(out, '-');
    while (st == STATUS_OK && count > 0)
        st = buffer_push(out, res[--count]);

    free(res);
    return st;
}

Status flag_to(const char *val, int base, int upper, Buffer *out){
    if (val == NULL || out == NULL)
        return STATUS_NULL_ARGUMENT;

    if (base < 2 || base > 36)
        base = 10;

    int i = 0;
    int negative = 0;
    if (val[i] == '-'){
        negative = 1;
        i++;
    }
    if (val[i] == '\0')
        return STATUS_INVALID_NUMBER;

    long long num = 0;
    for (; val[i] != '\0'; i++){
        char c = val[i];
        int digit;

        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (!upper && c >= 'a' && c <= 'z')
            digit = c - 'a' + 10;
        else if (upper && c >= 'A' && c <= 'Z')
            digit = c - 'A' + 10;
        else
            return STATUS_INVALID_NUMBER;

        if (digit >= base)
            return STATUS_INVALID_NUMBER;

        if (num > (LLONG_MAX - digit) / base)
            return STATUS_OVERFLOW;

        num = num * base + digit;
    }
    if (negative)
        num = -num;

    size_t max_digits = sizeof(num) * CHAR_BIT;
    char *res = malloc(max_digits * sizeof(*res));
    if (res == NULL)
        return STATUS_MEMORY_ERROR;

    int count = 0;
    long long v = num;
    if (v == 0)
        res[count++] = '0';
    while (v != 0){
        int d = (int)(v % 10);
        if (d < 0)
            d = -d;
        res[count++] = (char)('0' + d);
        v /= 10;
    }

    Status st = STATUS_OK;
    if (num < 0)
        st = buffer_push(out, '-');
    while (st == STATUS_OK && count > 0)
        st = buffer_push(out, res[--count]);

    free(res);
    return st;
}

Status dump_bytes(const void *ptr, size_t n, Buffer *out){
    if (ptr == NULL || out == NULL)
        return STATUS_NULL_ARGUMENT;

    const unsigned char *bytes = (const unsigned char *)ptr;

    for (size_t i = 0; i < n; i++){
        if (i > 0){
            Status st = buffer_push(out, ' ');
            if (st != STATUS_OK)
                return st;
        }
        for (int bit = 7; bit >= 0; bit--){
            Status st = buffer_push(out, ((bytes[i] >> bit) & 1) ? '1' : '0');
            if (st != STATUS_OK)
                return st;
        }
    }
    return STATUS_OK;
}

Status append_formatted(Buffer *out, const char *spec, ...){
    va_list ap, ap2;
    va_start(ap, spec);
    va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, spec, ap);     
    va_end(ap);

    if (n < 0){
        va_end(ap2);
        return STATUS_OUTPUT_ERROR;
    }

    char *tmp = malloc((size_t)n + 1);
    if (tmp == NULL){
        va_end(ap2);
        return STATUS_MEMORY_ERROR;
    }
    vsnprintf(tmp, (size_t)n + 1, spec, ap2);
    va_end(ap2);

    Status st = buffer_append(out, tmp);
    free(tmp);
    return st;
}

Status flag_standard(const char *format, size_t *l, va_list *values, Buffer *out){
    size_t i = *l + 1;

    if (format[i] == '%'){
        *l = i;
        return buffer_push(out, '%');
    }

    size_t spec_cap = string_length(format + i) + 3;
    char *spec = malloc(spec_cap);
    if (spec == NULL)
        return STATUS_MEMORY_ERROR;

    const char *allowed = "-+ #.0123456789hlL";
    size_t s = 0;
    spec[s++] = '%';

    int longs = 0;
    int big = 0;
    while (format[i] != '\0'){
        int ok = 0;
        for (size_t k = 0; allowed[k] != '\0'; k++)
            if (format[i] == allowed[k])
                ok = 1;
        if (!ok)
            break;
        if (format[i] == 'l')
            longs++;
        if (format[i] == 'L')
            big = 1;
        spec[s++] = format[i++];
    }

    char conv = format[i];
    spec[s++] = conv;
    spec[s] = '\0';

    Status st;

    switch (conv){
        case 'd': case 'i':
            if (longs == 0)
                st = append_formatted(out, spec, va_arg(*values, int));
            else if (longs == 1)
                st = append_formatted(out, spec, va_arg(*values, long));
            else
                st = append_formatted(out, spec, va_arg(*values, long long));
            break;
        case 'u': case 'o': case 'x': case 'X':
            if (longs == 0)
                st = append_formatted(out, spec, va_arg(*values, unsigned int));
            else if (longs == 1)
                st = append_formatted(out, spec, va_arg(*values, unsigned long));
            else
                st = append_formatted(out, spec, va_arg(*values, unsigned long long));
            break;
        case 'c':
            st = append_formatted(out, spec, va_arg(*values, int));
            break;
        case 's':
            st = append_formatted(out, spec, va_arg(*values, const char *));
            break;
        case 'p':
            st = append_formatted(out, spec, va_arg(*values, void *));
            break;
        case 'f': case 'F': case 'e': case 'E': case 'g': case 'G':
            if (big)
                st = append_formatted(out, spec, va_arg(*values, long double));
            else
                st = append_formatted(out, spec, va_arg(*values, double));
            break;
        default:
            st = STATUS_INVALID_FORMAT;
            break;
    }

    free(spec);
    if (st == STATUS_OK)
        *l = i;
    return st;
}

Status format_to_buffer(const char *format, va_list *values, Buffer *out){
    for (size_t l = 0; format[l] != '\0'; l++){
        if (format[l] != '%'){
            Status st = buffer_push(out, format[l]);
            if (st != STATUS_OK)
                return st;
            continue;
        }

        char a = format[l + 1];
        char b = (a != '\0') ? format[l + 2] : '\0';
        Status st;

        if (a == 'R' && b == 'o'){
            int value = va_arg(*values, int);
            st = flag_Ro(value, out);
            l += 2;
        } else if (a == 'Z' && b == 'r'){
            unsigned int value = va_arg(*values, unsigned int);
            st = flag_Zr(value, out);
            l += 2;
        } else if (a == 'C' && (b == 'v' || b == 'V')){
            int value = va_arg(*values, int);
            int base = va_arg(*values, int);
            st = flag_Cv(value, base, b == 'V', out);
            l += 2;
        } else if (a == 't' && b == 'o'){
            const char *value = va_arg(*values, const char *);
            int base = va_arg(*values, int);
            st = flag_to(value, base, 0, out);
            l += 2;
        } else if (a == 'T' && b == 'O'){
            const char *value = va_arg(*values, const char *);
            int base = va_arg(*values, int);
            st = flag_to(value, base, 1, out);
            l += 2;
        } else if (a == 'm' && b == 'i'){
            int value = va_arg(*values, int);
            st = dump_bytes(&value, sizeof(value), out);
            l += 2;
        } else if (a == 'm' && b == 'u'){
            unsigned int value = va_arg(*values, unsigned int);
            st = dump_bytes(&value, sizeof(value), out);
            l += 2;
        } else if (a == 'm' && b == 'd'){
            double value = va_arg(*values, double);
            st = dump_bytes(&value, sizeof(value), out);
            l += 2;
        } else if (a == 'm' && b == 'f'){
            float value = (float)va_arg(*values, double);
            st = dump_bytes(&value, sizeof(value), out);
            l += 2;
        } else {
            st = flag_standard(format, &l, values, out);
        }

        if (st != STATUS_OK)
            return st;
    }
    return STATUS_OK;
}

Status overfprintf(FILE *stream, const char *format, ...){
    if (stream == NULL || format == NULL)
        return STATUS_NULL_ARGUMENT;

    Buffer buf;
    Status st = buffer_init(&buf);
    if (st != STATUS_OK)
        return st;

    va_list values;
    va_start(values, format);
    st = format_to_buffer(format, &values, &buf);
    va_end(values);

    if (st == STATUS_OK && fputs(buf.data, stream) == EOF)
        st = STATUS_OUTPUT_ERROR;

    free(buf.data);
    return st;
}

Status oversprintf(char *str, const char *format, ...){
    if (str == NULL || format == NULL)
        return STATUS_NULL_ARGUMENT;

    Buffer buf;
    Status st = buffer_init(&buf);
    if (st != STATUS_OK)
        return st;

    va_list values;
    va_start(values, format);
    st = format_to_buffer(format, &values, &buf);
    va_end(values);

    if (st == STATUS_OK){
        size_t i = 0;
        for (; i < buf.len; i++)
            str[i] = buf.data[i];
        str[i] = '\0';
    }

    free(buf.data);
    return st;
}


void print_status_error(Status status){
    switch (status){
        case STATUS_NULL_ARGUMENT:  printf("Внутренняя ошибка: нулевой указатель.\n"); break;
        case STATUS_RIM_NUM_LIMIT:  printf("Ошибка: римскими цифрами записываются числа от 1 до 3999.\n"); break;
        case STATUS_INVALID_NUMBER: printf("Ошибка: некорректное число для заданной системы счисления.\n"); break;
        case STATUS_INVALID_FORMAT: printf("Ошибка: некорректный спецификатор формата.\n"); break;
        case STATUS_OUTPUT_ERROR:   printf("Ошибка вывода.\n"); break;
        case STATUS_MEMORY_ERROR:   printf("Ошибка: не удалось выделить память.\n"); break;
        case STATUS_OVERFLOW:       printf("Ошибка: переполнение.\n"); break;
        case STATUS_OK:             break;
        default:                    printf("Ошибка.\n"); break;
    }
}

int main(void){
    Status st;

    st = overfprintf(stdout, "Zr: %Zr | Ro: %Ro\n", 100, 1994);
    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }

    st = overfprintf(stdout, "Cv: %Cv | CV: %CV | -255 по осн. 2: %Cv\n", 255, 16, 255, 16, -255, 2);
    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }

    st = overfprintf(stdout, "to: %to | TO: %TO\n", "ff", 16, "FF", 16);
    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }

    st = overfprintf(stdout, "mi: %mi\nmu: %mu\nmf: %mf\nmd: %md\n", -1, 1u, 1.0f, 1.0);
    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }

    char *result = malloc(RESULT_CAP * sizeof(*result));
    if (result == NULL){
        print_status_error(STATUS_MEMORY_ERROR);
        return 1;
    }

    st = oversprintf(result, "Станд.: %d %5.2f %s %c %%", 42, 3.14159, "hi", 'x');
    if (st != STATUS_OK){
        print_status_error(st);
        free(result);
        return 1;
    }
    printf("%s\n", result);
    free(result);

    st = overfprintf(stdout, "%Ro\n", 0);
    if (st != STATUS_OK)
        print_status_error(st);

    st = overfprintf(stdout, "%to\n", "zz", 16);
    if (st != STATUS_OK)
        print_status_error(st);

    st = overfprintf(stdout, "%y\n", 1);
    if (st != STATUS_OK)
        print_status_error(st);

    return 0;
}