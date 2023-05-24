#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"




vector2d* mutation(vector2d* pop, double pm){
    srand(time(NULL));
    int px = pop->rows;
    int py = pop->cols;

    vector2d *newpop = malloc(sizeof(vector2d));
    vector2d_init(newpop,px,py);
    for(int i=0;i<px;i++){
        for(int j=0;j<py;j++){
            newpop->data[i][j] = 0;
        }
    }

    for(int i = 0; i<px;i++){
        for(int j=0;j<py;j++){
            newpop->data[i][j] = pop->data[i][j];
        }

        if((double) rand() / RAND_MAX < pm){
            int *index = malloc(sizeof(index)*py);
            for (size_t j = 0; j < py; j++) {
                index[j] = j;
            }
            shuffle(index,py);
            newpop->data[i][index[0]] = pop->data[i][index[1]];
            newpop->data[i][index[1]] = pop->data[i][index[0]];
            free(index);
        }
    }
    vector2d_free(pop);
    return newpop;
}



