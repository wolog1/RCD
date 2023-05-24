#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"


vector2d* initpop(int popsize, int branchnumber, int n){
    vector2d *pop = malloc(sizeof(vector2d));
    vector2d_init(pop,popsize,branchnumber);

    vector origin_index;
    vector_resize(&origin_index,branchnumber);
    for(int i=0;i<origin_index.size;i++){
        origin_index.data[i] = 0;
    }

    for(size_t i=0;i<n;i++){
        origin_index.data[i] = 1;
    }

    for(size_t i = 0; i<popsize; i++){
        shuffled(origin_index.data,branchnumber);
        for(int j=0;j<branchnumber;j++){
            pop->data[i][j] = origin_index.data[j];
        }
    }
    vector_free(&origin_index);
    return pop;
}
