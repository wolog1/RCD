#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "util.h"



const double INF = 1000000000;

vector2d *digraph(const vector2d *line,int line_num,int node_num, double *index){
    vector2d* G = malloc(sizeof(vector2d));
    vector2d_init(G,node_num,node_num);
    for(int i=0;i<node_num;i++){
        for (int j = 0; j < node_num; ++j) {
            G->data[i][j] = INF;
        }
    }
    for(int i=0;i<line->rows;i++){
        int x1 = line->data[i][0];
        int x2 = line->data[i][1];
        index[x2]+=1;
        G->data[x1][x2] = line->data[i][2];
    }


    return G;
}

void DFSPrint(double s, double v, vector* pre, vector* path)
{
    if (v == s) {
        vector_append(path,s);
        return;
    }
    int v_ = v;
    DFSPrint(s, pre->data[v_], pre, path);
    vector_append(path,v);
}

void Dijkstra(int n, double s, vector2d* G, vector* vis, vector* d, vector* pre)
{
    for(int i=0;i<vis->size;i++){
        vis->data[i] = 0;
    }

    for(int i=0;i<d->size;i++){
        d->data[i] = INF;
    }


    for (int i = 0; i < n; ++i)                            //新添加
        pre->data[i] = i;

    d->data[(int)s] = 0;                                              //起点s到达自身的距离为0
    for (int i = 0; i < n; ++i)
    {
        int u = -1;                                     //找到d[u]最小的u
        double MIN = INF;                                  //记录最小的d[u]
        for (int j = 0; j < n; ++j)                     //开始寻找最小的d[u]
        {
            if (vis->data[j] == 0 && d->data[j] < MIN)
            {
                u = j;
                MIN = d->data[j];
            }
        }
        //找不到小于INF的d[u]，说明剩下的顶点和起点s不连通
        if (u == -1)
            return;
        vis->data[u] = 1;                                  //标记u已被访问
        for (int v = 0; v < n; ++v)
        {
            //遍历所有顶点，如果v未被访问&&u能够到达v&&以u为中介点可以使d[v]更优
            if (vis->data[v] == 0 && d->data[u] + G->data[u][v] < d->data[v]) {
                d->data[v] = d->data[u] + G->data[u][v];             //更新d[v]
                pre->data[v] = u;                        //记录v的前驱顶点为u（新添加）
            }
        }
    }
}


vector* getpath(int node_num,double source, vector2d* g, double des){
    vector vis;
    vector_resize(&vis,node_num);

    vector d;
    vector_resize(&d,node_num);
    vector pre;
    vector_resize(&pre,node_num);

    Dijkstra(node_num,source,g,&vis,&d,&pre);



    vector *path = malloc(sizeof(vector));
    vector_init(path);
    DFSPrint(source,des,&pre,path);

    vector_free(&vis);
    vector_free(&d);
    vector_free(&pre);
    return path;
}


