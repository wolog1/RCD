#include "rcd_ga.h"
#include "rcd_reliability.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RcdGaConfig rcd_default_ga_config(void) {
    RcdGaConfig cfg;
    cfg.population_size = 30;
    cfg.iterations = 50;
    cfg.crossover_probability = 0.6;
    cfg.mutation_probability = 0.001;
    cfg.max_switches = 25;
    cfg.asai_delta_threshold = 2.0e-7;
    return cfg;
}

void rcd_optimization_result_init(RcdOptimizationResult *result) {
    result->switch_count = 0;
    result->base_asai = 0.0;
    result->optimized_asai = 0.0;
    result->reduced_outage_hours_per_year = 0.0;
    rcd_vector_init(&result->selected_line_indices);
    rcd_vector_init(&result->priority_line_indices);
    rcd_vector_init(&result->fitness_curve);
}

void rcd_optimization_result_free(RcdOptimizationResult *result) {
    rcd_vector_free(&result->selected_line_indices);
    rcd_vector_free(&result->priority_line_indices);
    rcd_vector_free(&result->fitness_curve);
}

static double quantile(const RcdVector *x, double k) {
    if (x->size == 0) {
        return 0.0;
    }
    double *tmp = (double *)malloc(x->size * sizeof(double));
    if (!tmp) {
        return 0.0;
    }
    memcpy(tmp, x->data, x->size * sizeof(double));
    rcd_sort_double(tmp, x->size);
    size_t index = (size_t)(x->size * k);
    if (index == 0) {
        index = 1;
    }
    if (index > x->size) {
        index = x->size;
    }
    double value = tmp[index - 1];
    free(tmp);
    return value;
}

int rcd_find_heavy_branch_switches(const RcdMatrix *line,
                                   const RcdMatrix *load,
                                   RcdVector *branch_line_indices) {
    RcdVector load_values;
    rcd_vector_init(&load_values);
    for (size_t i = 0; i < load->rows; ++i) {
        if (rcd_vector_push(&load_values, load->data[i][2]) != RCD_OK) {
            rcd_vector_free(&load_values);
            return RCD_ERR;
        }
    }

    double q1 = quantile(&load_values, 0.25);
    double q3 = quantile(&load_values, 0.75);
    double iqr = q3 - q1;

    for (size_t i = 0; i < load->rows; ++i) {
        if (load->data[i][2] <= q3 + 1.5 * iqr) {
            continue;
        }
        int load_node = (int)load->data[i][load->cols - 1];
        for (size_t j = 0; j < line->rows; ++j) {
            if ((int)line->data[j][1] == load_node &&
                (int)line->data[j][5] == 0 &&
                (int)line->data[j][6] == 0 &&
                (int)line->data[j][9] == 2) {
                if (rcd_vector_push(branch_line_indices, (double)j) != RCD_OK) {
                    rcd_vector_free(&load_values);
                    return RCD_ERR;
                }
                break;
            }
        }
    }

    rcd_vector_free(&load_values);
    return RCD_OK;
}

static RcdMatrix *init_population(int pop_size, int gene_count, int selected_count) {
    RcdMatrix *pop = (RcdMatrix *)malloc(sizeof(RcdMatrix));
    if (!pop) {
        return NULL;
    }
    rcd_matrix_init(pop);
    if (rcd_matrix_resize(pop, (size_t)pop_size, (size_t)gene_count) != RCD_OK) {
        free(pop);
        return NULL;
    }

    double *origin = (double *)calloc((size_t)gene_count, sizeof(double));
    if (!origin) {
        rcd_matrix_delete(pop);
        return NULL;
    }
    for (int i = 0; i < selected_count && i < gene_count; ++i) {
        origin[i] = 1.0;
    }
    for (int i = 0; i < pop_size; ++i) {
        rcd_shuffle_double(origin, (size_t)gene_count);
        for (int j = 0; j < gene_count; ++j) {
            pop->data[i][j] = origin[j];
        }
    }
    free(origin);
    return pop;
}

