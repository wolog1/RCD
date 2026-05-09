#include "rcd_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 4

static char *rcd_strdup(const char *src) {
    size_t len = strlen(src) + 1;
    char *dst = (char *)malloc(len);
    if (!dst) {
        return NULL;
    }
    memcpy(dst, src, len);
    return dst;
}

void rcd_vector_init(RcdVector *v) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

int rcd_vector_resize(RcdVector *v, size_t n) {
    double *data = NULL;
    if (n > 0) {
        data = (double *)calloc(n, sizeof(double));
        if (!data) {
            return RCD_ERR;
        }
    }
    free(v->data);
    v->data = data;
    v->size = n;
    v->capacity = n;
    return RCD_OK;
}

int rcd_vector_push(RcdVector *v, double value) {
    if (v->size == v->capacity) {
        size_t next_capacity = v->capacity == 0 ? INITIAL_CAPACITY : v->capacity * 2;
        double *next = (double *)realloc(v->data, next_capacity * sizeof(double));
        if (!next) {
            return RCD_ERR;
        }
        v->data = next;
        v->capacity = next_capacity;
    }
    v->data[v->size++] = value;
    return RCD_OK;
}

void rcd_vector_free(RcdVector *v) {
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

RcdVector *rcd_vector_new(void) {
    RcdVector *v = (RcdVector *)malloc(sizeof(RcdVector));
    if (!v) {
        return NULL;
    }
    rcd_vector_init(v);
    return v;
}

void rcd_vector_delete(RcdVector *v) {
    if (!v) {
        return;
    }
    rcd_vector_free(v);
    free(v);
}

void rcd_matrix_init(RcdMatrix *m) {
    m->data = NULL;
    m->rows = 0;
    m->cols = 0;
}

int rcd_matrix_resize(RcdMatrix *m, size_t rows, size_t cols) {
    rcd_matrix_free(m);
    m->rows = rows;
    m->cols = cols;
    if (rows == 0 || cols == 0) {
        return RCD_OK;
    }
    m->data = (double **)calloc(rows, sizeof(double *));
    if (!m->data) {
        return RCD_ERR;
    }
    for (size_t i = 0; i < rows; ++i) {
        m->data[i] = (double *)calloc(cols, sizeof(double));
        if (!m->data[i]) {
            rcd_matrix_free(m);
            return RCD_ERR;
        }
    }
    return RCD_OK;
}

int rcd_matrix_push_row(RcdMatrix *m, const RcdVector *row) {
    if (!row || row->size == 0) {
        return RCD_ERR;
    }
    if (m->rows > 0 && m->cols != row->size) {
        return RCD_ERR;
    }
    double **next_rows = (double **)realloc(m->data, (m->rows + 1) * sizeof(double *));
    if (!next_rows) {
        return RCD_ERR;
    }
    m->data = next_rows;
    m->data[m->rows] = (double *)malloc(row->size * sizeof(double));
    if (!m->data[m->rows]) {
        return RCD_ERR;
    }
    memcpy(m->data[m->rows], row->data, row->size * sizeof(double));
    m->rows += 1;
    m->cols = row->size;
    return RCD_OK;
}

RcdMatrix *rcd_matrix_clone(const RcdMatrix *m) {
    RcdMatrix *copy = (RcdMatrix *)malloc(sizeof(RcdMatrix));
    if (!copy) {
        return NULL;
    }
    rcd_matrix_init(copy);
    if (rcd_matrix_resize(copy, m->rows, m->cols) != RCD_OK) {
        free(copy);
        return NULL;
    }
    for (size_t i = 0; i < m->rows; ++i) {
        memcpy(copy->data[i], m->data[i], m->cols * sizeof(double));
    }
    return copy;
}

void rcd_matrix_free(RcdMatrix *m) {
    if (!m) {
        return;
    }
    for (size_t i = 0; i < m->rows; ++i) {
        free(m->data[i]);
    }
    free(m->data);
    m->data = NULL;
    m->rows = 0;
    m->cols = 0;
}

void rcd_matrix_delete(RcdMatrix *m) {
    if (!m) {
        return;
    }
    rcd_matrix_free(m);
    free(m);
}

void rcd_strings_init(RcdStringSet *s) {
    s->data = NULL;
    s->size = 0;
    s->capacity = 0;
}

int rcd_strings_insert(RcdStringSet *s, const char *value) {
    int existing = rcd_strings_index_of(s, value);
    if (existing >= 0) {
        return existing;
    }
    if (s->size == s->capacity) {
        size_t next_capacity = s->capacity == 0 ? INITIAL_CAPACITY : s->capacity * 2;
        char **next = (char **)realloc(s->data, next_capacity * sizeof(char *));
        if (!next) {
            return RCD_ERR;
        }
        s->data = next;
        s->capacity = next_capacity;
    }
    s->data[s->size] = rcd_strdup(value);
    if (!s->data[s->size]) {
        return RCD_ERR;
    }
    return (int)s->size++;
}

int rcd_strings_index_of(const RcdStringSet *s, const char *value) {
    for (size_t i = 0; i < s->size; ++i) {
        if (strcmp(s->data[i], value) == 0) {
            return (int)i;
        }
    }
    return -1;
}

void rcd_strings_free(RcdStringSet *s) {
    for (size_t i = 0; i < s->size; ++i) {
        free(s->data[i]);
    }
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->capacity = 0;
}

void rcd_intset_init(RcdIntSet *s) {
    s->data = NULL;
    s->size = 0;
    s->capacity = 0;
}

int rcd_intset_insert(RcdIntSet *s, int value) {
    if (rcd_intset_contains(s, value)) {
        return RCD_OK;
    }
    if (s->size == s->capacity) {
        size_t next_capacity = s->capacity == 0 ? INITIAL_CAPACITY : s->capacity * 2;
        int *next = (int *)realloc(s->data, next_capacity * sizeof(int));
        if (!next) {
            return RCD_ERR;
        }
        s->data = next;
        s->capacity = next_capacity;
    }
    s->data[s->size++] = value;
    return RCD_OK;
}

int rcd_intset_contains(const RcdIntSet *s, int value) {
    for (size_t i = 0; i < s->size; ++i) {
        if (s->data[i] == value) {
            return 1;
        }
    }
    return 0;
}

void rcd_intset_free(RcdIntSet *s) {
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->capacity = 0;
}

void rcd_dataset_init(RcdDataset *dataset) {
    rcd_strings_init(&dataset->node_names);
    rcd_matrix_init(&dataset->line);
    rcd_matrix_init(&dataset->load);
}

void rcd_dataset_free(RcdDataset *dataset) {
    rcd_strings_free(&dataset->node_names);
    rcd_matrix_free(&dataset->line);
    rcd_matrix_free(&dataset->load);
}

void rcd_sort_double(double *arr, size_t n) {
    for (size_t i = 0; i + 1 < n; ++i) {
        size_t min_idx = i;
        for (size_t j = i + 1; j < n; ++j) {
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            double tmp = arr[i];
            arr[i] = arr[min_idx];
            arr[min_idx] = tmp;
        }
    }
}

double rcd_accumulate(const double *arr, size_t n) {
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) {
        sum += arr[i];
    }
    return sum;
}

void rcd_shuffle_int(int *arr, size_t n) {
    if (n < 2) {
        return;
    }
    for (size_t i = n - 1; i > 0; --i) {
        size_t j = (size_t)(rand() % (int)(i + 1));
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

void rcd_shuffle_double(double *arr, size_t n) {
    if (n < 2) {
        return;
    }
    for (size_t i = n - 1; i > 0; --i) {
        size_t j = (size_t)(rand() % (int)(i + 1));
        double tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int rcd_min_int(int a, int b) {
    return a < b ? a : b;
}
