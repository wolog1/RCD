#include <stdio.h>
#include "util.h"
#include "read_data.h"
#include "best_switchnum.h"
#include "certain_switchnum.h"
double quantile(vector* x, double k) {
    double *tmp = malloc(sizeof(double)*x->size);

    for(int i=0;i<x->size;i++){
        tmp[i] = x->data[i];
    }

    selectionSort(tmp,x->size);

    int index = (int)(x->size*k)-1;
    double res = tmp[index];
    free(tmp);
    return res;
}


int main() {
    //printf("Hello, World!\n");


    char *dir = "/home/book/matlab_c/matlab_c/matlab_v3/";

    string_set name_set;
    string_set_init(&name_set);
    vector2d line;
    vector2d_init(&line,0,0);
    vector2d load;
    vector2d_init(&load,0,0);

    read_csv_file(dir, &name_set, &line, &load);
    vector temp;
    vector_init(&temp);
    for(int i=0;i<load.rows;i++){
        vector_append(&temp,load.data[i][2]);
    }
    double Q1 = quantile(&temp,0.25); // Q1
    double Q3 = quantile(&temp,0.75); // Q3
    double IQR = Q3 - Q1;
    vector load_index;
    vector_init(&load_index);

    for (int i=0;i<load.rows;i++) {
        if (load.data[i][2] > (Q3 + 1.5 * IQR)) {
            //printf("%d\n",i);
            vector_append(&load_index,load.data[i][load.cols-1]);
        }
    }
    //printf("load_index:%d\n",load_index.size);
    //cout<<"load_index:"<<load_index.size()<<endl;
    vector branch_node_index;
    vector_init(&branch_node_index);
    if(load_index.size>0){
        for(int i=0;i<load_index.size;i++){
            vector tmp;
            vector_init(&tmp);
            for(int j=0;j<line.rows;j++){
                if((int)line.data[j][1]==(int)load_index.data[i] && (int)line.data[j][5]==0 && (int)line.data[j][6]==0 && (int)line.data[j][9]==2){
                    vector_append(&tmp,line.data[j][line.cols-1]);
                }
            }
            if(tmp.size>0){
                vector_append(&branch_node_index,tmp.data[0]);
            }
            vector_free(&tmp);
        }
    }
    printf("branch feeders need to install:%ld\n",branch_node_index.size);

    vector2d *line_copy = vector2d_copy(&line);
    vector2d *load_copy = vector2d_copy(&load);
    int switch_num;
    switch_num=best_switchnum(dir,&line,line.rows,&load,load.rows,name_set.size,&branch_node_index);
    printf("switch_num:%d\n",switch_num);

    certain_switchnum(dir,&name_set,line_copy,line.rows,load_copy,load.rows,switch_num,&branch_node_index);




    return 0;
}