static void collect_candidate_lines(const RcdMatrix *line, RcdVector *indices) {
    for (size_t i = 0; i < line->rows; ++i) {
        if ((int)line->data[i][5] == 0 &&
            (int)line->data[i][6] == 0 &&
            (int)line->data[i][8] == 1) {
            rcd_vector_push(indices, (double)i);
        }
    }
}

static void apply_fixed_switches(RcdMatrix *line, const RcdVector *fixed) {
    for (size_t i = 0; i < fixed->size; ++i) {
        int idx = (int)fixed->data[i];
        if (idx >= 0 && (size_t)idx < line->rows) {
            line->data[idx][5] = 2.0;
        }
    }
}

static RcdVector *calculate_fitness(const RcdMatrix *pop,
                                    const RcdVector *candidate_lines,
                                    RcdMatrix *line,
                                    const RcdMatrix *load,
                                    size_t node_count) {
    RcdVector *fitness = rcd_vector_new();
    if (!fitness || rcd_vector_resize(fitness, pop->rows) != RCD_OK) {
        rcd_vector_delete(fitness);
        return NULL;
    }

    for (size_t i = 0; i < pop->rows; ++i) {
        for (size_t j = 0; j < candidate_lines->size; ++j) {
            int line_index = (int)candidate_lines->data[j];
            line->data[line_index][5] = pop->data[i][j] * 2.0;
        }
        fitness->data[i] = rcd_calculate_asai(line, load, node_count);
    }
    return fitness;
}

static RcdMatrix *selection(const RcdMatrix *pop, const RcdVector *fitness) {
    double min_fit = fitness->data[0];
    for (size_t i = 1; i < fitness->size; ++i) {
        if (fitness->data[i] < min_fit) {
            min_fit = fitness->data[i];
        }
    }

    double total = 0.0;
    double *cdf = (double *)malloc(fitness->size * sizeof(double));
    if (!cdf) {
        return NULL;
    }
    for (size_t i = 0; i < fitness->size; ++i) {
        total += fitness->data[i] - min_fit + 1e-12;
        cdf[i] = total;
    }

    RcdMatrix *newpop = (RcdMatrix *)malloc(sizeof(RcdMatrix));
    if (!newpop) {
        free(cdf);
        return NULL;
    }
    rcd_matrix_init(newpop);
    if (rcd_matrix_resize(newpop, pop->rows, pop->cols) != RCD_OK) {
        free(cdf);
        free(newpop);
        return NULL;
    }

    for (size_t i = 0; i < pop->rows; ++i) {
        double r = ((double)rand() / RAND_MAX) * total;
        size_t selected = 0;
        while (selected + 1 < fitness->size && r > cdf[selected]) {
            selected++;
        }
        memcpy(newpop->data[i], pop->data[selected], pop->cols * sizeof(double));
    }

    free(cdf);
    return newpop;
}

static void repair_selected_count(double *row, size_t cols, int selected_count) {
    int current = 0;
    for (size_t i = 0; i < cols; ++i) {
        current += (int)row[i];
    }
    if (current == selected_count) {
        return;
    }

    int *indices = (int *)malloc(cols * sizeof(int));
    if (!indices) {
        return;
    }
    for (size_t i = 0; i < cols; ++i) {
        indices[i] = (int)i;
    }
    rcd_shuffle_int(indices, cols);

    if (current > selected_count) {
        for (size_t k = 0; k < cols && current > selected_count; ++k) {
            int idx = indices[k];
            if ((int)row[idx] == 1) {
                row[idx] = 0.0;
                current--;
            }
        }
    } else {
        for (size_t k = 0; k < cols && current < selected_count; ++k) {
            int idx = indices[k];
            if ((int)row[idx] == 0) {
                row[idx] = 1.0;
                current++;
            }
        }
    }
    free(indices);
}

