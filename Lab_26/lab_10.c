#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#define MAX_INPUT_LENGTH 256
#define MAX_OUTPUT_LENGTH 70

typedef enum {
    STATUS_OK,
    STATUS_NULL_ARGUMENT,
    STATUS_EMPTY_STRING,
    STATUS_INVALID_BASE,
    STATUS_INVALID_NUMBER,
    STATUS_OVERFLOW,
    STATUS_INPUT_ERROR,
    STATUS_NO_NUMBERS
} Status;

/*
 * Удаляет символ '\n', который может остаться после fgets.
 */
Status remove_newline(char *str)
{
    if (str == NULL)
        return STATUS_NULL_ARGUMENT;

    size_t length = strlen(str);

    if (length == 0)
        return STATUS_EMPTY_STRING;

    if (str[length - 1] == '\n')
        str[length - 1] = '\0';

    return STATUS_OK;
}


/*
 * Проверяет строковое представление основания
 * и переводит его в int.
 */
Status parse_base(const char *str, int *base)
{
    if (str == NULL || base == NULL)
        return STATUS_NULL_ARGUMENT;

    if (*str == '\0')
        return STATUS_EMPTY_STRING;

    int value = 0;
    int i = 0;

    if (str[0] == '+')
        i++;

    if (str[i] == '\0')
        return STATUS_INVALID_BASE;

    while (str[i] != '\0') {

        if (str[i] < '0' || str[i] > '9')
            return STATUS_INVALID_BASE;

        int digit = str[i] - '0';

        if (value > (36 - digit) / 10)
            return STATUS_INVALID_BASE;

        value = value * 10 + digit;
        i++;
    }

    if (value < 2 || value > 36)
        return STATUS_INVALID_BASE;

    *base = value;

    return STATUS_OK;
}


/*
 * Переводит один символ в его числовое значение.
 *
 * 0..9 -> 0..9
 * A..Z -> 10..35
 */
Status get_digit_value(char symbol, int *digit)
{
    if (digit == NULL)
        return STATUS_NULL_ARGUMENT;

    if (symbol >= '0' && symbol <= '9') {
        *digit = symbol - '0';
        return STATUS_OK;
    }

    if (symbol >= 'A' && symbol <= 'Z') {
        *digit = symbol - 'A' + 10;
        return STATUS_OK;
    }

    return STATUS_INVALID_NUMBER;
}


/*
 * Переводит строковое число из системы base
 * в тип long long.
 *
 * Поддерживаются '+' и '-'.
 * Ввод цифр больше 9 разрешён только
 * прописными латинскими буквами.
 */
Status convert_to_decimal(
    const char *str,
    int base,
    long long *result
)
{
    if (str == NULL || result == NULL)
        return STATUS_NULL_ARGUMENT;

    if (base < 2 || base > 36)
        return STATUS_INVALID_BASE;

    if (*str == '\0')
        return STATUS_EMPTY_STRING;

    int i = 0;
    int negative = 0;

    if (str[i] == '-') {
        negative = 1;
        i++;
    }
    else if (str[i] == '+') {
        i++;
    }

    if (str[i] == '\0')
        return STATUS_INVALID_NUMBER;

    /*
     * Храним модуль числа в unsigned long long.
     * Это позволяет корректно обработать LLONG_MIN,
     * поскольку его модуль на 1 больше LLONG_MAX.
     */
    unsigned long long value = 0;

    while (str[i] != '\0') {

        int digit;
        Status status = get_digit_value(str[i], &digit);

        if (status != STATUS_OK)
            return STATUS_INVALID_NUMBER;

        if (digit >= base)
            return STATUS_INVALID_NUMBER;

        /*
         * Проверяем переполнение перед:
         *
         * value = value * base + digit
         */
        if (value > (ULLONG_MAX - (unsigned long long)digit)
                    / (unsigned long long)base)
            return STATUS_OVERFLOW;

        value = value * (unsigned long long)base
              + (unsigned long long)digit;

        i++;
    }

    /*
     * Для положительного числа максимум:
     * LLONG_MAX.
     *
     * Для отрицательного:
     * -(LLONG_MAX + 1) = LLONG_MIN.
     */
    if (!negative) {

        if (value > (unsigned long long)LLONG_MAX)
            return STATUS_OVERFLOW;

        *result = (long long)value;
    }
    else {

        if (value > (unsigned long long)LLONG_MAX + 1ULL)
            return STATUS_OVERFLOW;

        if (value == (unsigned long long)LLONG_MAX + 1ULL)
            *result = LLONG_MIN;
        else
            *result = -(long long)value;
    }

    return STATUS_OK;
}


/*
 * Безопасно складывает два long long.
 */
