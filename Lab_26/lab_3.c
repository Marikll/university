#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>

typedef enum{
    STATUS_NULL_ARGUMENT,
    STATUS_INVALID_NUMBERS,
    STATUS_OK,
    STATUS_NO_SOLUTION,
    STATUS_ANY_SOLUTION,
    STATUS_NO_MULTIPLES,
    STATUS_EMPTY_STRING,
    STATUS_INVALID_FLAG,
    STATUS_FLAG_Q,
    STATUS_FLAG_M,
    STATUS_FLAG_T,
    STATUS_NO_DIGITS,
    STATUS_OVERFLOW,
    STATUS_INVALID_NUMBER,
    STATUS_SIGNED_NUMBER,
    STATUS_TWO_ANSWER
} Status;

Status flag_q(const double eps, double *a, double *b, double *c, double res[], int *cur){
    if (a == NULL || b == NULL || c == NULL)
        return STATUS_NULL_ARGUMENT;

    if (eps <= 0.0) return STATUS_INVALID_NUMBERS;

    double a1 = *a, b1 = *b, c1 = *c;
    if (fabs(a1) <= eps && fabs(b1) <= eps && fabs(c1) <= eps)
        return STATUS_ANY_SOLUTION;

    double variants[6][3] = { 
        {a1, b1, c1}, 
        {a1, c1, b1}, 
        {b1, a1, c1}, 
        {b1, c1, a1}, 
        {c1, a1, b1}, 
        {c1, b1, a1} 
    };

    int two_answer = 0;
    int var_count = 0;
    *cur = 0;
    double add_var;
    for (int k = 0; k < 6; k++){
        int double_var = 0;
        for (int i = 0; i < var_count; i++){
            if ( fabs(variants[i][0] - variants[k][0]) < eps &&  fabs(variants[i][1] - variants[k][1]) < eps && fabs(variants[i][2] - variants[k][2]) < eps){
                double_var = 1;
                break;
            }
        }
        if (double_var)
            continue;
        variants[var_count][0] = variants[k][0];
        variants[var_count][1] = variants[k][1];
        variants[var_count][2] = variants[k][2];
        var_count++;

        a1 = variants[k][0];
        b1 = variants[k][1];
        c1 = variants[k][2];

        if ((fabs(a1) <= eps && fabs(c1) <= eps) || (fabs(b1) <= eps && fabs(c1) <= eps)){
            res[*cur] = 0.0;
            (*cur)++; /* в случае, когда среди коэф. два нуля: решение только 0 или нет решений вовсе*/
        } else if (fabs(a1) <= eps && fabs(b1) <= eps){
            two_answer++;
            continue;
        } else if (fabs(a1) <= eps){
            res[*cur] = (-c1/b1);
            (*cur)++;
        } else{
            double d = b1 * b1 - 4 * a1 * c1;
            if (d > eps){
                res[*cur] = (-b1 + sqrt(d))/(2 * a1);
                (*cur)++;
                res[*cur] = (-b1 - sqrt(d))/(2 * a1);
                (*cur)++;
            } else if (fabs(d) <= eps){
                res[*cur] = -b1/(2 * a1);
                (*cur)++;
            }
        }
    }
    

    if (*cur>0 && !two_answer)
        return STATUS_OK;
    else if (*cur>0)
        return STATUS_TWO_ANSWER;
    else   
        return STATUS_NO_SOLUTION;
}

Status flag_m(long long *x1, long long *x2){
    if (x1 == NULL || x2 == NULL)
        return STATUS_NULL_ARGUMENT;
    long long x = *x1, y = *x2;
    if (x == 0 || y == 0)
        return STATUS_INVALID_NUMBERS;
    if (x%y == 0)
        return STATUS_OK;
    else
        return STATUS_NO_MULTIPLES;
}

