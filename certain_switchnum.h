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

void certain_switchnum(const char* dir,string_set* node_name, vector2d* line, int line_num, vector2d* load, int load_num, int switch_num, vector* branch_node_index) {

    double ASAI = main_minimal_path(node_name->size, line_num, line, load_num, load);
    vector2d *preline = vector2d_copy(line);
    if(branch_node_index->size>0){
        for(int i=0;i<branch_node_index->size;i++){
            int in = branch_node_index->data[i];
            line->data[in][5] = 2;
        }
    }
    int popsize = 30;
    vector index_main_feeder;
    vector_init(&index_main_feeder);
    for (int i = 0; i < line->rows; ++i) {
        if ((int)line->data[i][5] == 0 && (int)line->data[i][6] == 0 && (int)line->data[i][8] == 1) {
            vector_append(&index_main_feeder,i);
        }
    }

    int branchnumber = index_main_feeder.size;
    //äş¤ĺćŚç
    double pc = 0.6;
    //%ĺĺźćŚç
    double pm = 0.001;
    //%ćĺ¤§čż­äťŁćŹĄć°
    int iter = 50;

    int n = switch_num;

    vector2d *pop = initpop(popsize,branchnumber,n);
    vector2d best_pop;
    vector2d_init(&best_pop,iter,branchnumber);
    vector best_fit;
    vector_resize(&best_fit,iter);

    for (int i = 0; i < iter; i++) {
        printf("certain_switch_iter:%d\n",i);
        vector *objvalue = cal_objvalue(pop,node_name->size,line_num,line,load_num,load,&index_main_feeder);


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

        vector2d* newpop = selection(pop, objvalue);
        newpop = crossover(newpop, pc, n);
        newpop = mutation(newpop, pm);
        pop = newpop;

    }

    double max_final_fit = best_fit.data[0];
    int pos = 0;
    for(int j=1;j<best_fit.size;j++){
        if(best_fit.data[j]>max_final_fit){
            max_final_fit = best_fit.data[j];
            pos = j;
        }
    }

    vector final_pop;
    vector_resize(&final_pop,best_pop.cols);
    for(int j=0;j<final_pop.size;j++){
        final_pop.data[j] = best_pop.data[pos][j];
    }

    double Time = 8760 * (max_final_fit - ASAI);

    vector index_number;
    vector_init(&index_number);
    for (int i = 0; i < branchnumber; i++)
    {
        if (final_pop.data[i] == 1)
        {
            vector_append(&index_number,index_main_feeder.data[i]);
        }
    }
    if(branch_node_index->size>0){
        line = preline;
        for(int i=0;i<branch_node_index->size;i++){
            vector_append(&index_number,branch_node_index->data[i]);
        }
    }

    string_set setting_position;
    //string_set_init(&setting_position);
    string_set_resize(&setting_position,index_number.size);
    for (int i = 0; i < index_number.size; i++)
    {
        setting_position.data[i] = node_name->data[(int)line->data[(int)index_number.data[i]][0]];
    }


    // čŽĄçŽĺźĺłĺŽčŁä˝ç˝Žçäźĺçş§
    vector switch_position;
    vector_resize(&switch_position,index_number.size);
    for(int i=0;i<index_number.size;i++){
        switch_position.data[i] = index_number.data[i];
    }

    int end_index = switch_position.size;
    vector switch_index;
    vector_init(&switch_index);
    for(int j=0; j<end_index; j++){
        vector ASAI_result;
        vector_init(&ASAI_result);
        for(int i=0; i<switch_position.size; i++){
            if(j==0){
                vector_append(&switch_index,switch_position.data[i]);
            } else{
                switch_index.data[j] = switch_position.data[i];
            }
            vector2d* new_line = vector2d_copy(line);
            for(int k=0; k<switch_index.size; k++){
                new_line->data[(int)switch_index.data[k]][5] = 2;
            }
            vector output_vec;
            vector_init(&output_vec);
            double temp = main_minimal_path(node_name->size,line_num,new_line,load_num,load);
            vector_append(&ASAI_result,temp);
        }
        int temp_index = 0;
        double max_val = ASAI_result.data[0];
        for(int j=1;j<ASAI_result.size;j++){
            if(ASAI_result.data[j]>max_val){
                temp_index = j;
                max_val = ASAI_result.data[j];
            }
        }
        switch_index.data[j] = switch_position.data[temp_index];

        for(int j=temp_index+1;j<switch_position.size;j++){
            switch_position.data[j-1] = switch_position.data[j];
        }
        switch_position.size -= 1;
    }
    string_set setting_sequence;
    string_set_resize(&setting_sequence,switch_index.size);
    //cout<<switch_index.size()<<endl;
    for(int i=0; i<switch_index.size; i++){
        setting_sequence.data[i] = node_name->data[(int)line->data[(int)switch_index.data[i]][0]];
    }

    char file[20] = "output.csv";
    char filepath[100]= {'\0'};
    strcat(filepath,dir);
    strcat(filepath,file);
    printf("%s\n",filepath);

    FILE *fp = fopen(filepath, "w");

    fputs("ĐÂÔöżŞšŘ:\n", fp);
    fputs("ĐňşĹ,°˛×°ÎťÖĂ\n", fp);
    //çťĺžâx-final_fitâ

    for(int i=0;i< setting_position.size;i++){
        fprintf(fp, "%d,%s\n",i+1,setting_position.data[i]);
    }


    fputs("\nÓĹĎČźś,°˛×°ÎťÖĂ\n", fp);

    for(int i=0;i< setting_sequence.size;i++){
        fprintf(fp, "%d,%s\n",i+1,setting_sequence.data[i]);
    }

    fputs("\nÓĹťŻÇ°şóľÄżÉżżĐÔÖ¸ąęASAI:\n", fp);
    fputs("ÓĹťŻÇ°,ÓĹťŻşó\n", fp);
    fprintf(fp, "%g,%g\n",ASAI,max_final_fit);

    fputs("\nÄęĆ˝žůźőÉŮÍŁľçĘąźä(ĐĄĘą)\n", fp);
    fprintf(fp, "%g\n",Time);
    fclose(fp);
}
