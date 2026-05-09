#include "rcd_csv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 2048

static void trim_crlf(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[--len] = '\0';
    }
}

static int join_path(char *dst, size_t dst_size, const char *dir, const char *file) {
    size_t len = strlen(dir);
    const char *sep = (len > 0 && (dir[len - 1] == '/' || dir[len - 1] == '\\')) ? "" : "/";
    return snprintf(dst, dst_size, "%s%s%s", dir, sep, file) < (int)dst_size ? RCD_OK : RCD_ERR;
}

static int read_lines_csv(const char *path, RcdDataset *dataset) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror(path);
        return RCD_ERR;
    }

    char buffer[BUFFER_SIZE];
    size_t row_no = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        row_no++;
        if (row_no == 1) {
            continue;
        }
        trim_crlf(buffer);
        if (strlen(buffer) <= 10) {
            continue;
        }

        RcdVector row;
        rcd_vector_init(&row);
        char *token = strtok(buffer, ",");
        size_t col = 0;
        while (token) {
            col++;
            if (col == 1 || col == 2) {
                int id = rcd_strings_insert(&dataset->node_names, token);
                if (id < 0 || rcd_vector_push(&row, (double)id) != RCD_OK) {
                    rcd_vector_free(&row);
                    fclose(fp);
                    return RCD_ERR;
                }
            } else {
                char *end = NULL;
                double value = strtod(token, &end);
                if (rcd_vector_push(&row, value) != RCD_OK) {
                    rcd_vector_free(&row);
                    fclose(fp);
                    return RCD_ERR;
                }
            }
            token = strtok(NULL, ",");
        }

        if (rcd_matrix_push_row(&dataset->line, &row) != RCD_OK) {
            rcd_vector_free(&row);
            fclose(fp);
            return RCD_ERR;
        }
        rcd_vector_free(&row);
    }

    fclose(fp);
    return RCD_OK;
}

static int read_load_csv(const char *path, RcdDataset *dataset) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror(path);
        return RCD_ERR;
    }

    char buffer[BUFFER_SIZE];
    size_t row_no = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        row_no++;
        if (row_no == 1) {
            continue;
        }
        trim_crlf(buffer);
        if (strlen(buffer) <= 10) {
            continue;
        }

        RcdVector row;
        rcd_vector_init(&row);
        char *token = strtok(buffer, ",");
        size_t col = 0;
        while (token) {
            col++;
            if (col == 6) {
                int id = rcd_strings_index_of(&dataset->node_names, token);
                if (id < 0) {
                    fprintf(stderr, "Unknown load connection node: %s\n", token);
                    rcd_vector_free(&row);
                    fclose(fp);
                    return RCD_ERR;
                }
                if (rcd_vector_push(&row, (double)id) != RCD_OK) {
                    rcd_vector_free(&row);
                    fclose(fp);
                    return RCD_ERR;
                }
            } else {
                char *end = NULL;
                double value = strtod(token, &end);
                if (rcd_vector_push(&row, value) != RCD_OK) {
                    rcd_vector_free(&row);
                    fclose(fp);
                    return RCD_ERR;
                }
            }
            token = strtok(NULL, ",");
        }

        if (rcd_matrix_push_row(&dataset->load, &row) != RCD_OK) {
            rcd_vector_free(&row);
            fclose(fp);
            return RCD_ERR;
        }
        rcd_vector_free(&row);
    }

    fclose(fp);
    return RCD_OK;
}

int rcd_read_dataset(const char *data_dir, RcdDataset *dataset) {
    char sheet1[512];
    char sheet2[512];
    if (join_path(sheet1, sizeof(sheet1), data_dir, "Sheet1.csv") != RCD_OK ||
        join_path(sheet2, sizeof(sheet2), data_dir, "Sheet2.csv") != RCD_OK) {
        fprintf(stderr, "Data path is too long.\n");
        return RCD_ERR;
    }

    if (read_lines_csv(sheet1, dataset) != RCD_OK) {
        return RCD_ERR;
    }
    if (read_load_csv(sheet2, dataset) != RCD_OK) {
        return RCD_ERR;
    }
    return RCD_OK;
}
