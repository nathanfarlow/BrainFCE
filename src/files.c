#include "files.h"

#include <stdlib.h>
#include <string.h>

#include <debug.h>

void list_Create(FileList_t *list) {

    uint16_t i = 0;

    char **temp_arr;

    char *var_name;
    uint8_t *search_pos = NULL;

    list->amount = 0;
    while((var_name = ti_DetectVar(&search_pos, NULL, TI_PRGM_TYPE)) != NULL) {
        list->amount++;
    }

    //to account for the ! and # programs
    list->amount -= 2;
    temp_arr = malloc(list->amount * sizeof(char*));

    search_pos = NULL;
    while((var_name = ti_DetectVar(&search_pos, NULL, TI_PRGM_TYPE)) != NULL) {

        if(strcmp(var_name, "!") != 0 && strcmp(var_name, "#") != 0) {
            char *tmp = (char*)malloc(strlen(var_name) + 1);

            strcpy(tmp, var_name);

            temp_arr[i] = tmp;

            i++;
        }
        
        
    }

    //sort temp_arr alphabetically and put in list->files
    list->files = malloc(list->amount * sizeof(char*));

    for(i = 0; i < list->amount; i++) {
        uint16_t j;
        uint16_t smallest = 0;
        for(j = 0; j < list->amount; j++) {
            if(strcmp(temp_arr[j], temp_arr[smallest]) < 0)
                smallest = j;
        }
        list->files[i] = temp_arr[smallest];
        free(temp_arr[smallest]);
        temp_arr[smallest] = NULL;
    }

    free(temp_arr);
}

void list_Cleanup(FileList_t *list) {
    uint16_t i = 0;
    for(i = 0; i < list->amount; i++)
        free(list->files[i]);
    free(list->files);
}

ti_var_t file_SaveName(const char *data, uint32_t length, const char *name, int *error) {
    ti_var_t app_var;

    ti_CloseAll();

    app_var = ti_Open(name, "w");
    if(!app_var)
        *error = -1;
    else
        return file_SaveVar(data, length, app_var, error);

    return app_var;
}

ti_var_t file_SaveVar(const char *data, uint32_t length, ti_var_t var, int *error) {

    *error = ti_Write(data, length, 1, var);
    
    if(ti_SetArchiveStatus(true, var) == 0) {
        *error = -1;
        return var;
    }

    ti_CloseAll();

    return var;
}