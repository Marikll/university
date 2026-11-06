#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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
    STATUS_SIGN_ERROR,
    STATUS_NO_DIGITS,
    STATUS_INVALID_NUMBER,
    STATUS_INVALID_HOUR,
    STATUS_INVALID_MINUTS,
    STATUS_INVALID_TIME,
    STATUS_INVALID_ID,
    STATUS_DUPLICATE_ID
} Status;

int compare_id(const void a, const void b){
    const Student *id_a = (const Student *)a;
    const Student *id_b = (const Student *)b;

    if (id_a->id < id_b->id)
        return -1;
    if (id_a->id > id_b->id)
        return 1;
    return 0;
}

int compare_name(const void a, const void b){
    const Student *name_a = (const Student *)a;
    const Student *name_b = (const Student *)b;

    if (strcmp(name_a->name, name_b->name) < 0)
        return -1;
    if (strcmp(name_a->name, name_b->name) > 0)
        return 1;
    return 0;
}

int compare_lastname(const void a, const void b){
    const Student *lastname_a = (const Student *)a;
    const Student *lastname_b = (const Student *)b;

    if (strcmp(lastname_a->lastname, lastname_b->lastname) < 0)
        return -1;
    if (strcmp(lastname_a->lastname, lastname_b->lastname) > 0)
        return 1;
    return 0;
}

int compare_group(const void a, const void b){
    const Student *group_a = (const Student *)a;
    const Student *group_b = (const Student *)b;

    if (strcmp(group_a->group, group_b->group) < 0)
        return -1;
    if (strcmp(group_a->group, group_b->group) > 0)
        return 1;
    return 0;
}

Status search_id(Student *students, size_t count_students, unsigned long long st_id, Student *st){
    if (students == NULL || st == NULL)
        return STATUS_NULL_ARGUMENT;

    Student st_res;

    size_t bin = (count_students - 1)/2;
    size_t l = count_students - 1;
    bool st_l;
    if (st_l = (students[l].id ==  st_id))
        st_res = students[l];
    size_t r = 0;
    bool st_r;
    if (st_r = (students[r].id == st_id))
        st_res = students[r];
    while(bin != 0 && !st_r && !st_l){
        if (students[bin].id < st_id)
            l = bin;
        else if (students[bin].id > st_id)
            r = bin;
        else{
            st_res = students[bin];
            break;
        }
        bin = (l - r)/2;
    }

    if (bin != 0){
        *st = st_res;
        return STATUS_OK;
    } else {
        return STATUS_NO_SEARCH;
    }
}

Status search_lastname(Student *students, size_t count_students, char *st_lastname, Student *st){
    if (students == NULL || st == NULL)
        return STATUS_NULL_ARGUMENT;

    Student st_res;

    size_t bin = (count_students - 1)/2;
    size_t l = count_students - 1;
    int st_l;
    if (!(st_l = strcmp(students[l].lastname, st_lastname)))
        st_res = students[l];
    size_t r = 0;
    bool st_r;
    if (!(st_r = strcmp(students[r].lastname, st_lastname)))
        st_res = students[r];
    while(bin != 0 && !st_r && !st_l){
        if (strcmp(students[bin].lastname, st_lastname) < 0)
            l = bin;
        else if (strcmp(students[bin].lastname > st_lastname) > 0)
            r = bin;
        else{
            st_res = students[bin];
            break;
        }
        bin = (l - r)/2;
    }

    if (bin != 0){
        *st = st_res;
        return STATUS_OK;
    } else {
        return STATUS_NO_SEARCH;
    }
}

Status search_name(Student *students, size_t count_students, char *st_name, Student *st){
    if (students == NULL || st == NULL)
        return STATUS_NULL_ARGUMENT;

    Student st_res;

    size_t bin = (count_students - 1)/2;
    size_t l = count_students - 1;
    bool st_l;
    if (!(st_l = strcmp(students[l].name ==  st_name)))
        st_res = students[l];
    size_t r = 0;
    bool st_r;
    if (!(st_r = strcmp(students[r].name == st_name)))
        st_res = students[r];
    while(bin != 0 && !st_r && !st_l){
        if (strcmp(students[bin].name, st_name) < 0)
            l = bin;
        else if (strcmp(students[bin].name, st_name) > 0)
            r = bin;
        else{
            st_res = students[bin];
            break;
        }
        bin = (l - r)/2;
    }

    if (bin != 0){
        *st = st_res;
        return STATUS_OK;
    } else {
        return STATUS_NO_SEARCH;
    }
}

Status search_group(Student *students, size_t count_students, char *st_group, Student *st){
    if (students == NULL || st == NULL)
        return STATUS_NULL_ARGUMENT;

    Student st_res;

    size_t bin = (count_students - 1)/2;
    size_t l = count_students - 1;
    bool st_l;
    if (!(st_l = strcmp(students[l].group, st_group)))
        st_res = students[l];
    size_t r = 0;
    bool st_r;
    if (!(st_r = strcmp(students[r].group, st_group)))
        st_res = students[r];
    while(bin != 0 && !st_r && !st_l){
        if (strcmp(students[bin].group, st_group) < 0)
            l = bin;
        else if (strcmp(students[bin].group, st_group) > 0)
            r = bin;
        else{
            st_res = students[bin];
            break;
        }
        bin = (l - r)/2;
    }

    if (bin != 0){
        *st = st_res;
        return STATUS_OK;
    } else {
        return STATUS_NO_SEARCH;
    }
}

Status search_score(Student *students, size_t count_st, unsigned int ave_score, Student **res){
    if (students == NULL || res == NULL)
        return STATUS_NULL_ARGUMENT;

    Student *result = malloc(count_st * sizeof(*result));
    if (result == NULL)
        return STATUS_NULL_ARGUMENT;
    size_t c_st = 0;
    for (size_t i = 0; i < count_st; i++){
        unsigned int score = 0;
        for (size_t j = 0; j < 5; j++){
            score += students[i].scores[j];
        } 
        if (score > ave_score){
            result[c_st] = students[i];
            c_st++;
        }
    }

    if (c_st != 0){
        *res = result;
        return STATUS_OK;
    } else  
        return STATUS_NO_SEARCH;
}

Status write_array(FILE *output, const Student *array, size_t count){
    if (output == NULL || array == NULL)
        return STATUS_NULL_ARGUMENT;

    for (size_t i = 0; i < count; i++) {
        if (fprintf(output, "%llu %s %s %s %.2f\n", array[i].id, array[i].name, array[i].lastname, array[i]. array[i].zp) < 0)
            return STATUS_FILE_ERROR;
    }
    return STATUS_OK;
}

Status check_flag(const char *str){
    if (str == NULL)
        return STATUS_NULL_ARGUMENT;
    if (str[0] == '\0')
        return STATUS_EMPTY_STRING;
    if ((str[0] != '-' && str[0] != '/') || str[1] == '\0' || str[2] != '\0')
        return STATUS_INVALID_FLAG;

    switch (str[1]) {
        case 'a': return STATUS_FLAG_A;
        case 'd': return STATUS_FLAG_D;
        default:  return STATUS_INVALID_FLAG;
    }
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

int main(void){

}
