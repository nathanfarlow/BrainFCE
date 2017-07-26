#ifndef FILES_H_
#define FILES_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <fileioc.h>

typedef struct {
    char **files;
    uint16_t amount;
} FileList_t;

void list_Create(FileList_t *list);
void list_Cleanup(FileList_t *list);

ti_var_t file_SaveName(const char *data, uint32_t length, const char *name, int *error);
ti_var_t file_SaveVar(const char *data, uint32_t length, ti_var_t var, int *error);

#endif