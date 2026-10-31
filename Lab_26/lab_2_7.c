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
    STATUS_INVALID_NUMBER,
    STATUS_NOT_POSITIVE_DEFINITE,
    STATUS_NUMERIC_OVERFLOW,
    STATUS_MEMORY_ERROR
} Status;

typedef struct{
    const double *coords;              
    size_t dim;                       
} Vector;

typedef struct{
    const double *data;                
    size_t dim;
    double eps;                       
} Matrix;

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

Status check_finite(const double *x, size_t n){
    if (x == NULL)
        return STATUS_NULL_ARGUMENT;
    for (size_t j = 0; j < n; j++)
        if (!isfinite(x[j]))
            return STATUS_INVALID_NUMBER;
    return STATUS_OK;
}

Status norm_inf(const double *x, size_t n, const void *params, double *result){
    (void)params;
    if (x == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;

    Status st = check_finite(x, n);
    if (st != STATUS_OK)
        return st;

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
    if (!isfinite(p) || p < 1.0)
        return STATUS_INVALID_P;

    Status st = check_finite(x, n);
    if (st != STATUS_OK)
        return st;

    double max = 0.0;
    for (size_t j = 0; j < n; j++)
        if (fabs(x[j]) > max)
            max = fabs(x[j]);

    if (max == 0.0){
        *result = 0.0;
        return STATUS_OK;
    }

    double sum = 0.0;
    for (size_t j = 0; j < n; j++)
        sum += pow(fabs(x[j]) / max, p);

    double value = max * pow(sum, 1.0 / p);
    if (!isfinite(value))
        return STATUS_NUMERIC_OVERFLOW;

    *result = value;
    return STATUS_OK;
}

Status check_positive_definite(const Matrix *m, size_t n){
    if (m == NULL || m->data == NULL)
        return STATUS_NULL_ARGUMENT;
    if (n == 0 || m->dim != n || n > (size_t)-1 / n)
        return STATUS_INVALID_DIMENSION;
    if (!isfinite(m->eps) || m->eps <= 0.0)
        return STATUS_INVALID_EPS;

    size_t total = n * n;
    if (total > (size_t)-1 / sizeof(double))
        return STATUS_INVALID_DIMENSION;

    Status st = check_finite(m->data, total);
    if (st != STATUS_OK)
        return st;

    const double *a = m->data;

    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < i; j++)
            if (fabs(a[i * n + j] - a[j * n + i]) > m->eps)
                return STATUS_NOT_POSITIVE_DEFINITE;

    double *l = calloc(total, sizeof(*l));
    if (l == NULL)
        return STATUS_MEMORY_ERROR;

    st = STATUS_OK;
    for (size_t i = 0; i < n && st == STATUS_OK; i++){
        for (size_t j = 0; j <= i; j++){
            double sum = a[i * n + j];
            for (size_t k = 0; k < j; k++)
                sum -= l[i * n + k] * l[j * n + k];

            if (i == j){
                if (!isfinite(sum) || sum <= m->eps){
                    st = STATUS_NOT_POSITIVE_DEFINITE;
                    break;
                }
                l[i * n + i] = sqrt(sum);
            } else {
                l[i * n + j] = sum / l[j * n + j];
            }
        }
    }

    free(l);
    return st;
}

