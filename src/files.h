#ifndef FILES_H_
#define FILES_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
	char **files;
	uint16_t amount;
} FileList;

char **get_file_list();
char *read_file();

#endif