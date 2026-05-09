#ifndef RCD_TYPES_H
#define RCD_TYPES_H

#include <stddef.h>

#define RCD_OK 0
#define RCD_ERR (-1)
#define RCD_INF 1.0e12

typedef struct {
    double *data;
    size_t size;
    size_t capacity;
} RcdVector;

typedef struct {
    double **data;
    size_t rows;
    size_t cols;
} RcdMatrix;

typedef struct {
    char **data;
    size_t size;
    size_t capacity;
} RcdStringSet;

typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} RcdIntSet;

typedef struct {
    RcdStringSet node_names;
    RcdMatrix line;
    RcdMatrix load;
} RcdDataset;

void rcd_vector_init(RcdVector *v);
int rcd_vector_resize(RcdVector *v, size_t n);
int rcd_vector_push(RcdVector *v, double value);
void rcd_vector_free(RcdVector *v);
RcdVector *rcd_vector_new(void);
void rcd_vector_delete(RcdVector *v);

void rcd_matrix_init(RcdMatrix *m);
int rcd_matrix_resize(RcdMatrix *m, size_t rows, size_t cols);
int rcd_matrix_push_row(RcdMatrix *m, const RcdVector *row);
RcdMatrix *rcd_matrix_clone(const RcdMatrix *m);
void rcd_matrix_free(RcdMatrix *m);
void rcd_matrix_delete(RcdMatrix *m);

void rcd_strings_init(RcdStringSet *s);
int rcd_strings_insert(RcdStringSet *s, const char *value);
int rcd_strings_index_of(const RcdStringSet *s, const char *value);
void rcd_strings_free(RcdStringSet *s);

void rcd_intset_init(RcdIntSet *s);
int rcd_intset_insert(RcdIntSet *s, int value);
int rcd_intset_contains(const RcdIntSet *s, int value);
void rcd_intset_free(RcdIntSet *s);

void rcd_dataset_init(RcdDataset *dataset);
void rcd_dataset_free(RcdDataset *dataset);

void rcd_sort_double(double *arr, size_t n);
double rcd_accumulate(const double *arr, size_t n);
void rcd_shuffle_int(int *arr, size_t n);
void rcd_shuffle_double(double *arr, size_t n);
int rcd_min_int(int a, int b);

#endif
