#pragma once

typedef struct file_lines {
        unsigned short length;
        unsigned short cap;
        char** line;
} file_lines;

void add_line(file_lines* array, const char* string);

file_lines* new_file_lines();

void free_file_lines(file_lines* array);