double main_minimal_path(int node_num, int line_num, const vector2d* line, int load_num, const vector2d* load) {
    double *index = malloc(sizeof(double)*node_num);
    memset(index,0,sizeof(double)*node_num);

    vector2d *g = digraph(line,line_num,node_num,index);
    double *indeg = malloc(sizeof(double)*node_num);

    for(int i=0;i<node_num;i++){
        indeg[i] = index[i];
    }
    free(index);
    vector source_node;
    vector_init(&source_node);
    for(int i=0;i<node_num;i++){
        if((int)indeg[i]==0){
            vector_append(&source_node,i);
        }
    }

    double *Lambda_load = malloc(sizeof(double)*load_num);
    memset(Lambda_load,0,sizeof(double)*load_num);

    double *Gamma_load = malloc(sizeof(double)*load_num);
    memset(Gamma_load,0,sizeof(double)*load_num);

    double *load_users = malloc(sizeof(double)*load_num);
    memset(load_users,0,sizeof(double)*load_num);

    double *U_load = malloc(sizeof(double)*load_num);
    memset(U_load,0,sizeof(double)*load_num);




    for(int n=0;n<source_node.size;n++) {
        for (int i = 0; i < load_num; i++) {
            vector2d *line_copy = vector2d_copy(line);
            vector *path = getpath(node_num, source_node.data[n], g, load->data[i][5]);


            vector equipment, breaker, operator_;
            vector_init(&equipment);
            vector_init(&breaker);
            vector_init(&operator_);

            int_set mmset;
            int_set_init(&mmset);
            for (int k = 0; k < path->size - 1; k++) {
                vector tmp;
                vector_init(&tmp);
                for (int j = 0; j < line->rows; ++j) {
                    //printf("%g,%g,%g,%g\n",line->data[j][0],path->data[k],line->data[j][1],path->data[k + 1]);
                    if (line->data[j][0] == path->data[k] && line->data[j][1] == path->data[k + 1]) {
                        //printf("abk\n");
                        vector_append(&tmp, j);
                        int_set_insert(&mmset, j);
                    }
                }
                for (int j = 0; j < tmp.size; j++) {
                    vector_append(&equipment, tmp.data[j]);
                }
            }
            if (path->size - 1 != equipment.size) {
                //printf("path.size()-1 != equipment.size()\n");
            }
            int idn = 0;
            for (int j = 0; j < line_copy->rows; j++) {
                if (int_set_get(&mmset, j) > 0) {
                    continue;
                }
                line_copy->data[idn++] = line_copy->data[j];
            }
            line_copy->rows = idn;


            for (int m = 0; m < line_copy->rows; m++) {
                vector *path2 = getpath(node_num, source_node.data[n], g, line_copy->data[m][1]);
                vector equipment1;
                vector_init(&equipment1);
                for (int k = 0; k < path2->size - 1; k++) {
                    for (int c = 0; c < line_copy->rows; c++) {
                        if (line_copy->data[c][0] == path2->data[k] && line_copy->data[c][1] == path2->data[k + 1]) {
                            vector_append(&equipment1, line_copy->data[c][9] - 1);
                        }
                    }
                }
                int c1 = 0;
                for (int k = 0; k < equipment1.size; k++) {
                    int e = equipment1.data[k];
                    if (line->data[e][5] == 3 || line->data[e][5] == 2 || line->data[e][5] == 1) {
                        c1++;
                    }
                }

                int c2 = 0;
                for (int k = 0; k < equipment1.size; k++) {
                    int e = equipment1.data[k];
                    if (line->data[e][5] == 1) {
                        c2++;
                    }
                }

                int c3 = 0;
                for (int k = 0; k < equipment1.size; k++) {
                    int e = equipment1.data[k];
                    if (line->data[e][5] == 3 || line->data[e][5] == 2) {
                        c3++;
                    }
                }
                if (c1 == 0) {
                    vector_append(&equipment, line_copy->data[m][9] - 1);
                } else if (c2 > 0 && c3 == 0) {
                    vector_append(&operator_, line_copy->data[m][9] - 1);
                }
                vector_free(path2);
                vector_free(&equipment1);
            }

            for (int k = 0; k < equipment.size; k++) {
                int e = equipment.data[k];
                Lambda_load[i] += line->data[e][2] * line->data[e][3];
                Gamma_load[i] += line->data[e][2] * line->data[e][3] * line->data[e][4];
            }
            for (int k = 0; k < operator_.size; k++) {
                int o = operator_.data[k];
                Lambda_load[i] += line->data[o][2] * line->data[o][3];
                Gamma_load[i] += line->data[o][2] * line->data[o][3] * line->data[o][7];
            }

            Gamma_load[i] /= Lambda_load[i];
            U_load[i] = Gamma_load[i] * Lambda_load[i];
            load_users[i] = load->data[i][4];


            vector_free(&equipment);
            vector_free(&breaker);
            vector_free(&operator_);
            vector_free(path);
            vector2d_free(line_copy);
        }


    }
    vector_free(&source_node);

    double inner = 0;
    for(int i=0;i<load_num;i++){
        inner += (Lambda_load[i]*load_users[i]);
    }


    double SAIFI = inner / accumulate(load_users,load_num);

    int idx = 0;
    for (int i = 0; i < load_num; ++i) {
        if(!isnan(U_load[i])){
            U_load[idx] = U_load[i];
            load_users[idx] = load_users[i];
            idx++;
        }
    }

    inner = 0;
    for(int i=0;i<idx;i++){
        inner += (U_load[i]*load_users[i]);
    }



    double SAIDI = inner / accumulate(load_users,idx);
    double CAIDI = SAIDI / SAIFI;
    double ASAI = 1 - SAIDI / 8760.0;
    free(Lambda_load);
    free(Gamma_load);
    free(load_users);
    free(U_load);
    return ASAI;
}





