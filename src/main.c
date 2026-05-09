#include "rcd_csv.h"
#include "rcd_ga.h"
#include "rcd_output.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void print_usage(const char *program) {
    fprintf(stderr, "Usage: %s <data_dir> [output_dir]\n", program);
    fprintf(stderr, "  data_dir   Directory containing Sheet1.csv and Sheet2.csv.\n");
    fprintf(stderr, "  output_dir Directory for draw.csv and output.csv. Defaults to data_dir.\n");
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *data_dir = argv[1];
    const char *output_dir = argc >= 3 ? argv[2] : data_dir;
    srand((unsigned)time(NULL));

    RcdDataset dataset;
    rcd_dataset_init(&dataset);
    if (rcd_read_dataset(data_dir, &dataset) != RCD_OK) {
        fprintf(stderr, "Failed to read input dataset.\n");
        rcd_dataset_free(&dataset);
        return EXIT_FAILURE;
    }

    printf("Loaded %zu nodes, %zu lines, %zu loads.\n",
           dataset.node_names.size, dataset.line.rows, dataset.load.rows);

    RcdVector fixed_branch_switches;
    rcd_vector_init(&fixed_branch_switches);
    if (rcd_find_heavy_branch_switches(&dataset.line, &dataset.load, &fixed_branch_switches) != RCD_OK) {
        fprintf(stderr, "Failed to detect heavy branch switches.\n");
        rcd_vector_free(&fixed_branch_switches);
        rcd_dataset_free(&dataset);
        return EXIT_FAILURE;
    }
    printf("Fixed heavy-branch switches: %zu\n", fixed_branch_switches.size);

    RcdGaConfig config = rcd_default_ga_config();
    RcdOptimizationResult result;
    rcd_optimization_result_init(&result);

    if (rcd_optimize_switches(&dataset.node_names,
                              &dataset.line,
                              &dataset.load,
                              &fixed_branch_switches,
                              &config,
                              &result) != RCD_OK) {
        fprintf(stderr, "Switch optimization failed.\n");
        rcd_optimization_result_free(&result);
        rcd_vector_free(&fixed_branch_switches);
        rcd_dataset_free(&dataset);
        return EXIT_FAILURE;
    }

    if (rcd_write_outputs(output_dir, &dataset.node_names, &dataset.line, &result) != RCD_OK) {
        fprintf(stderr, "Failed to write output files.\n");
        rcd_optimization_result_free(&result);
        rcd_vector_free(&fixed_branch_switches);
        rcd_dataset_free(&dataset);
        return EXIT_FAILURE;
    }

    printf("Recommended switch count: %d\n", result.switch_count);
    printf("ASAI before: %.12f\n", result.base_asai);
    printf("ASAI after : %.12f\n", result.optimized_asai);
    printf("Reduced outage hours/year: %.6f\n", result.reduced_outage_hours_per_year);
    printf("Output written to %s\n", output_dir);

    rcd_optimization_result_free(&result);
    rcd_vector_free(&fixed_branch_switches);
    rcd_dataset_free(&dataset);
    return EXIT_SUCCESS;
}
