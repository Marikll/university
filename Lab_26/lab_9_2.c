
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>

typedef enum {
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_ARGUMENT,
    STATUS_MEMORY_ERROR
} Status;


/* Заполняет массив случайными числами из диапазона [-1000..1000] */
Status fill_random_array(int *array, size_t size) {
    if (array == NULL)
        return STATUS_NULL_ARGUMENT;

    if (size == 0)
        return STATUS_INVALID_ARGUMENT;

    for (size_t i = 0; i < size; i++)
        array[i] = rand() % 2001 - 1000;

    return STATUS_OK;
}


/* Создаёт динамический массив случайного размера */
Status create_random_array(int **array, size_t *size) {
    if (array == NULL || size == NULL)
        return STATUS_NULL_ARGUMENT;

    *size = (size_t)(rand() % 9991 + 10);

    *array = malloc(*size * sizeof(**array));

    if (*array == NULL)
        return STATUS_MEMORY_ERROR;

    Status status = fill_random_array(*array, *size);

    if (status != STATUS_OK) {
        free(*array);
        *array = NULL;
        return status;
    }

    return STATUS_OK;
}


/* Находит ближайший по значению элемент массива B */
Status find_nearest(const int *array, size_t size,
                    int value, int *nearest) {
    if (array == NULL || nearest == NULL)
        return STATUS_NULL_ARGUMENT;

    if (size == 0)
        return STATUS_INVALID_ARGUMENT;

    size_t nearest_index = 0;
    long long min_difference =
        llabs((long long)array[0] - value);

    for (size_t i = 1; i < size; i++) {
        long long difference =
            llabs((long long)array[i] - value);

        if (difference < min_difference) {
            min_difference = difference;
            nearest_index = i;
        }
    }

    *nearest = array[nearest_index];

    return STATUS_OK;
}


/* Формирует массив C */
Status create_array_c(const int *a, size_t size_a,
                      const int *b, size_t size_b,
                      int **c) {
    if (a == NULL || b == NULL || c == NULL)
        return STATUS_NULL_ARGUMENT;

    if (size_a == 0 || size_b == 0)
        return STATUS_INVALID_ARGUMENT;

    *c = malloc(size_a * sizeof(**c));

    if (*c == NULL)
        return STATUS_MEMORY_ERROR;

    for (size_t i = 0; i < size_a; i++) {
        int nearest;
        Status status = find_nearest(b, size_b, a[i], &nearest);

        if (status != STATUS_OK) {
            free(*c);
            *c = NULL;
            return status;
        }

        (*c)[i] = a[i] + nearest;
    }

    return STATUS_OK;
}


/* Выводит массив */
Status print_array(const int *array, size_t size) {
    if (array == NULL)
        return STATUS_NULL_ARGUMENT;

    for (size_t i = 0; i < size; i++)
        printf("%d ", array[i]);

    printf("\n");

    return STATUS_OK;
}

int main(void) {
    srand((unsigned int)time(NULL));

    int *a = NULL;
    int *b = NULL;
    int *c = NULL;

    size_t size_a = 0;
    size_t size_b = 0;

    Status status = create_random_array(&a, &size_a);

    if (status != STATUS_OK) {
        printf("Ошибка создания массива A.\n");
        return 1;
    }

    status = create_random_array(&b, &size_b);

    if (status != STATUS_OK) {
        printf("Ошибка создания массива B.\n");
        free(a);
        return 1;
    }

    status = create_array_c(a, size_a, b, size_b, &c);

    if (status != STATUS_OK) {
        printf("Ошибка создания массива C.\n");
        free(a);
        free(b);
        return 1;
    }

    printf("Размер A: %zu\n", size_a);
    printf("Размер B: %zu\n", size_b);

    printf("Массив A:\n");
    print_array(a, size_a);

    printf("Массив B:\n");
    print_array(b, size_b);

    printf("Массив C:\n");
    print_array(c, size_a);

    free(a);
    free(b);
    free(c);

    return 0;
}