static void crossover(RcdMatrix *pop, double probability, int selected_count) {
    for (size_t i = 0; i + 1 < pop->rows; i += 2) {
        if ((double)rand() / RAND_MAX >= probability || pop->cols < 2) {
            continue;
        }
        size_t point = 1 + (size_t)(rand() % (int)(pop->cols - 1));
        for (size_t j = point; j < pop->cols; ++j) {
            double tmp = pop->data[i][j];
            pop->data[i][j] = pop->data[i + 1][j];
            pop->data[i + 1][j] = tmp;
        }
        repair_selected_count(pop->data[i], pop->cols, selected_count);
        repair_selected_count(pop->data[i + 1], pop->cols, selected_count);
    }
}

static void mutation(RcdMatrix *pop, double probability) {
    for (size_t i = 0; i < pop->rows; ++i) {
        if ((double)rand() / RAND_MAX >= probability || pop->cols < 2) {
            continue;
        }
        int a = rand() % (int)pop->cols;
        int b = rand() % (int)pop->cols;
        double tmp = pop->data[i][a];
        pop->data[i][a] = pop->data[i][b];
        pop->data[i][b] = tmp;
    }
}

static int run_ga_for_count(const RcdMatrix *base_line,
                            const RcdMatrix *load,
                            size_t node_count,
                            const RcdVector *candidate_lines,
                            const RcdGaConfig *config,
                            int switch_count,
                            RcdVector *best_genes,
                            double *best_asai) {
    RcdMatrix *line = rcd_matrix_clone(base_line);
    RcdMatrix *pop = init_population(config->population_size, (int)candidate_lines->size, switch_count);
    if (!line || !pop) {
        rcd_matrix_delete(line);
        rcd_matrix_delete(pop);
        return RCD_ERR;
    }

    *best_asai = -1.0;
    if (rcd_vector_resize(best_genes, candidate_lines->size) != RCD_OK) {
        rcd_matrix_delete(line);
        rcd_matrix_delete(pop);
        return RCD_ERR;
    }

    for (int iter = 0; iter < config->iterations; ++iter) {
        RcdVector *fitness = calculate_fitness(pop, candidate_lines, line, load, node_count);
        if (!fitness) {
            rcd_matrix_delete(line);
            rcd_matrix_delete(pop);
            return RCD_ERR;
        }

        size_t best_pos = 0;
        for (size_t i = 1; i < fitness->size; ++i) {
            if (fitness->data[i] > fitness->data[best_pos]) {
                best_pos = i;
            }
        }
        if (fitness->data[best_pos] > *best_asai) {
            *best_asai = fitness->data[best_pos];
            memcpy(best_genes->data, pop->data[best_pos], candidate_lines->size * sizeof(double));
        }

        RcdMatrix *selected = selection(pop, fitness);
        rcd_vector_delete(fitness);
        if (!selected) {
            rcd_matrix_delete(line);
            rcd_matrix_delete(pop);
            return RCD_ERR;
        }
        rcd_matrix_delete(pop);
        pop = selected;
        crossover(pop, config->crossover_probability, switch_count);
        mutation(pop, config->mutation_probability);
    }

    rcd_matrix_delete(line);
    rcd_matrix_delete(pop);
    return RCD_OK;
}

static int append_selected_lines(const RcdVector *genes,
                                 const RcdVector *candidate_lines,
                                 RcdVector *out) {
    for (size_t i = 0; i < genes->size; ++i) {
        if ((int)genes->data[i] == 1) {
            if (rcd_vector_push(out, candidate_lines->data[i]) != RCD_OK) {
                return RCD_ERR;
            }
        }
    }
    return RCD_OK;
}

