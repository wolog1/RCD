#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define INITIAL_CAPACITY 4



/*
 * 一维vector
 */
typedef struct {
    double *data;          // 指向动态数组的指针
    size_t size;        // 向量中元素的数量
    size_t capacity;    // 向量中可容纳元素的数量
} vector;

void vector_init(vector *v) {
    v->data = malloc(sizeof(double ) * INITIAL_CAPACITY);
    v->size = 0;
    v->capacity = INITIAL_CAPACITY;
}

void vector_resize(vector *v, int n){
    v->data = malloc(sizeof(double ) * n);
    v->size = n;
    v->capacity = n;
}


void vector_append(vector *v, double value) {
    if (v->size >= v->capacity) {
        // 当数组已满时，增加容量
        v->capacity *= 2;
        double *p = realloc(v->data, sizeof(double) * v->capacity);
        if(p==NULL){
            printf("vector1 append error\n");
            exit(1);
        }
        v->data = p;
    }
    // 向向量末尾添加新元素
    v->data[v->size] = value;
    v->size++;
}

double vector_get(vector *v, size_t index) {
    if (index >= v->size) {
        printf("Error: Index %zu out of range\n", index);
        exit(1);
    }
    return v->data[index];
}

void vector_set(vector *v, size_t index, double value) {
    if (index >= v->size) {
        printf("Error: Index %zu out of range\n", index);
        exit(1);
    }
    v->data[index] = value;
}

void vector_print(vector* v){
    for(size_t i=0;i<v->size;i++){
        printf("%g,",v->data[i]);
    }
    printf("%ld\n",v->size);
}

void vector_free(vector *v) {
    v->size = 0;
    v->capacity = 0;
    free(v->data);
    v->data = NULL;
}

/*
 * 二维数组
 */

typedef struct {
    double **data;
    size_t rows;
    size_t cols;
} vector2d;

void vector2d_init(vector2d *v, size_t rows, size_t cols) {
    v->rows = rows;
    v->cols = cols;
    v->data = NULL;
    if(rows>0){
        // 分配内存空间
        v->data = malloc(sizeof(double *) * rows);
        for (size_t i = 0; i < rows; i++) {
            v->data[i] = malloc(sizeof(double ) * cols);
            memset(v->data[i],0,sizeof(double)*cols);
        }
    }


}

void vector2d_append_row(vector2d *v, vector *row_data) {

    if(v->data==NULL){
        v->data = malloc(sizeof(double *) * 1);
    }else{
        v->data = realloc(v->data, sizeof(double *) * (v->rows+1));
    }
    double *arr = malloc(sizeof(double)*row_data->size);
    for(size_t i=0; i< row_data->size; i++){
        arr[i] = vector_get(row_data,i);
    }
    // 向向量末尾添加新行
    v->data[v->rows] = arr;
    v->rows++;
    v->cols = row_data->size;
}

vector2d *vector2d_copy(const vector2d *v){
    vector2d* vec = malloc(sizeof(vector2d));
    vector2d_init(vec,v->rows,v->cols);
    for(int i=0;i<v->rows;i++){
        for(int j=0;j<v->cols;j++){
            vec->data[i][j] = v->data[i][j];
        }
    }
    return vec;
}

void vector2d_set_row(vector2d *v, size_t row_index, double *p){
    if (row_index >= v->rows) {
        printf("Error: Row index %zu out of range\n", row_index);
        exit(1);
    }
    //(v->data[row_index]);
    v->data[row_index] = p;
}

double *vector2d_get_row(vector2d *v, size_t row_index) {
    if (row_index >= v->rows) {
        printf("Error: Row index %zu out of range\n", row_index);
        exit(1);
    }
    return v->data[row_index];
}

double vector2d_get(vector2d *v, size_t row_index, size_t col_index) {
    if (row_index >= v->rows || col_index >= v->cols) {
        printf("Error: Index [%zu, %zu] out of range\n", row_index, col_index);
        exit(1);
    }
    return v->data[row_index][col_index];
}



void vector2d_set(vector2d *v, size_t row_index, size_t col_index, double value) {
    if (row_index >= v->rows || col_index >= v->cols) {
        printf("Error: Index [%zu, %zu] out of range\n", row_index, col_index);
        exit(1);
    }
    v->data[row_index][col_index] = value;
}