Status add_numbers(
    long long first,
    long long second,
    long long *result
)
{
    if (result == NULL)
        return STATUS_NULL_ARGUMENT;

    if (second > 0 && first > LLONG_MAX - second)
        return STATUS_OVERFLOW;

    if (second < 0 && first < LLONG_MIN - second)
        return STATUS_OVERFLOW;

    *result = first + second;

    return STATUS_OK;
}


/*
 * Возвращает модуль числа в unsigned long long.
 *
 * Это нужно, потому что для LLONG_MIN обычный
 * abs() использовать нельзя.
 */
Status absolute_value(
    long long number,
    unsigned long long *result
)
{
    if (result == NULL)
        return STATUS_NULL_ARGUMENT;

    if (number >= 0) {
        *result = (unsigned long long)number;
    }
    else {
        /*
         * Для LLONG_MIN:
         *
         * -(LLONG_MIN + 1) + 1
         *
         * вычисляется без переполнения.
         */
        *result = (unsigned long long)(-(number + 1)) + 1ULL;
    }

    return STATUS_OK;
}


/*
 * Проверяет, является ли candidate большим по модулю,
 * чем current_max.
 *
 * Возвращает:
 * 1 - candidate больше по модулю
 * 0 - иначе
 */
Status is_greater_by_abs(
    long long candidate,
    long long current_max,
    int *result
)
{
    if (result == NULL)
        return STATUS_NULL_ARGUMENT;

    unsigned long long candidate_abs;
    unsigned long long current_abs;

    Status status = absolute_value(candidate, &candidate_abs);

    if (status != STATUS_OK)
        return status;

    status = absolute_value(current_max, &current_abs);

    if (status != STATUS_OK)
        return status;

    *result = candidate_abs > current_abs;

    return STATUS_OK;
}


/*
 * Переводит число из десятичной системы
 * в систему с основанием base.
 *
 * Результат записывается в output.
 */
Status convert_from_decimal(
    long long number,
    int base,
    char *output,
    size_t output_size
)
{
    if (output == NULL)
        return STATUS_NULL_ARGUMENT;

    if (base < 2 || base > 36)
        return STATUS_INVALID_BASE;

    if (output_size == 0)
        return STATUS_NULL_ARGUMENT;

    const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    unsigned long long value;
    int negative = number < 0;

    Status status = absolute_value(number, &value);

    if (status != STATUS_OK)
        return status;

    char reversed[MAX_OUTPUT_LENGTH];
    size_t length = 0;

    /*
     * Ноль обрабатывается отдельно.
     */
    if (value == 0) {
        if (output_size < 2)
            return STATUS_OVERFLOW;

        output[0] = '0';
        output[1] = '\0';

        return STATUS_OK;
    }

    while (value > 0) {

        if (length >= sizeof(reversed))
            return STATUS_OVERFLOW;

        reversed[length++] = digits[value % (unsigned long long)base];

        value /= (unsigned long long)base;
    }

    if (negative)
        length++;

    if (length + 1 > output_size)
        return STATUS_OVERFLOW;

    size_t position = 0;

    if (negative) {
        output[position++] = '-';
    }

    while (length > (size_t)(negative ? 1 : 0)) {

        length--;
        output[position++] = reversed[length];
    }

    output[position] = '\0';

    return STATUS_OK;
}


/*
 * Выводит сообщение об ошибке.
 *
 * Функция занимается только выводом.
 */
void print_error(Status status)
{
    switch (status) {

        case STATUS_NULL_ARGUMENT:
            printf("Ошибка: передан пустой указатель.\n");
            break;

        case STATUS_EMPTY_STRING:
            printf("Ошибка: введена пустая строка.\n");
            break;

        case STATUS_INVALID_BASE:
            printf("Ошибка: основание системы счисления должно быть в диапазоне [2, 36].\n");
            break;

        case STATUS_INVALID_NUMBER:
            printf("Ошибка: некорректное число для заданной системы счисления.\n");
            break;

        case STATUS_OVERFLOW:
            printf("Ошибка: произошло переполнение.\n");
            break;

        case STATUS_INPUT_ERROR:
            printf("Ошибка: ошибка чтения ввода.\n");
            break;

        case STATUS_NO_NUMBERS:
            printf("Ошибка: не введено ни одного числа.\n");
            break;

        case STATUS_OK:
            break;

        default:
            printf("Ошибка исполнения программы.\n");
            break;
    }
}