Status flag_t(const double eps, double *x1, double *x2, double *x3){
    if (x1 == NULL || x2 == NULL || x3 == NULL)
        return STATUS_NULL_ARGUMENT;
    double x = *x1, y = *x2, z = *x3;
    if (fabs(x) <= eps || fabs(y) <= eps || fabs(z) <= eps || x < -eps || y < -eps || z < -eps)
        return STATUS_INVALID_NUMBERS;
    if (fabs((x*x + y*y) - z*z) <= eps || fabs((y*y + z*z) - x*x ) <= eps || fabs((z*z + x*x) - y*y) <= eps)
        return STATUS_OK;
    else
        return STATUS_NO_SOLUTION;
}

Status check_flag(const char * str){
    if (str == NULL)
        return STATUS_NULL_ARGUMENT;
    if (str[0] != '-' && str[0] != '/')
        return STATUS_INVALID_FLAG;
    if (str[2] != '\0')
        return STATUS_INVALID_FLAG;
    if (str[1] == '\0')
        return STATUS_INVALID_FLAG;
    if (str[0] == '\0')
        return STATUS_EMPTY_STRING;
    switch (str[1]){
        case 'q':
            return STATUS_FLAG_Q;
        case 'm':
            return STATUS_FLAG_M;
        case 't':
            return STATUS_FLAG_T;
        default:
            return STATUS_INVALID_FLAG;
    }
}

Status check_number(const char * str, long long * num){
    if (str == NULL || num == NULL)
        return STATUS_NULL_ARGUMENT;
    if (str[0] == '\0')
        return STATUS_EMPTY_STRING;
    int i = 0;
    int sign = 1;

    if (str[i] == '+' || str[i] == '-'){
        if (str[i] == '-')
            sign = -1;
        i++;
        if (str[i] == '\0')
            return STATUS_NO_DIGITS;
    }

    long long value = 0;
    long long lim_ll;
    if (sign == -1){
        lim_ll = 1ULL << (sizeof(long long)*8 - 1);
    } else {
        lim_ll = (1ULL << (sizeof(long long))*8 - 1) - 1;
    }
    
    for (; str[i] != '\0'; i++){
        if (str[i] == '.' || str[i] == ',')
            return STATUS_INVALID_NUMBERS;
        if (str[i] < '0' || str[i] > '9')
            return STATUS_NO_DIGITS;

        int digit = str[i] - '0';

        if (value > (lim_ll - digit) / 10)
            return STATUS_OVERFLOW;
        
        value = value*10 + digit;
    }
    if (sign == -1)
        *num = -value;
    else
        *num = value;
    return STATUS_OK;
}

