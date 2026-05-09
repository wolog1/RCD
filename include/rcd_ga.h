#ifndef RCD_GA_H
#define RCD_GA_H

#include "rcd_types.h"

typedef struct {
    int population_size;
    int iterations;
    double crossover_probability;
    double mutation_probability;
    int max_switches;
    double asai_delta_threshold;
} RcdGaConfig;

typedef struct {
    int switch_count;
    double base_asai;
    double optimized_asai;
    double reduced_outage_hours_per_year;
    RcdVector selected_line_indices;
    RcdVector priority_line_indices;
    RcdVector fitness_curve;
} RcdOptimizationResult;

RcdGaConfig rcd_default_ga_config(void);
void rcd_optimization_result_init(RcdOptimizationResult *result);
void rcd_optimization_result_free(RcdOptimizationResult *result);

int rcd_find_heavy_branch_switches(const RcdMatrix *line,
                                   const RcdMatrix *load,
                                   RcdVector *branch_line_indices);

int rcd_optimize_switches(const RcdStringSet *node_names,
                          const RcdMatrix *line,
                          const RcdMatrix *load,
                          const RcdVector *fixed_branch_switches,
                          const RcdGaConfig *config,
                          RcdOptimizationResult *result);

#endif
