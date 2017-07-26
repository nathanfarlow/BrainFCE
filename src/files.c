#include "files.h"

#include <stdlib.h>

void list_Create(FileList_t *list) {

	uint8_t *search_pos = NULL;
    char *var_name;
    const char search_string[] = {0xBF, 0xBA, 0xBE};

    uint16_t index = 0;

    list->amount = 0;
    while((var_name = ti_Detect(&search_pos, search_string)) != NULL) {
    	list->amount++;
    }

    list->files = malloc(list->amount * sizeof(char*));

    search_pos = NULL;
    while((var_name = ti_Detect(&search_pos, search_string)) != NULL) {
    	list->files[index++] = var_name;
    }

}

void list_Cleanup(FileList_t *list) {
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