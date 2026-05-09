# RCD

RCD is a C implementation for reliability-oriented distribution-network switch placement optimization.

The program reads feeder line data and load data, evaluates reliability metrics such as ASAI, and uses a genetic algorithm to recommend the number and positions of additional switches.

## Input files

Place the following files in one data directory:

### Sheet1.csv

Line/equipment data. Expected columns:

```csv
首端节点,末端节点,线路长度(km),故障率/(km·次),修复时间(小时),首端开关,末端开关,切换时间,线路类型,线路编号
```

### Sheet2.csv

Load/customer data. Expected columns:

```csv
编号,用户类型,峰值负荷,平均负荷MW,用户数,连接节点
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/rcd <data_dir> [output_dir]
```

Example:

```bash
./build/rcd ./data ./out
```

If `output_dir` is omitted, output files are written into `data_dir`.

## Output files

- `draw.csv`: switch count versus ASAI curve.
- `output.csv`: selected switch positions, installation priority, ASAI before/after, and reduced annual outage hours.

## Refactored architecture

```text
include/
  rcd_types.h        Dynamic vectors, matrices, string sets, dataset model
  rcd_csv.h          CSV input API
  rcd_reliability.h  SAIFI/SAIDI/CAIDI/ASAI calculation API
  rcd_ga.h           Genetic algorithm optimization API
  rcd_output.h       CSV output API
src/
  main.c             CLI entrypoint
  rcd_types.c        Shared containers and utilities
  rcd_csv.c          Sheet1/Sheet2 parser
  rcd_reliability.c  Reliability calculation model
  rcd_ga.c           Switch optimization logic
  rcd_output.c       Output writer
```

## Reliability metrics

The core optimization objective is to maximize ASAI:

```text
SAIFI = Σ(λ_i × N_i) / ΣN_i
SAIDI = Σ(U_i × N_i) / ΣN_i
CAIDI = SAIDI / SAIFI
ASAI  = 1 - SAIDI / 8760
```

## Notes

- Source files are expected to be UTF-8.
- Random seeding is performed once in `main.c`.
- The previous hard-coded data path has been removed.
- The genetic crossover operator now repairs chromosome cardinality after crossover, keeping the selected switch count stable.
