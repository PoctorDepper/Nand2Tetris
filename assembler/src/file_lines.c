#include "file_lines.h"
#include <stdlib.h>
#include <string.h>

void add_line(file_lines* array, const char* string)
{
        // Double the cap of the data structure and reallocate
        if (array->length == array->cap) {
                array->cap *= 2;
                array->line = realloc(array->line, array->cap * sizeof(char*));
        }
        // Duplicate the string into the array
        array->line[array->length++] = strdup(string);
}

file_lines* new_file_lines()
{
        const file_lines proto = {0, 10, malloc(10 * sizeof(char*))};
        file_lines *data = malloc(sizeof(file_lines));
        *data = proto;
        return data;
}

void free_file_lines(file_lines* array)
{
        for (unsigned short i = 0; i < array->length; ++i) {
                free(array->line[i]);
        }
        free(array->line);
        free(array);
}