int main(void){
    char input[MAX_INPUT_LENGTH];

    int base;

    long long sum = 0;
    long long max_number = 0;

    int number_count = 0;

    printf("Введите основание системы счисления [2..36]: ");

    if (fgets(input, sizeof(input), stdin) == NULL) {
        print_error(STATUS_INPUT_ERROR);
        return 1;
    }

    Status status = remove_newline(input);

    if (status != STATUS_OK) {
        print_error(status);
        return 1;
    }

    status = parse_base(input, &base);

    if (status != STATUS_OK) {
        print_error(status);
        return 1;
    }


    /*
     * ==========================
     * ВВОД ЧИСЕЛ
     * ==========================
     */

    printf("Введите числа в системе счисления %d.\n", base);
    printf("Для завершения введите Stop.\n");

    while (1) {

        printf("> ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            print_error(STATUS_INPUT_ERROR);
            return 1;
        }

        status = remove_newline(input);

        if (status != STATUS_OK) {
            print_error(status);
            return 1;
        }

        /*
         * Stop завершает ввод.
         */
        if (strcmp(input, "Stop") == 0)
            break;


        /*
         * ==========================
         * ОБРАБОТКА ЧИСЛА
         * ==========================
         */

        long long number;

        status = convert_to_decimal(input, base, &number);

        if (status != STATUS_OK) {
            print_error(status);
            return 1;
        }


        /*
         * Первое число становится
         * текущим максимумом.
         */
        if (number_count == 0) {
            max_number = number;
            sum = number;
            number_count++;
            continue;
        }


        /*
         * Проверяем новый максимум
         * по модулю.
         */
        int greater;

        status = is_greater_by_abs(
            number,
            max_number,
            &greater
        );

        if (status != STATUS_OK) {
            print_error(status);
            return 1;
        }

        if (greater)
            max_number = number;


        /*
         * Добавляем число к сумме.
         */
        long long new_sum;

        status = add_numbers(
            sum,
            number,
            &new_sum
        );

        if (status != STATUS_OK) {
            print_error(status);
            return 1;
        }

        sum = new_sum;

        number_count++;
    }


    /*
     * ==========================
     * ПРОВЕРКА НАЛИЧИЯ ЧИСЕЛ
     * ==========================
     */

    if (number_count == 0) {
        print_error(STATUS_NO_NUMBERS);
        return 1;
    }


    /*
     * ==========================
     * ПОДГОТОВКА РЕЗУЛЬТАТОВ
     * ==========================
     */

    char max_base9[MAX_OUTPUT_LENGTH];
    char max_base18[MAX_OUTPUT_LENGTH];
    char max_base27[MAX_OUTPUT_LENGTH];
    char max_base36[MAX_OUTPUT_LENGTH];

    char sum_base9[MAX_OUTPUT_LENGTH];
    char sum_base18[MAX_OUTPUT_LENGTH];
    char sum_base27[MAX_OUTPUT_LENGTH];
    char sum_base36[MAX_OUTPUT_LENGTH];


    status = convert_from_decimal(
        max_number,
        9,
        max_base9,
        sizeof(max_base9)
    );

    if (status != STATUS_OK) {
        print_error(status);
        return 1;
    }

    status = convert_from_decimal(
        max_number,
        18,
        max_base18,
        sizeof(max_base18)
    );

    if (status != STATUS_OK) {
        print_error(status);
        return 1;
    }

    status = convert_from_decimal(
        max_number,
        27,
        max_base27,
        sizeof(max_base27)
    );

    if (status != STATUS_OK) {
        print_error(status);
        return 1;
    }

    status = convert_from_decimal(
        max_number,
        36,
        max_base36,
        sizeof(max_base36)
    );

    if (status != STATUS_OK) {
        print_error(status);
        return 1;
    }


    status = convert_from_decimal(
        sum,
        9,
        sum_base9,
        sizeof(sum_base9)
    );

    if (status != STATUS_OK) {
        print_error(status);
        return 1;
    }

    status = convert_from_decimal(
        sum,
        18,
        sum_base18,
        sizeof(sum_base18)
    );

    if (status != STATUS_OK) {
        print_error(status);
        return 1;
    }

    status = convert_from_decimal(
        sum,
        27,
        sum_base27,
        sizeof(sum_base27)
    );

    if (status != STATUS_OK) {
        print_error(status);
        return 1;
    }

    status = convert_from_decimal(
        sum,
        36,
        sum_base36,
        sizeof(sum_base36)
    );

    if (status != STATUS_OK) {
        print_error(status);
        return 1;
    }


    /*
     * ==========================
     * ВЫВОД
     * ==========================
     */

    printf("\nМаксимальное по модулю число:\n");
    printf("10: %lld\n", max_number);
    printf("9:  %s\n", max_base9);
    printf("18: %s\n", max_base18);
    printf("27: %s\n", max_base27);
    printf("36: %s\n", max_base36);

    printf("\nСумма всех чисел:\n");
    printf("10: %lld\n", sum);
    printf("9:  %s\n", sum_base9);
    printf("18: %s\n", sum_base18);
    printf("27: %s\n", sum_base27);
    printf("36: %s\n", sum_base36);

    return 0;
}