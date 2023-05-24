
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"



vector2d* selection(vector2d* pop, vector* fitvalue) {
    int px = pop->rows;
    int py = pop->cols;
    
    double totalfit = 0;
    for (size_t i = 0; i < fitvalue->size; i++) {
        totalfit += fitvalue->data[i];   // 计算总的适应度
    }
    double *p_fitvalue = (double*)malloc(sizeof(double) * fitvalue->size); // 按概率选择
    for (size_t i = 0; i < fitvalue->size; i++) {
        p_fitvalue[i] = fitvalue->data[i] / totalfit;
    }
    for (size_t i = 1; i < fitvalue->size; i++) {
        p_fitvalue[i] += p_fitvalue[i - 1];
    }
    
    double *ms = malloc(sizeof(double) * px);
    for (size_t i = 0; i < px; i++) {
        ms[i] = (double)rand() / RAND_MAX;
    }
    selectionSort(ms, px);

    vector2d *newpop = malloc(sizeof(vector2d));
    vector2d_init(newpop, px, py);
    for (int i = 0; i < px; i++) {
        for (int j = 0; j < py; j++) {
            newpop->data[i][j] = 0;
        }
    }
    
    int fitin = 0;
    int newin = 0;
    while (newin < px) {
        if (ms[newin] < p_fitvalue[fitin]) {
			 for(int j=0;j<py;j++){
                
				 newpop->data[newin][j] = pop->data[fitin][j];
			 }

			 newin++;
        }
        else {
            fitin++;
        }
    }
    free(ms);
    free(p_fitvalue);
    vector2d_free(pop);
    
    return newpop;
}
