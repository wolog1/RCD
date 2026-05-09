#include "rcd_reliability.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static RcdMatrix *build_graph(const RcdMatrix *line, size_t node_count, double *indegree) {
    RcdMatrix *graph = (RcdMatrix *)malloc(sizeof(RcdMatrix));
    if (!graph) {
        return NULL;
    }
    rcd_matrix_init(graph);
    if (rcd_matrix_resize(graph, node_count, node_count) != RCD_OK) {
        free(graph);
        return NULL;
    }
    for (size_t i = 0; i < node_count; ++i) {
        for (size_t j = 0; j < node_count; ++j) {
            graph->data[i][j] = RCD_INF;
        }
    }
    for (size_t i = 0; i < line->rows; ++i) {
        int from = (int)line->data[i][0];
        int to = (int)line->data[i][1];
        if (from >= 0 && to >= 0 && (size_t)from < node_count && (size_t)to < node_count) {
            indegree[to] += 1.0;
            graph->data[from][to] = line->data[i][2];
        }
    }
    return graph;
}

static int dijkstra(size_t n, int source, const RcdMatrix *graph, RcdVector *dist, RcdVector *prev) {
    RcdVector visited;
    rcd_vector_init(&visited);
    if (rcd_vector_resize(&visited, n) != RCD_OK ||
        rcd_vector_resize(dist, n) != RCD_OK ||
        rcd_vector_resize(prev, n) != RCD_OK) {
        rcd_vector_free(&visited);
        return RCD_ERR;
    }

    for (size_t i = 0; i < n; ++i) {
        visited.data[i] = 0.0;
        dist->data[i] = RCD_INF;
        prev->data[i] = (double)i;
    }
    dist->data[source] = 0.0;

    for (size_t i = 0; i < n; ++i) {
        int u = -1;
        double best = RCD_INF;
        for (size_t j = 0; j < n; ++j) {
            if (visited.data[j] == 0.0 && dist->data[j] < best) {
                best = dist->data[j];
                u = (int)j;
            }
        }
        if (u < 0) {
            break;
        }
        visited.data[u] = 1.0;
        for (size_t v = 0; v < n; ++v) {
            if (visited.data[v] == 0.0 && dist->data[u] + graph->data[u][v] < dist->data[v]) {
                dist->data[v] = dist->data[u] + graph->data[u][v];
                prev->data[v] = (double)u;
            }
        }
    }

    rcd_vector_free(&visited);
    return RCD_OK;
}

static RcdVector *get_path(size_t node_count, int source, const RcdMatrix *graph, int dest) {
    RcdVector dist;
    RcdVector prev;
    rcd_vector_init(&dist);
    rcd_vector_init(&prev);
    if (dijkstra(node_count, source, graph, &dist, &prev) != RCD_OK) {
        rcd_vector_free(&dist);
        rcd_vector_free(&prev);
        return NULL;
    }
    if (dest < 0 || (size_t)dest >= node_count || dist.data[dest] >= RCD_INF / 2) {
        rcd_vector_free(&dist);
        rcd_vector_free(&prev);
        return NULL;
    }

    RcdVector *reverse = rcd_vector_new();
    RcdVector *path = rcd_vector_new();
    if (!reverse || !path) {
        rcd_vector_delete(reverse);
        rcd_vector_delete(path);
        rcd_vector_free(&dist);
        rcd_vector_free(&prev);
        return NULL;
    }

    int current = dest;
    while (current != source) {
        if (rcd_vector_push(reverse, current) != RCD_OK) {
            rcd_vector_delete(reverse);
            rcd_vector_delete(path);
            rcd_vector_free(&dist);
            rcd_vector_free(&prev);
            return NULL;
        }
        int next = (int)prev.data[current];
        if (next == current) {
            rcd_vector_delete(reverse);
            rcd_vector_delete(path);
            rcd_vector_free(&dist);
            rcd_vector_free(&prev);
            return NULL;
        }
        current = next;
    }
    rcd_vector_push(path, source);
    for (size_t i = reverse->size; i > 0; --i) {
        rcd_vector_push(path, reverse->data[i - 1]);
    }

    rcd_vector_delete(reverse);
    rcd_vector_free(&dist);
    rcd_vector_free(&prev);
    return path;
}

static int line_index_by_edge(const RcdMatrix *line, int from, int to) {
    for (size_t i = 0; i < line->rows; ++i) {
        if ((int)line->data[i][0] == from && (int)line->data[i][1] == to) {
            return (int)i;
        }
    }
    return -1;
}