Status norm_a(const double *x, size_t n, const void *params, double *result){
    if (x == NULL || params == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;

    const Matrix *m = (const Matrix *)params;

    Status st = check_positive_definite(m, n);
    if (st != STATUS_OK)
        return st;

    st = check_finite(x, n);
    if (st != STATUS_OK)
        return st;

    double sum = 0.0;
    for (size_t i = 0; i < n; i++){
        double row = 0.0;                       
        for (size_t j = 0; j < n; j++)
            row += m->data[i * n + j] * x[j];
        if (!isfinite(row))
            return STATUS_NUMERIC_OVERFLOW;
        sum += row * x[i];
        if (!isfinite(sum))
            return STATUS_NUMERIC_OVERFLOW;
    }

    if (sum < 0.0)                            
        sum = 0.0;

    double value = sqrt(sum);
    if (!isfinite(value))
        return STATUS_NUMERIC_OVERFLOW;

    *result = value;
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
    *results = NULL;

    if (n == 0)
        return STATUS_INVALID_DIMENSION;
    if (!isfinite(eps) || eps <= 0.0)
        return STATUS_INVALID_EPS;
    if (vec_count == 0 || norm_count == 0 ||
        vec_count > (size_t)-1 / sizeof(Vector) ||
        vec_count > (size_t)-1 / sizeof(double) ||
        vec_count > (size_t)-1 / sizeof(size_t) ||
        norm_count > (size_t)-1 / sizeof(NormSpec) ||
        norm_count > (size_t)-1 / sizeof(NormResult))
        return STATUS_INVALID_COUNT;

    Vector *vectors = malloc(vec_count * sizeof(*vectors));
    NormSpec *norms = malloc(norm_count * sizeof(*norms));
    NormResult *res = calloc(norm_count, sizeof(*res));
    double *lengths = malloc(vec_count * sizeof(*lengths));

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

    for (size_t i = 0; i < vec_count && st == STATUS_OK; i++){
        if (vectors[i].coords == NULL)
            st = STATUS_NULL_ARGUMENT;
        else if (vectors[i].dim != n)
            st = STATUS_INVALID_DIMENSION;
    }
    for (size_t k = 0; k < norm_count && st == STATUS_OK; k++)
        if (norms[k].func == NULL)
            st = STATUS_NULL_ARGUMENT;

    for (size_t k = 0; k < norm_count && st == STATUS_OK; k++){
        double max = 0.0;

        for (size_t i = 0; i < vec_count && st == STATUS_OK; i++){
            st = norms[k].func(vectors[i].coords, n, norms[k].params, &lengths[i]);
            if (st == STATUS_OK && (i == 0 || lengths[i] > max))
                max = lengths[i];
        }
        if (st != STATUS_OK)
            break;

        res[k].indices = malloc(vec_count * sizeof(*res[k].indices));
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
        case STATUS_NULL_ARGUMENT:         printf("Внутренняя ошибка: нулевой указатель.\n"); break;
        case STATUS_INVALID_DIMENSION:     printf("Ошибка: некорректная или несовпадающая размерность.\n"); break;
        case STATUS_INVALID_EPS:           printf("Ошибка: эпсилон должен быть конечным и положительным.\n"); break;
        case STATUS_INVALID_COUNT:         printf("Ошибка: некорректное количество векторов или норм.\n"); break;
        case STATUS_INVALID_P:             printf("Ошибка: для p-нормы нужно конечное p >= 1.\n"); break;
        case STATUS_INVALID_NUMBER:        printf("Ошибка: координаты и матрица должны состоять из конечных чисел.\n"); break;
        case STATUS_NOT_POSITIVE_DEFINITE: printf("Ошибка: матрица не является симметричной положительно определённой.\n"); break;
        case STATUS_NUMERIC_OVERFLOW:      printf("Ошибка: переполнение при вычислении нормы.\n"); break;
        case STATUS_MEMORY_ERROR:          printf("Ошибка: не удалось выделить память.\n"); break;
        case STATUS_OK:                    break;
        default:                           printf("Ошибка.\n"); break;
    }
}

/* вывод отделён от вычислений */
Status print_results(const char *const *names, const NormResult *results, size_t norm_count){
    if (names == NULL || results == NULL)
        return STATUS_NULL_ARGUMENT;

    for (size_t k = 0; k < norm_count; k++){
        if (names[k] == NULL || results[k].indices == NULL)
            return STATUS_NULL_ARGUMENT;
        printf("%s: длина %.4f, векторы:", names[k], results[k].length);
        for (size_t i = 0; i < results[k].count; i++)
            printf(" v%zu", results[k].indices[i] + 1);
        printf("\n");
    }
    return STATUS_OK;
}

int main(void){
    const size_t n = 3;
    const double eps = 1e-9;

    const double c1[] = {1, 2, 2};
    const double c2[] = {3, 0, 0};
    const double c3[] = {0, 0, 1};
    const double c4[] = {-3, 0, 0};

    Vector v1 = {c1, 3}, v2 = {c2, 3}, v3 = {c3, 3}, v4 = {c4, 3};

    const double p1 = 1.0, p2 = 2.0;
    const double a[] = {2, 1, 0,
                        1, 2, 0,
                        0, 0, 3};
    Matrix m = {a, 3, eps};

    NormSpec s_inf = {norm_inf, NULL};
    NormSpec s_p1  = {norm_p, &p1};
    NormSpec s_p2  = {norm_p, &p2};
    NormSpec s_a   = {norm_a, &m};

    const char *names[] = {"inf", "p=1", "p=2", "A"};

    NormResult *res = NULL;
    Status st = longest_vectors(n, eps, 4, 4, &res, v1, v2, v3, v4, s_inf, s_p1, s_p2, s_a);
    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }
    st = print_results(names, res, 4);
    free_results(res, 4);
    if (st != STATUS_OK){
        print_status_error(st);
        return 1;
    }


    const double bad_p = 0.5;
    NormSpec s_bad_p = {norm_p, &bad_p};
    st = longest_vectors(n, eps, 2, 1, &res, v1, v2, s_bad_p);
    if (st != STATUS_OK) print_status_error(st); else free_results(res, 1);

    const double c_nan[] = {1, NAN, 0};
    Vector v_nan = {c_nan, 3};
    st = longest_vectors(n, eps, 2, 1, &res, v1, v_nan, s_inf);
    if (st != STATUS_OK) print_status_error(st); else free_results(res, 1);

    const double c_short[] = {1, 2};
    Vector v_short = {c_short, 2};
    st = longest_vectors(n, eps, 2, 1, &res, v1, v_short, s_inf);
    if (st != STATUS_OK) print_status_error(st); else free_results(res, 1);

    const double bad_a[] = {1, 2, 0,
                            2, 1, 0,
                            0, 0, 1};
    Matrix m_bad = {bad_a, 3, eps};
    NormSpec s_bad_a = {norm_a, &m_bad};
    st = longest_vectors(n, eps, 2, 1, &res, v1, v2, s_bad_a);
    if (st != STATUS_OK) print_status_error(st); else free_results(res, 1);

    Matrix m_small = {a, 2, eps};
    NormSpec s_small = {norm_a, &m_small};
    st = longest_vectors(n, eps, 2, 1, &res, v1, v2, s_small);
    if (st != STATUS_OK) print_status_error(st); else free_results(res, 1);

    NormSpec s_null = {NULL, NULL};
    st = longest_vectors(n, eps, 2, 1, &res, v1, v2, s_null);
    if (st != STATUS_OK) print_status_error(st); else free_results(res, 1);

    st = longest_vectors(n, NAN, 2, 1, &res, v1, v2, s_inf);
    if (st != STATUS_OK) print_status_error(st); else free_results(res, 1);

    st = longest_vectors(0, eps, 2, 1, &res, v1, v2, s_inf);
    if (st != STATUS_OK) print_status_error(st); else free_results(res, 1);

    st = longest_vectors(n, eps, 0, 1, &res);
    if (st != STATUS_OK) print_status_error(st); else free_results(res, 1);

    st = longest_vectors(n, eps, 1, 1, (NormResult **)NULL, v1, s_inf);
    if (st != STATUS_OK) print_status_error(st);

    const double c_big[] = {1e308, 1e308, 0};
    Vector v_big = {c_big, 3};
    st = longest_vectors(n, eps, 1, 1, &res, v_big, s_p2);        
    if (st != STATUS_OK) print_status_error(st); else free_results(res, 1);

    return 0;
}