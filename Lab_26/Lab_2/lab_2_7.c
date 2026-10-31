#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

typedef enum{
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_DIMENSION,
    STATUS_INVALID_EPS,
    STATUS_INVALID_COUNT,
    STATUS_INVALID_P,
    STATUS_NOT_POSITIVE_DEFINITE,
    STATUS_NUMERIC_OVERFLOW,
    STATUS_MEMORY_ERROR
} Status;

typedef struct{
    const double *coords;              
} Vector;

typedef Status (*NormFunc)(const double *x, size_t n, const void *params, double *result);

typedef struct{
    NormFunc func;
    const void *params;              
} NormSpec;

typedef struct{
    size_t *indices;                   
    size_t count;
    double length;
} NormResult;

Status norm_inf(const double *x, size_t n, const void *params, double *result){
    (void)params;
    if (x == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;

    double max = 0.0;
    for (size_t j = 0; j < n; j++)
        if (fabs(x[j]) > max)
            max = fabs(x[j]);

    *result = max;
    return STATUS_OK;
}

Status norm_p(const double *x, size_t n, const void *params, double *result){
    if (x == NULL || params == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;

    double p = *(const double *)params;
    if (p < 1.0)
        return STATUS_INVALID_P;

    double sum = 0.0;
    for (size_t j = 0; j < n; j++)
        sum += pow(fabs(x[j]), p);

    double value = pow(sum, 1.0 / p);
    if (!isfinite(value))
        return STATUS_NUMERIC_OVERFLOW;

    *result = value;
    return STATUS_OK;
}

Status norm_a(const double *x, size_t n, const void *params, double *result){
    if (x == NULL || params == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;

    const double *a = (const double *)params;  
    double sum = 0.0;

    for (size_t i = 0; i < n; i++){
        double row = 0.0;                      
        for (size_t j = 0; j < n; j++)
            row += a[i * n + j] * x[j];
        sum += row * x[i];
    }

    if (sum < 0.0)
        return STATUS_NOT_POSITIVE_DEFINITE;

    double value = sqrt(sum);
    if (!isfinite(value))
        return STATUS_NUMERIC_OVERFLOW;

    *result = value;
    return STATUS_OK;
}

Status check_positive_definite(const double *a, size_t n, double eps){
    if (a == NULL)
        return STATUS_NULL_ARGUMENT;
    if (n == 0 || n > (size_t)-1 / n / sizeof(double))
        return STATUS_INVALID_DIMENSION;
    if (eps <= 0.0)
        return STATUS_INVALID_EPS;

    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < i; j++)
            if (fabs(a[i * n + j] - a[j * n + i]) > eps)
                return STATUS_NOT_POSITIVE_DEFINITE;

    double *l = calloc(n * n, sizeof(double));
    if (l == NULL)
        return STATUS_MEMORY_ERROR;

    for (size_t i = 0; i < n; i++){
        for (size_t j = 0; j <= i; j++){
            double sum = a[i * n + j];
            for (size_t k = 0; k < j; k++)
                sum -= l[i * n + k] * l[j * n + k];

            if (i == j){
                if (sum <= eps){                
                    free(l);
                    return STATUS_NOT_POSITIVE_DEFINITE;
                }
                l[i * n + i] = sqrt(sum);
            } else {
                l[i * n + j] = sum / l[j * n + j];
            }
        }
    }

    free(l);
    return STATUS_OK;
}

void free_results(NormResult *results, size_t count){
    if (results == NULL)
        return;
    for (size_t i = 0; i < count; i++)
        free(results[i].indices);
    free(results);
}

Status longest_vectors(size_t n, double eps, size_t vec_count, size_t norm_count, NormResult **results, ...){
    if (results == NULL)
        return STATUS_NULL_ARGUMENT;
    if (n == 0)
        return STATUS_INVALID_DIMENSION;
    if (eps <= 0.0)
        return STATUS_INVALID_EPS;
    if (vec_count == 0 || norm_count == 0 ||
        vec_count > (size_t)-1 / sizeof(Vector) ||
        norm_count > (size_t)-1 / sizeof(NormSpec) ||
        norm_count > (size_t)-1 / sizeof(NormResult) ||
        vec_count > (size_t)-1 / sizeof(double) ||
        vec_count > (size_t)-1 / sizeof(size_t))
        return STATUS_INVALID_COUNT;

    Vector *vectors = malloc(vec_count * sizeof(Vector));
    NormSpec *norms = malloc(norm_count * sizeof(NormSpec));
    NormResult *res = calloc(norm_count, sizeof(NormResult));
    double *lengths = malloc(vec_count * sizeof(double));

    if (vectors == NULL || norms == NULL || res == NULL || lengths == NULL){
        free(vectors);
        free(norms);
        free(res);
        free(lengths);
        return STATUS_MEMORY_ERROR;
    }

    va_list args;
    va_start(args, results);
    for (size_t i = 0; i < vec_count; i++)
        vectors[i] = va_arg(args, Vector);
    for (size_t k = 0; k < norm_count; k++)
        norms[k] = va_arg(args, NormSpec);
    va_end(args);

    Status st = STATUS_OK;

    for (size_t k = 0; k < norm_count && st == STATUS_OK; k++){
        if (norms[k].func == NULL){
            st = STATUS_NULL_ARGUMENT;
            break;
        }

        double max = 0.0;
        for (size_t i = 0; i < vec_count && st == STATUS_OK; i++){
            st = norms[k].func(vectors[i].coords, n, norms[k].params, &lengths[i]);
            if (st == STATUS_OK && (i == 0 || lengths[i] > max))
                max = lengths[i];
        }
        if (st != STATUS_OK)
            break;

        res[k].indices = malloc(vec_count * sizeof(size_t));
        if (res[k].indices == NULL){
            st = STATUS_MEMORY_ERROR;
            break;
        }

        res[k].length = max;
        res[k].count = 0;
        for (size_t i = 0; i < vec_count; i++)
            if (fabs(lengths[i] - max) <= eps)
                res[k].indices[res[k].count++] = i;
    }

    free(vectors);
    free(norms);
    free(lengths);

    if (st != STATUS_OK){
        free_results(res, norm_count);
        return st;
    }

    *results = res;
    return STATUS_OK;
}

void print_status_error(Status status){
    switch (status){
        case STATUS_NULL_ARGUMENT:        printf("Внутренняя ошибка: нулевой указатель.\n"); break;
        case STATUS_INVALID_DIMENSION:    printf("Ошибка: некорректная размерность.\n"); break;
        case STATUS_INVALID_EPS:          printf("Ошибка: эпсилон должен быть положительным.\n"); break;
        case STATUS_INVALID_COUNT:        printf("Ошибка: некорректное количество векторов или норм.\n"); break;
        case STATUS_INVALID_P:            printf("Ошибка: для p-нормы нужно p >= 1.\n"); break;
        case STATUS_NOT_POSITIVE_DEFINITE:printf("Ошибка: матрица не является положительно определённой.\n"); break;
        case STATUS_NUMERIC_OVERFLOW:     printf("Ошибка: переполнение при вычислении нормы.\n"); break;
        case STATUS_MEMORY_ERROR:         printf("Ошибка: не удалось выделить память.\n"); break;
        default:                          printf("Ошибка.\n"); break;
    }
}

void print_results(const char **names, const NormResult *results, size_t norm_count){
    for (size_t k = 0; k < norm_count; k++){
        printf("%s: длина %.4f, векторы:", names[k], results[k].length);
        for (size_t i = 0; i < results[k].count; i++)
            printf(" v%zu", results[k].indices[i] + 1);
        printf("\n");
    }
}

int main(void){
    const size_t n = 3;
    const double eps = 1e-9;

    const double c1[] = {1, 2, 2};
    const double c2[] = {3, 0, 0};
    const double c3[] = {0, 0, 1};
    const double c4[] = {-3, 0, 0};

    Vector v1 = {c1}, v2 = {c2}, v3 = {c3}, v4 = {c4};

    const double p1 = 1.0, p2 = 2.0;
    const double a[] = {2, 1, 0, 1, 2, 0, 0, 0, 3};

    Status st = check_positive_definite(a, n, eps);
    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }

    NormSpec s_inf = {norm_inf, NULL};
    NormSpec s_p1  = {norm_p, &p1};
    NormSpec s_p2  = {norm_p, &p2};
    NormSpec s_a   = {norm_a, a};

    const char *names[] = {"inf", "p=1", "p=2", "A"};

    NormResult *res = NULL;
    st = longest_vectors(n, eps, 4, 4, &res, v1, v2, v3, v4, s_inf, s_p1, s_p2, s_a);
    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }

    print_results(names, res, 4);
    free_results(res, 4);

    const double bad_p = 0.5;
    NormSpec s_bad = {norm_p, &bad_p};
    st = longest_vectors(n, eps, 2, 1, &res, v1, v2, s_bad);
    if (st != STATUS_OK)
        print_status_error(st);
    else
        free_results(res, 1);

    return 0;
}