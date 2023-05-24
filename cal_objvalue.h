#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "main_minimal_path.h"
#include "util.h"

vector* cal_objvalue(vector2d* pop, int node_num,int line_num, vector2d* line, int load_num, vector2d* load, vector* index_main_feeder) {
    int px = pop->rows; 
    vector* objvalue =(vector*) malloc(sizeof(vector));
 //  printf("objvalue=%p\n",objvalue);
    vector_resize(objvalue,px);
  //  printf("objvalue-->data=%p\n",objvalue->data);
    for (int i = 0; i < px; i++) {
        for (int j = 0; j < index_main_feeder->size; j++) {
            int index = (int)index_main_feeder->data[j];
            line->data[index][5] = pop->data[i][j] * 2;  //i,index
        }
        double ASAI = 0;
        ASAI = main_minimal_path(node_num,line_num,line,load_num,load);
        objvalue->data[i] = ASAI;
    }
    return objvalue;
}
