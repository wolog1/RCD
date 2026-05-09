#include "rcd_output.h"

#include <stdio.h>
#include <string.h>

static int join_path(char *dst, size_t dst_size, const char *dir, const char *file) {
    size_t len = strlen(dir);
    const char *sep = (len > 0 && (dir[len - 1] == '/' || dir[len - 1] == '\\')) ? "" : "/";
    return snprintf(dst, dst_size, "%s%s%s", dir, sep, file) < (int)dst_size ? RCD_OK : RCD_ERR;
}

static const char *line_install_node_name(const RcdStringSet *node_names,
                                          const RcdMatrix *line,
                                          int line_index) {
    if (line_index < 0 || (size_t)line_index >= line->rows) {
        return "UNKNOWN";
    }
    int node_index = (int)line->data[line_index][0];
    if (node_index < 0 || (size_t)node_index >= node_names->size) {
        return "UNKNOWN";
    }
    return node_names->data[node_index];
}

int rcd_write_outputs(const char *output_dir,
                      const RcdStringSet *node_names,
                      const RcdMatrix *line,
                      const RcdOptimizationResult *result) {
    char draw_path[512];
    char output_path[512];
    if (join_path(draw_path, sizeof(draw_path), output_dir, "draw.csv") != RCD_OK ||
        join_path(output_path, sizeof(output_path), output_dir, "output.csv") != RCD_OK) {
        fprintf(stderr, "Output path is too long.\n");
        return RCD_ERR;
    }

    FILE *draw = fopen(draw_path, "w");
    if (!draw) {
        perror(draw_path);
        return RCD_ERR;
    }
    fprintf(draw, "switch_count,asai\n");
    for (size_t i = 0; i < result->fitness_curve.size; ++i) {
        fprintf(draw, "%zu,%.12f\n", i + 1, result->fitness_curve.data[i]);
    }
    fclose(draw);

    FILE *out = fopen(output_path, "w");
    if (!out) {
        perror(output_path);
        return RCD_ERR;
    }
    fprintf(out, "新增开关:\n");
    fprintf(out, "序号,线路索引,安装位置\n");
    for (size_t i = 0; i < result->selected_line_indices.size; ++i) {
        int line_index = (int)result->selected_line_indices.data[i];
        fprintf(out, "%zu,%d,%s\n", i + 1, line_index,
                line_install_node_name(node_names, line, line_index));
    }

    fprintf(out, "\n优先级,线路索引,安装位置\n");
    for (size_t i = 0; i < result->priority_line_indices.size; ++i) {
        int line_index = (int)result->priority_line_indices.data[i];
        fprintf(out, "%zu,%d,%s\n", i + 1, line_index,
                line_install_node_name(node_names, line, line_index));
    }

    fprintf(out, "\n优化前后可靠性指标ASAI:\n");
    fprintf(out, "优化前,优化后\n");
    fprintf(out, "%.12f,%.12f\n", result->base_asai, result->optimized_asai);
    fprintf(out, "\n年平均减少停电时间(小时)\n");
    fprintf(out, "%.6f\n", result->reduced_outage_hours_per_year);
    fclose(out);
    return RCD_OK;
}