int rcd_calculate_reliability(const RcdMatrix *line,
                              const RcdMatrix *load,
                              size_t node_count,
                              RcdReliabilityMetrics *metrics) {
    if (!line || !load || !metrics || node_count == 0) {
        return RCD_ERR;
    }

    memset(metrics, 0, sizeof(*metrics));
    double *indegree = (double *)calloc(node_count, sizeof(double));
    double *lambda_load = (double *)calloc(load->rows, sizeof(double));
    double *gamma_load = (double *)calloc(load->rows, sizeof(double));
    double *u_load = (double *)calloc(load->rows, sizeof(double));
    double *load_users = (double *)calloc(load->rows, sizeof(double));
    if (!indegree || !lambda_load || !gamma_load || !u_load || !load_users) {
        free(indegree); free(lambda_load); free(gamma_load); free(u_load); free(load_users);
        return RCD_ERR;
    }

    RcdMatrix *graph = build_graph(line, node_count, indegree);
    if (!graph) {
        free(indegree); free(lambda_load); free(gamma_load); free(u_load); free(load_users);
        return RCD_ERR;
    }

    RcdVector sources;
    rcd_vector_init(&sources);
    for (size_t i = 0; i < node_count; ++i) {
        if ((int)indegree[i] == 0) {
            rcd_vector_push(&sources, (double)i);
        }
    }

    for (size_t s = 0; s < sources.size; ++s) {
        int source = (int)sources.data[s];
        for (size_t i = 0; i < load->rows; ++i) {
            int load_node = (int)load->data[i][5];
            RcdVector *path = get_path(node_count, source, graph, load_node);
            if (!path || path->size < 2) {
                rcd_vector_delete(path);
                continue;
            }

            RcdIntSet path_lines;
            rcd_intset_init(&path_lines);
            RcdVector equipment;
            RcdVector operators;
            rcd_vector_init(&equipment);
            rcd_vector_init(&operators);

            for (size_t k = 0; k + 1 < path->size; ++k) {
                int idx = line_index_by_edge(line, (int)path->data[k], (int)path->data[k + 1]);
                if (idx >= 0) {
                    rcd_intset_insert(&path_lines, idx);
                    rcd_vector_push(&equipment, (double)idx);
                }
            }

            for (size_t m = 0; m < line->rows; ++m) {
                if (rcd_intset_contains(&path_lines, (int)m)) {
                    continue;
                }
                int endpoint = (int)line->data[m][1];
                RcdVector *path2 = get_path(node_count, source, graph, endpoint);
                if (!path2 || path2->size < 2) {
                    rcd_vector_delete(path2);
                    continue;
                }

                int c1 = 0;
                int c2 = 0;
                int c3 = 0;
                for (size_t k = 0; k + 1 < path2->size; ++k) {
                    int e = line_index_by_edge(line, (int)path2->data[k], (int)path2->data[k + 1]);
                    if (e < 0) {
                        continue;
                    }
                    int sw = (int)line->data[e][5];
                    if (sw == 1 || sw == 2 || sw == 3) {
                        c1++;
                    }
                    if (sw == 1) {
                        c2++;
                    }
                    if (sw == 2 || sw == 3) {
                        c3++;
                    }
                }

                if (c1 == 0) {
                    rcd_vector_push(&equipment, (double)m);
                } else if (c2 > 0 && c3 == 0) {
                    rcd_vector_push(&operators, (double)m);
                }
                rcd_vector_delete(path2);
            }

            for (size_t k = 0; k < equipment.size; ++k) {
                int e = (int)equipment.data[k];
                lambda_load[i] += line->data[e][2] * line->data[e][3];
                gamma_load[i] += line->data[e][2] * line->data[e][3] * line->data[e][4];
            }
            for (size_t k = 0; k < operators.size; ++k) {
                int o = (int)operators.data[k];
                lambda_load[i] += line->data[o][2] * line->data[o][3];
                gamma_load[i] += line->data[o][2] * line->data[o][3] * line->data[o][7];
            }

            if (lambda_load[i] > 0.0) {
                gamma_load[i] /= lambda_load[i];
                u_load[i] = gamma_load[i] * lambda_load[i];
            } else {
                u_load[i] = NAN;
            }
            load_users[i] = load->data[i][4];

            rcd_vector_free(&equipment);
            rcd_vector_free(&operators);
            rcd_intset_free(&path_lines);
            rcd_vector_delete(path);
        }
    }

    double total_users = rcd_accumulate(load_users, load->rows);
    double inner = 0.0;
    for (size_t i = 0; i < load->rows; ++i) {
        inner += lambda_load[i] * load_users[i];
    }
    metrics->saifi = total_users > 0.0 ? inner / total_users : 0.0;

    inner = 0.0;
    double valid_users = 0.0;
    for (size_t i = 0; i < load->rows; ++i) {
        if (!isnan(u_load[i])) {
            inner += u_load[i] * load_users[i];
            valid_users += load_users[i];
        }
    }
    metrics->saidi = valid_users > 0.0 ? inner / valid_users : 0.0;
    metrics->caidi = metrics->saifi > 0.0 ? metrics->saidi / metrics->saifi : 0.0;
    metrics->asai = 1.0 - metrics->saidi / 8760.0;

    rcd_vector_free(&sources);
    rcd_matrix_delete(graph);
    free(indegree);
    free(lambda_load);
    free(gamma_load);
    free(u_load);
    free(load_users);
    return RCD_OK;
}

double rcd_calculate_asai(const RcdMatrix *line, const RcdMatrix *load, size_t node_count) {
    RcdReliabilityMetrics metrics;
    if (rcd_calculate_reliability(line, load, node_count, &metrics) != RCD_OK) {
        return 0.0;
    }
    return metrics.asai;
}
