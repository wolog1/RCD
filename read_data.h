#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#define BUFFER_SIZE 1024

void read_csv_file(const char * dir, string_set *name_set, vector2d* line, vector2d* load)
{
    char file1[20] = "Sheet1.csv";
    char filepath1[100]= {'\0'};
    strcat(filepath1,dir);
    strcat(filepath1,file1);
    printf("%s\n",filepath1);

    FILE *file = fopen(filepath1, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    size_t count = 0;
    while (fgets(buffer, BUFFER_SIZE, file)) {
        count++;
        if(count==1) continue;
        if (strlen(buffer) <= 10) break;
        size_t buffer_len = strlen(buffer);
        if (buffer[buffer_len - 1] == '\n') {
            buffer[buffer_len - 1] = ',';
        }
        vector *tmp = malloc(sizeof(vector));
        vector_init(tmp);

        char *token = strtok(buffer, ",");
        size_t index = 0;
        while (token != NULL) {
            index++;
            if(index==1 || index==2){
                string_set_insert(name_set, token);
                int name_id = string_set_get(name_set,token);
                vector_append(tmp,name_id);
                token = strtok(NULL, ",");
                continue;
            }
            char *p;
            double num= strtod(token, &p);
            vector_append(tmp,num);
            token = strtok(NULL, ",");
        }
        vector2d_append_row(line,tmp);
        memset(buffer,0,BUFFER_SIZE);
        free(tmp);
    }
    //vector2d_print(line);
    fclose(file);
    ////////////////////////////////////////////
    char file2[20] = "Sheet2.csv";
    char filepath2[100]= {'\0'};
    strcat(filepath2,dir);
    strcat(filepath2,file2);
    printf("%s\n",filepath2);

    file = fopen(filepath2, "r");
    if (file == NULL) {
        perror("Error opening file2");
        exit(EXIT_FAILURE);
    }

    count = 0;
    while (fgets(buffer, BUFFER_SIZE, file)) {
        count++;
        if(count==1) continue;
        if (strlen(buffer) <= 10) break;
        size_t buffer_len = strlen(buffer);
        if (buffer[buffer_len - 1] == '\n') {
            buffer[buffer_len - 1] = ',';
        }
        vector *tmp = malloc(sizeof(vector));
        vector_init(tmp);

        char *token = strtok(buffer, ",");
        size_t index = 0;
        while (token != NULL) {
            index++;
            if(index==6){
                int name_id = string_set_get(name_set,token);
                vector_append(tmp,name_id);
                token = strtok(NULL, ",");
                continue;
            }
            char *p;
            double num= strtod(token, &p);
            vector_append(tmp,num);
            token = strtok(NULL, ",");
        }
        vector2d_append_row(load,tmp);
        memset(buffer,0,BUFFER_SIZE);
        free(tmp);
    }
    //vector2d_print(load);
    fclose(file);

}
