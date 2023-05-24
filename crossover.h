
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "util.h"


vector2d* crossover(vector2d *pop, double pc, int n) {
    int px = pop->rows;
    int py = pop->cols;
    vector2d *newpop = malloc(sizeof(vector2d));
    vector2d_init(newpop,px,py);
    for(int i=0;i<px;i++){
        for(int j=0;j<py;j++){
            newpop->data[i][j] = 0;
        }
    }



    for (int i = 0; i < px-1; i += 2) {
        if ((double) rand() / RAND_MAX < pc) {
            int cpoint = round((double) rand() / RAND_MAX * py);

            double *x1 = malloc(sizeof(double)*cpoint);
            for(int j=0;j<cpoint;j++){
                x1[j] = pop->data[i][j];
            }
            double *x2 = malloc(sizeof(double)*(py-cpoint));
            for(int j=0;j<py-cpoint;j++){
                x2[j] = pop->data[i+1][j+cpoint];
            }
            double *y1 = malloc(sizeof(double)*cpoint);
            for(int j=0;j<cpoint;j++){
                y1[j] = pop->data[i+1][j];
            }
            double *y2 = malloc(sizeof(double)*(py-cpoint));
            for(int j=0;j<py-cpoint;j++){
                y2[j] = pop->data[i][j+cpoint];
            }

            int div = accumulate(x1,cpoint)+ accumulate(x2,py-cpoint) - n;
            if(div !=0 ){
                int condi = (div>0);
                vector temp_index;
                vector_init(&temp_index);
                //printf("%p\n",&temp_index);
                for (int j = 0; j < (py-cpoint); j++) {
                    if (x2[j] == condi) {
                        vector_append(&temp_index,j);
                    }
                }
                shuffled(temp_index.data,temp_index.size);
                for (int j = 0; j < min(abs(div),temp_index.size); j++) {
                    int b = temp_index.data[j];
                    x2[b] = abs(condi-1);
                }
                vector_free(&temp_index);

                for(int j = 0;j<cpoint;j++){
                   newpop->data[i][j] = x1[j];
                }
                for(int j = cpoint;j<py-cpoint;j++){
                    newpop->data[i][j] = x2[j];
                }


                div = accumulate(y1, cpoint) + accumulate(y2, py-cpoint) - n;
                if (div != 0) {
                    int condi2 = (div > 0);
                    vector temp_index2;
                    vector_init(&temp_index2);
                    for (int j = 0; j < (py-cpoint); j++) {
                        if (y2[j] == condi2) {
                            vector_append(&temp_index2,j);
                        }
                    }
                    shuffled(temp_index2.data,temp_index2.size);
                    for (int j = 0; j < min(abs(div),temp_index.size); j++) {
                        int b = temp_index2.data[j];
                        y2[b] = abs(condi2-1);
                    }
                    vector_free(&temp_index2);
                }

                for(int j = 0;j<cpoint;j++){
                    newpop->data[i+1][j] = y1[j];
                }
                for(int j = cpoint;j<py-cpoint;j++){
                    newpop->data[i+1][j] = y2[j];
                }
            }
            free(x1);
            free(x2);
            free(y1);
            free(y2);
        } else{
            for(int j=0;j<py;j++){
                newpop->data[i][j] = pop->data[i][j];
                newpop->data[i+1][j] = pop->data[i+1][j];
            }
        }
    }
    vector2d_free(pop);
    return newpop;
}