void vector2d_print(vector2d * v){
    for(size_t i=0;i<v->rows;i++){
        for(size_t j=0;j<v->cols;j++){
            printf("%g,",v->data[i][j]);
        }
        printf("\n");
    }
    printf("%ld,%ld\n",v->rows,v->cols);
}


void vector2d_free(vector2d *v) {
    for (size_t i = 0; i < v->rows; i++) {
        free(v->data[i]);
    }
    free(v->data);
    v->rows = 0;
    v->cols = 0;
}


/*
 * string set
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    char **data;      // 字符串数组的数据
    size_t size;      // 字符串数组的大小
    size_t capacity;  // 字符串数组可容纳字符串的数量
} string_set;

void string_set_init(string_set *arr) {
    arr->data = malloc(sizeof(char*) * INITIAL_CAPACITY);
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
}
void string_set_resize(string_set *v, int n){
    v->data = malloc(sizeof(char*) * n);
    v->size = n;
    v->capacity = n;
}
void string_set_insert(string_set *arr, const char *str) {
    if (arr->size >= arr->capacity) {
        // 当数组已满时，增加容量
        arr->capacity *= 2;
        arr->data = realloc(arr->data, sizeof(char*) * arr->capacity);
    }

    for(size_t i=0; i< arr->size; i++){
        if(strcmp(arr->data[i],str)==0){
            return;
        }
    }
    char *new_str = strdup(str);
    arr->data[arr->size] = new_str;
    arr->size++;
}

void string_set_free(string_set *arr) {
    for (size_t i = 0; i < arr->size; i++) {
        free(arr->data[i]);
    }
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

void string_set_print(string_set *arr) {

    for (size_t i = 0; i < arr->size; i++) {
        printf("%s,", arr->data[i]);
    }
    printf("%ld\n",arr->size);
}

int string_set_get(string_set *arr, const char *str){
    for(size_t i=0; i< arr->size; i++){
        if(strcmp(arr->data[i],str)==0){
            return i;
        }
    }
    return -1;
}


/*
 * int set
 */



typedef struct {
    int *data;      // 字符串数组的数据
    size_t size;      // 字符串数组的大小
    size_t capacity;  // 字符串数组可容纳字符串的数量
} int_set;

void int_set_init(int_set *arr) {
    arr->data = malloc(sizeof(int) * INITIAL_CAPACITY);
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
}

void int_set_insert(int_set *arr, int val) {
    if (arr->size >= arr->capacity) {
        // 当数组已满时，增加容量
        arr->capacity *= 2;
        arr->data = realloc(arr->data, sizeof(int) * arr->capacity);
    }

    for(size_t i=0; i< arr->size; i++){
        if((int)arr->data[i]==val){
            return;
        }
    }
    arr->data[arr->size] = val;
    arr->size++;
}

void int_set_free(int_set *arr) {

    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}


int int_set_get(int_set *arr, int val){
    int res = 0;
    for(size_t i=0; i< arr->size; i++){
        if(arr->data[i]==val){
            res++;
        }
    }
    return res;
}

/*
 * 工具函数
 */

// 随机打乱数组
void shuffle(int *arr, int len) {
    srand(time(NULL));  // 使用当前时间作为随机种子
    for (int i = len - 1; i > 0; i--) {
        // 从未洗牌的部分随机选取一个元素
        int j = rand() % (i + 1);
        // 交换当前元素和随机选取的元素
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}
void shuffled(double *arr, int len) {
    srand(time(NULL));  // 使用当前时间作为随机种子
    for (int i = len - 1; i > 0; i--) {
        // 从未洗牌的部分随机选取一个元素
        int j = rand() % (i + 1);
        // 交换当前元素和随机选取的元素
        double temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}



double accumulate(double *arr, int n){
    double res = 0;
    for(int i=0;i<n;i++){
        res += arr[i];
    }
    return res;
}

void selectionSort(double* arr, int n) {
    int i, j, minIdx;
    double temp;
    for (i = 0; i < n - 1; i++) {
        minIdx = i;
        for (j = i + 1; j < n; j++) {
            if (arr[j] < arr[minIdx]) {
                minIdx = j;
            }
        }
        if (minIdx != i) {
            temp = arr[i];
            arr[i] = arr[minIdx];
            arr[minIdx] = temp;
        }
    }
}

int min(int a,int b){
    if(a>b){
        return b;
    }else{
        return a;
    }
}
