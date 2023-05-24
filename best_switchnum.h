#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "main_minimal_path.h"
#include "util.h"

#include "initpop.h"
#include "mutation.h"
#include "selection.h"
#include "crossover.h"
#include "cal_objvalue.h"



int best_switchnum(const char * dir,vector2d* line, int line_num, vector2d* load, int load_num, int node_num, vector* branch_node_index) {
    if(branch_node_index->size>0){
        for(int i=0;i<branch_node_index->size;i++){
            int in = branch_node_index->data[i];
            line->data[in][5] = 2;
        }
    }

    int popsize = 20;
    vector index_main_feeder;
    vector_init(&index_main_feeder);
    for (int i = 0; i < line->rows; ++i) {
        if ((int)line->data[i][5] == 0 && (int)line->data[i][6] == 0 && (int)line->data[i][8] == 1) {
            vector_append(&index_main_feeder,i);
        }
    }
    int branchnumber = index_main_feeder.size;
    int max_switch=25;

    for(int i=0;i<branchnumber;i++){
        printf("%g,",index_main_feeder.data[i]);
    }
    printf("\n");
    //printf("%d\n",branchnumber);

    //交叉概率
    double pc = 0.6;
    //%变异概率
    double pm = 0.001;
    //%最大迭代次数
    int iter = 30;

    if (branchnumber<max_switch){
        max_switch=branchnumber;
    }

    vector final_fit;
    vector_resize(&final_fit,max_switch);
    for(int i=0;i<max_switch;i++){
        final_fit.data[i] = 0;
    }


    vector2d final_pop;
    vector2d_init(&final_pop,max_switch,branchnumber);
    for(int i=0;i<max_switch;i++){
        for(int j=0;j<branchnumber;j++){
            final_pop.data[i][j] = 0;
        }
    }

    for (int n = 1; n <= max_switch; n++) {
        printf("max_switch%d:%d\n",max_switch,n);
        vector2d *pop = initpop(popsize,branchnumber,n);
        vector2d best_pop;
        vector2d_init(&best_pop,iter,branchnumber);
        vector best_fit;
        vector_resize(&best_fit,iter);
        for (int i = 0; i < iter; i++) {
            printf("best_switch_iter:%d\n",i);
            vector *objvalue = cal_objvalue(pop,node_num,line_num,line,load_num,load,&index_main_feeder);
            best_fit.data[i] = objvalue->data[0];
            int pos = 0;
            for(int j=1;j<objvalue->size;j++){
                if(objvalue->data[j]>best_fit.data[i]){
                    best_fit.data[i] = objvalue->data[j];
                    pos = j;
                }
            }
            for(int j=0;j<best_pop.cols;j++){
                best_pop.data[i][j] = pop->data[pos][j];
            }

            vector2d* newpop = selection(pop, objvalue); // 选择
            newpop = crossover(newpop, pc, n); // 交叉
            newpop = mutation(newpop, pm); // 变异
            pop = newpop;
            free(objvalue->data);
            free(objvalue);
        }

        final_fit.data[n - 1] = best_fit.data[0];
        int pos = 0;
        for(int j=1;j<best_fit.size;j++){
            if(best_fit.data[j]>final_fit.data[n - 1]){
                final_fit.data[n - 1] = best_fit.data[j];
                pos = j;
            }
        }

        for(int j=0;j<final_pop.cols;j++){
            final_pop.data[n - 1][j] = best_pop.data[pos][j];
        }
        vector2d_free(&best_pop);
        vector_free(&best_fit);
    }




    for(int i=0;i< final_fit.size;i++){
        printf("%g,",final_fit.data[i]);
    }
    printf("\n");

    vector x;
    vector_resize(&x,branchnumber);

    for (int i = 0; i < branchnumber; i++) {
        x.data[i] = i + 1;
        //新增
        x.data[i] += branch_node_index->size;
        //新增
    }



    char file[20] = "draw.csv";
    char filepath1[100]= {'\0'};
    strcat(filepath1,dir);
    strcat(filepath1,file);
    printf("%s\n",filepath1);

    FILE *fp = fopen(filepath1, "w");

    fputs("x, y\n", fp);
    //画图”x-final_fit“

    for(int i=0;i< final_fit.size;i++){
        fprintf(fp, "%g,%g\n", x.data[i],final_fit.data[i]);
    }
    fclose(fp);

    vector ASAI_diff;
    vector_resize(&ASAI_diff,final_fit.size-1);

    for (int i = 0; i < ASAI_diff.size; i++) {
        ASAI_diff.data[i] = final_fit.data[i + 1] - final_fit.data[i];
        printf("%g,",ASAI_diff.data[i]);
    }
    printf("\n");


    int index = 0;
    while (index<ASAI_diff.size && fabs(ASAI_diff.data[index]) > 2e-07){
        index++;
    }

    int switch_num = index+1;

    if (switch_num == max_switch) {
        double max_num=0;
        for (int i = 0; i < final_fit.size; i++) {
            if (final_fit.data[i]>max_num){
                max_num=final_fit.data[i];
                switch_num=i+1;
                printf("switch_num == max_switch:%d\n",switch_num);
            }
        }
    }

    return switch_num;

}