Status conversion_num(const char *str, double *num, int flag_eps){ 
    if (str == NULL || num == NULL) 
        return STATUS_NULL_ARGUMENT; 

    if (str[0] == '\0') 
        return STATUS_EMPTY_STRING; 
    
    int i = 0;
    int sign = 1; 
    
    if (str[i] == '-' || str[i] == '+'){ 
        if (str[i] == '-') 
            sign = -1; i++; 
        if (str[i] == '\0') 
            return STATUS_NO_DIGITS; 
    } 
    if (str[i] < '0' || str[i] > '9')
        return STATUS_INVALID_NUMBER;
    
    double value = 0.0; 
    double fraction = 0.0; 
    double divider = 10.0; 
    int point = 0; 
    int digits = 0; 
    
    for (; str[i] != '\0' && str[i] != 'e' && str[i] != 'E'; i++){ 
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

    if ((str[i] == 'e' || str[i] == 'E') && flag_eps == 0)
            return STATUS_INVALID_NUMBERS;
    else if (flag_eps == 1){
        if (str[i] == 'e' || str[i] == 'E'){ 
            i++; 
            int exponent_sign = 1; 
            if (str[i] == '+' || str[i] == '-'){ 
                if (str[i] == '-') 
                    exponent_sign = -1; 
                i++; 
            } 
            if (str[i] == '\0') 
                return STATUS_NO_DIGITS; 
        
            int exponent = 0; 
            int exponent_digits = 0; 
            while (str[i] != '\0'){ 
                if (str[i] < '0' || str[i] > '9') 
                    return STATUS_INVALID_NUMBER; 
            
                int digit = str[i] - '0'; 
                exponent_digits++;  
            
                if (exponent > (INT_MAX - digit) / 10) 
                    return STATUS_OVERFLOW; 
                
                exponent = exponent * 10 + digit; 
                i++; 
            } 
        
            if (exponent_digits == 0) 
                return STATUS_NO_DIGITS; 
        
            exponent *= exponent_sign; 
            double multiplier = pow(10.0, exponent); 
        
            if (!isfinite(multiplier)) 
                return STATUS_OVERFLOW; 
            
            value *= multiplier; 
        
            if (!isfinite(value)) 
                return STATUS_OVERFLOW; 
        } 

        if (sign == -1)
            return STATUS_SIGNED_NUMBER;
    } else{
        if (sign == -1) 
            value = -value; 
    }

    *num = value; 
    return STATUS_OK; 
}

int main(int argc, char *argv[]){
    if (argc < 2 || argc > 6){
        printf("Введите флаг и соответствующее ему количество чисел.\n");
        return 1;
    }
    
    Status st_flag = check_flag(argv[1]);
    switch(st_flag){
        case STATUS_EMPTY_STRING:
            printf("Ошибка: передана пустая строка. Введите флаг и соответствующее ему количество чисел.\n");
            return 1;
        
        case STATUS_NULL_ARGUMENT:
            printf("Внутренняя ошибка: введите флаг и соответствующее ему количество чисел еще раз.\n");
            return 1;
        
        case STATUS_INVALID_FLAG:
            printf("Ошибка: введен некорректный флаг. Допустимые флаги: -q, -m, -p.\n");
            return 1;

        case STATUS_FLAG_Q: {
            if (argc != 6){
                printf("Ошибка: для флага -q необходимо ввести эпсилон и три вещественных числа.\n");
                return 1;
            }
            double eps = 0.0;
            int flag_eps = 1;
            Status st_eps = conversion_num(argv[2], &eps, flag_eps);

            switch(st_eps){
                case STATUS_NULL_ARGUMENT:
                    printf("Внутренняя ошибка: введите флаг и соответствующее ему количество чисел еще раз.\n");
                    return 1;
                
                case STATUS_EMPTY_STRING:
                    printf("Ошибка: передана пустая строка. Введите флаг и соответствующее ему количество чисел.\n");
                    return 1;

                case STATUS_NO_DIGITS:
                    printf("Ошибка: введено некорректное эпсилон.\n");
                    return 1;
                
                case STATUS_INVALID_NUMBERS:
                    printf("Ошибка: введено некорректное эпсилон.\n");
                    return 1;

                case STATUS_OVERFLOW:
                    printf("Ошибка: переполнение. Введите эпсилон поменьше.\n");
                    return 1;

                case STATUS_OK:
                    break;
                
                default:
                    printf("Ошибка: неизвестный статус.\n");
                    return 1;
            }

            if (eps <= 0){
                printf("Введите положительный эпсилон.\n");
                return 1;
            }

            double a = 0.0;
            double b = 0.0;
            double c = 0.0;
            flag_eps = 0;

            Status st_a = conversion_num(argv[3], &a, flag_eps);
            Status st_b = conversion_num(argv[4], &b, flag_eps);
            Status st_c = conversion_num(argv[5], &c, flag_eps);

            if (st_a == STATUS_NULL_ARGUMENT || st_b == STATUS_NULL_ARGUMENT || st_c == STATUS_NULL_ARGUMENT){
                printf("Внутренняя ошибка: введите флаг и соответствующее ему количество чисел еще раз.\n");
                return 1;
            }
            if (st_a == STATUS_EMPTY_STRING || st_b == STATUS_EMPTY_STRING || st_c == STATUS_EMPTY_STRING){
                printf("Ошибка: передана пустая строка. Введите флаг и соответствующее ему количество чисел.\n");
                return 1;
            }
            if (st_a == STATUS_NO_DIGITS || st_b == STATUS_NO_DIGITS || st_c == STATUS_NO_DIGITS){
                printf("Ошибка: введены некорректные коэффициенты.\n");
                    return 1;
            }
            if (st_a == STATUS_INVALID_NUMBERS || st_b == STATUS_INVALID_NUMBERS || st_c == STATUS_INVALID_NUMBERS){
                printf("Ошибка: введены некорректные коэффициенты.\n");
                return 1;
            }
            if (st_a == STATUS_OVERFLOW || st_b == STATUS_OVERFLOW || st_c == STATUS_OVERFLOW){
                printf("Ошибка: переполнение. Введите коэффициенты поменьше.\n");
                return 1;
            }
            if (st_a == STATUS_OK && st_b == STATUS_OK && st_c == STATUS_OK){
              
                double results[12];
                int cur;
                Status st_q = flag_q(eps, &a, &b, &c, results, &cur);
            
                if (st_q == STATUS_NULL_ARGUMENT){
                    printf("Внутренняя ошибка: введите флаг и соответствующее ему количество чисел еще раз.\n");
                    return 1;
                }
                if (st_q == STATUS_ANY_SOLUTION){
                    printf("Уравнение имеет бесконечное количество решений.\n");
                    return 1;
                }
                if (st_q == STATUS_NO_SOLUTION){
                    printf("Уравнение не имеет решений.\n");
                    return 1;
                }
                if (st_q == STATUS_OK){
                    printf("Корни уравнения: ");
                    for (int i = 0; i < cur; i++)
                        printf("%.5g ", results[i]);
                    printf("\n");
                }
                if (st_q == STATUS_TWO_ANSWER){
                    printf("Корни уравнения: ");
                    for (int i = 0; i < cur; i++)
                        printf("%.5g ", results[i]);
                    printf("\n");
                    printf("Уравнение вида: 0x^2 + 0x + c = 0, не имеет решений.\n");
                }
                break;
            }
        }
        
        case STATUS_FLAG_M: {
            if (argc != 4){
                printf("Ошибка: для флага -m необходимо ввести два целых числа, не равных нулю.\n");
                return 1;
            }

            long long x1 = 1;
            long long x2 = 1;

            Status st_x1 = check_number(argv[2], &x1);
            Status st_x2 = check_number(argv[3], &x2);

            if (st_x1 == STATUS_NULL_ARGUMENT || st_x2 == STATUS_NULL_ARGUMENT){
                printf("Внутренняя ошибка: введите флаг и соответствующее ему количество чисел еще раз.\n");
                return 1;
            }
            if (st_x1 == STATUS_EMPTY_STRING || st_x2 == STATUS_EMPTY_STRING){
                printf("Ошибка: передана пустая строка. Введите флаг и соответствующее ему количество чисел.\n");
                return 1;
            }
            if (st_x1 == STATUS_NO_DIGITS || st_x2 == STATUS_NO_DIGITS){
                printf("Ошибка: введены некорректные числа.\n");
                    return 1;
            }
            if (st_x1 == STATUS_INVALID_NUMBERS || st_x2 == STATUS_INVALID_NUMBERS){
                printf("Ошибка: введены некорректные числа.\n");
                return 1;
            }
            if (st_x1 == STATUS_OVERFLOW || st_x2 == STATUS_OVERFLOW){
                printf("Ошибка: переполнение. Введите числа поменьше.\n");
                return 1;
            } 
            if (st_x1 == STATUS_OK && st_x2 == STATUS_OK){

                Status st_m = flag_m(&x1, &x2);

                if (st_m == STATUS_NULL_ARGUMENT){
                    printf("Внутренняя ошибка: введите флаг и соответствующее ему количество чисел еще раз.\n");
                    return 1;
                }
                if (st_m == STATUS_INVALID_NUMBERS){
                    printf("Неподходящие числа: для флага -m подходят только целые числа неравные нулю.\n");
                    return 0;
                }
                if (st_m == STATUS_NO_MULTIPLES){
                    printf("Число %lld некратно %lld.\n", x1, x2);
                    return 0;
                }
                if (st_m == STATUS_OK){
                    printf("Число %lld кратно %lld.\n", x1, x2);
                    return 0;
                }
            }
        }

        case STATUS_FLAG_T: {
            if (argc != 6){
                printf("Ошибка: для флага -t необходимо ввести эпсилон и три положительных числа.\n");
                return 1;
            }

            double eps = 0.0;
            int flag_eps = 1;
            Status st_eps = conversion_num(argv[2], &eps, flag_eps);

            switch(st_eps){
                case STATUS_NULL_ARGUMENT:
                    printf("Внутренняя ошибка: введите флаг и соответствующее ему количество чисел еще раз.\n");
                    return 1;
                
                case STATUS_EMPTY_STRING:
                    printf("Ошибка: передана пустая строка. Введите флаг и соответствующее ему количество чисел.\n");
                    return 1;

                case STATUS_NO_DIGITS:
                    printf("Ошибка: введено некорректное эпсилон.\n");
                    return 1;
                
                case STATUS_INVALID_NUMBERS:
                    printf("Ошибка: введено некорректное эпсилон.\n");
                    return 1;

                case STATUS_OVERFLOW:
                    printf("Ошибка: переполнение. Введите эпсилон поменьше.\n");
                    return 1;

                case STATUS_OK:
                    break;
                
                default:
                    printf("Ошибка: неизвестный статус.\n");
                    return 1;
            }

            if (eps <= 0){
                printf("Введите положительный эпсилон.\n");
                return 1;
            }

            double x = 0.0;
            double y = 0.0;
            double z = 0.0;
            flag_eps = 0;

            Status st_x = conversion_num(argv[3], &x, flag_eps);
            Status st_y = conversion_num(argv[4], &y, flag_eps);
            Status st_z = conversion_num(argv[5], &z, flag_eps);

            if (st_x == STATUS_NULL_ARGUMENT || st_y == STATUS_NULL_ARGUMENT || st_z == STATUS_NULL_ARGUMENT){
                printf("Внутренняя ошибка: введите флаг и соответствующее ему количество чисел еще раз.\n");
                return 1;
            }
            if (st_x == STATUS_EMPTY_STRING || st_y == STATUS_EMPTY_STRING || st_z == STATUS_EMPTY_STRING){
                printf("Ошибка: передана пустая строка. Введите флаг и соответствующее ему количество чисел.\n");
                return 1;
            }
            if (st_x == STATUS_NO_DIGITS || st_y == STATUS_NO_DIGITS || st_z == STATUS_NO_DIGITS){
                printf("Ошибка: введены некорректные числа.\n");
                    return 1;
            }
            if (st_x == STATUS_INVALID_NUMBERS || st_y == STATUS_INVALID_NUMBERS || st_z == STATUS_INVALID_NUMBERS){
                printf("Ошибка: введены некорректные числа.\n");
                return 1;
            }
            if (st_x == STATUS_OVERFLOW || st_y == STATUS_OVERFLOW || st_z == STATUS_OVERFLOW){
                printf("Ошибка: переполнение. Введите числа поменьше.\n");
                return 1;
            }
            if (st_x == STATUS_OK && st_y == STATUS_OK && st_z == STATUS_OK){

                Status st_t = flag_t(eps, &x, &y, &z);

                if (st_t == STATUS_NULL_ARGUMENT){
                    printf("Внутренняя ошибка: введите флаг и соответствующее ему количество чисел еще раз.\n");
                    return 1;
                }
                if (st_t == STATUS_INVALID_NUMBERS){
                    printf("Неподходящие числа: для флага -t подходят только положительные числа.\n");
                    return 0;
                }
                if (st_t == STATUS_NO_SOLUTION){
                    printf("Стороны треугольника не могут быть равны: %.5g, %.5g, %.5g.\n", x, y, z);
                    return 0;
                }
                if (st_t == STATUS_OK){
                    printf("Стороны треугольника могут быть равны: %.5g, %.5g, %.5g.\n", x, y, z);
                    return 0;
                }
            }
        }
    }
}
