#ifndef RCD_RELIABILITY_H
#define RCD_RELIABILITY_H

#include "rcd_types.h"

typedef struct {
    double saifi;
    double saidi;
    double caidi;
    double asai;
} RcdReliabilityMetrics;

int rcd_calculate_reliability(const RcdMatrix *line,
                              const RcdMatrix *load,
                              size_t node_count,
                              RcdReliabilityMetrics *metrics);

double rcd_calculate_asai(const RcdMatrix *line,
                          const RcdMatrix *load,
                          size_t node_count);

#endif
