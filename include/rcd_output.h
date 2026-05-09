#ifndef RCD_OUTPUT_H
#define RCD_OUTPUT_H

#include "rcd_ga.h"
#include "rcd_types.h"

int rcd_write_outputs(const char *output_dir,
                      const RcdStringSet *node_names,
                      const RcdMatrix *line,
                      const RcdOptimizationResult *result);

#endif