static int build_priority_sequence(const RcdStringSet *node_names,
                                   const RcdMatrix *line,
                                   const RcdMatrix *load,
                                   const RcdVector *selected_lines,
                                   RcdVector *priority) {
    (void)node_names;
    RcdVector remaining;
    RcdVector chosen;
    rcd_vector_init(&remaining);
    rcd_vector_init(&chosen);

    for (size_t i = 0; i < selected_lines->size; ++i) {
        rcd_vector_push(&remaining, selected_lines->data[i]);
    }

    while (remaining.size > 0) {
        double best_asai = -1.0;
        size_t best_pos = 0;
        for (size_t i = 0; i < remaining.size; ++i) {
            RcdMatrix *trial = rcd_matrix_clone(line);
            if (!trial) {
                rcd_vector_free(&remaining);
                rcd_vector_free(&chosen);
                return RCD_ERR;
            }
            for (size_t k = 0; k < chosen.size; ++k) {
                int idx = (int)chosen.data[k];
                trial->data[idx][5] = 2.0;
            }
            int idx = (int)remaining.data[i];
            trial->data[idx][5] = 2.0;
            double asai = rcd_calculate_asai(trial, load, node_names->size);
            rcd_matrix_delete(trial);
            if (asai > best_asai) {
                best_asai = asai;
                best_pos = i;
            }
        }
        double line_idx = remaining.data[best_pos];
        rcd_vector_push(priority, line_idx);
        rcd_vector_push(&chosen, line_idx);
        for (size_t j = best_pos + 1; j < remaining.size; ++j) {
            remaining.data[j - 1] = remaining.data[j];
        }
        remaining.size--;
    }

    rcd_vector_free(&remaining);
    rcd_vector_free(&chosen);
    return RCD_OK;
}

int rcd_optimize_switches(const RcdStringSet *node_names,
                          const RcdMatrix *line,
                          const RcdMatrix *load,
                          const RcdVector *fixed_branch_switches,
                          const RcdGaConfig *config,
                          RcdOptimizationResult *result) {
    RcdGaConfig cfg = config ? *config : rcd_default_ga_config();
    RcdMatrix *base_line = rcd_matrix_clone(line);
    if (!base_line) {
        return RCD_ERR;
    }
    apply_fixed_switches(base_line, fixed_branch_switches);

    result->base_asai = rcd_calculate_asai(line, load, node_names->size);

    RcdVector candidate_lines;
    rcd_vector_init(&candidate_lines);
    collect_candidate_lines(base_line, &candidate_lines);
    if (candidate_lines.size == 0) {
        rcd_matrix_delete(base_line);
        rcd_vector_free(&candidate_lines);
        return RCD_ERR;
    }

    int max_switches = cfg.max_switches;
    if ((size_t)max_switches > candidate_lines.size) {
        max_switches = (int)candidate_lines.size;
    }

    RcdVector best_genes;
    rcd_vector_init(&best_genes);
    double previous_asai = -1.0;
    double best_overall_asai = -1.0;
    int best_count = 1;

    for (int n = 1; n <= max_switches; ++n) {
        RcdVector genes;
        rcd_vector_init(&genes);
        double asai = 0.0;
        if (run_ga_for_count(base_line, load, node_names->size, &candidate_lines, &cfg, n, &genes, &asai) != RCD_OK) {
            rcd_vector_free(&genes);
            rcd_vector_free(&best_genes);
            rcd_vector_free(&candidate_lines);
            rcd_matrix_delete(base_line);
            return RCD_ERR;
        }
        rcd_vector_push(&result->fitness_curve, asai);
        if (asai > best_overall_asai) {
            best_overall_asai = asai;
            best_count = n;
            rcd_vector_resize(&best_genes, genes.size);
            memcpy(best_genes.data, genes.data, genes.size * sizeof(double));
        }
        if (previous_asai > 0.0 && fabs(asai - previous_asai) <= cfg.asai_delta_threshold) {
            rcd_vector_free(&genes);
            break;
        }
        previous_asai = asai;
        rcd_vector_free(&genes);
    }

    result->switch_count = best_count + (int)fixed_branch_switches->size;
    result->optimized_asai = best_overall_asai;
    result->reduced_outage_hours_per_year = 8760.0 * (result->optimized_asai - result->base_asai);

    append_selected_lines(&best_genes, &candidate_lines, &result->selected_line_indices);
    for (size_t i = 0; i < fixed_branch_switches->size; ++i) {
        rcd_vector_push(&result->selected_line_indices, fixed_branch_switches->data[i]);
    }
    build_priority_sequence(node_names, line, load, &result->selected_line_indices, &result->priority_line_indices);

    rcd_vector_free(&best_genes);
    rcd_vector_free(&candidate_lines);
    rcd_matrix_delete(base_line);
    return RCD_OK;
}
