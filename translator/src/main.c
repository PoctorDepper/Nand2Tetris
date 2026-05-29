#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_lines.h"

unsigned short validate_i(const char* value)
{
        char* end;
        const unsigned short number = strtol(value, &end, 10);
        if (*end && !isspace(*end))
                return 0;
        return number;
}

void push_segment(file_lines* lines, char* region, short i, char* buffer)
{
        // Calculate segment pointer + i
        sprintf(buffer, "@%d", i);
        add_line(lines, buffer);
        add_line(lines, "D=A");
        add_line(lines, region);
        add_line(lines, "A=D+M");
        // Grab value at segment + i
        add_line(lines, "D=M");
}

void push_static(file_lines* lines, short address, char* buffer)
{
        sprintf(buffer, "@%d", address);
        add_line(lines, buffer);
        add_line(lines, "D=M");
}

void push(file_lines* lines, const file_lines* arguments, char* buffer)
{
        const unsigned short i = validate_i(arguments->line[2]);
        if (!strcmp("local", arguments->line[1]))
                push_segment(lines, "@LCL", i, buffer);
        else if (!strcmp("argument", arguments->line[1]))
                push_segment(lines, "@ARG", i, buffer);
        else if (!strcmp("this", arguments->line[1]))
                push_segment(lines, "@THIS", i, buffer);
        else if (!strcmp("that", arguments->line[1]))
                push_segment(lines, "@THAT", i, buffer);
        else if (!strcmp("static", arguments->line[1]))
                push_static(lines, i + 16, buffer);
        else if (!strcmp("temp", arguments->line[1]))
                push_static(lines, i + 5, buffer);
        else if (!strcmp("pointer", arguments->line[1]))
                push_static(lines, i + 3, buffer);
        else {
                sprintf(buffer, "@%d", i);
                add_line(lines, buffer);
                add_line(lines, "D=A");
        }
        // Increment SP and go to previous location
        add_line(lines, "@SP");
        add_line(lines, "AM=M+1");
        add_line(lines, "A=A-1");
        // Push the value to the top of the stack
        add_line(lines, "M=D");
}

void pop_segment(file_lines* lines, char* region, short i, char* buffer)
{
        // get RAM[segment + i]
        sprintf(buffer, "@%d", i);
        add_line(lines, buffer);
        add_line(lines, "D=A");
        add_line(lines, region);
        add_line(lines, "D=D+M");
        // go to stack and decrement pointer
        add_line(lines, "@SP");
        add_line(lines, "AM=M-1");
        // cycle A, D, and M left
        add_line(lines, "M=D+M");
        add_line(lines, "D=M-D");
        add_line(lines, "A=M-D");
}

void pop_static(file_lines* lines, short address, char* buffer)
{
        // go to stack and decrement pointer
        add_line(lines, "@SP");
        add_line(lines, "AM=M-1");
        // grab RAM[SP]
        add_line(lines, "D=M");
        // push into RAM[segment + i]
        sprintf(buffer, "@%d", address);
        add_line(lines, buffer);
}

void pop(file_lines* lines, file_lines* arguments, char* buffer)
{
        const unsigned short i = validate_i(arguments->line[2]);
        if (!strcmp("local", arguments->line[1]))
                pop_segment(lines, "@LCL", i, buffer);
        else if (!strcmp("argument", arguments->line[1]))
                pop_segment(lines, "@ARG", i, buffer);
        else if (!strcmp("this", arguments->line[1]))
                pop_segment(lines, "@THIS", i, buffer);
        else if (!strcmp("that", arguments->line[1]))
                pop_segment(lines, "@THAT", i, buffer);
        else if (!strcmp("static", arguments->line[1]))
                pop_static(lines, i + 16, buffer);
        else if (!strcmp("temp", arguments->line[1]))
                pop_static(lines, i + 5, buffer);
        else if (!strcmp("pointer", arguments->line[1]))
                pop_static(lines, i + 3, buffer);
        // RAM[segment + i] <- RAM[SP]
        add_line(lines, "M=D");
}

// add, sub, and, or
void double_arithmetic(file_lines* lines, const char operation, char* buffer)
{
        // decrement and grab the stack pointer
        add_line(lines, "@SP");
        add_line(lines, "AM=M-1");
        // grab 'y'
        add_line(lines, "D=M");
        // go to 'x' and overwrite with 'x[op]y'
        add_line(lines, "A=A-1");
        sprintf(buffer, "MD=M%cD", operation);
        add_line(lines, buffer);
}

// neg, not
void single_arithmetic(file_lines* lines, const char operation, char* buffer)
{
        // grab stack pointer - 1
        add_line(lines, "@SP");
        add_line(lines, "A=M-1");
        // replace 'x' with '[op]x'
        sprintf(buffer, "M=%cM", operation);
        add_line(lines, buffer);
}

// eq, gt, lt
void stack_evaluation(file_lines* lines, const char* jump, short i, char* buffer)
{
        // subtraction is great for a difference check
        double_arithmetic(lines, '-', buffer);
        // D and A come primed from double_arithmetic
        add_line(lines, "M=0");
        sprintf(buffer, "@EVAL%d", i);
        add_line(lines, buffer);
        sprintf(buffer, "D;%s", jump);
        add_line(lines, buffer);
        // True
        add_line(lines, "@SP");
        add_line(lines, "A=M-1");
        add_line(lines, "M=-1");
        // False
        sprintf(buffer, "(EVAL%d)", i);
        add_line(lines, buffer);
}

int main(const int argc, const char* argv[])
{
        if (argc <= 1) {
                fprintf(stderr, "You must pass a file.\n");
                return 1;
        }
        const unsigned short filepath_length = strlen(argv[1]);
        char filepath[filepath_length + 2];
        char* extension = strcpy(filepath, argv[1]) + filepath_length - 3;
        if (!extension || strcmp(extension, ".vm")) {
                fprintf(stderr, "%s is not a VM file.\n", filepath);
                return 1;
        }
        FILE* in = fopen(filepath, "r");
        if (!in) {
                fprintf(stderr, "%s couldn't be opened.\n", filepath);
                return 1;
        }
        file_lines* vm = new_file_lines();
        char line_in[512];
        while (fgets(line_in, 512, in)) {
                const char* line_ptr = line_in;
                while (isspace(*line_ptr))
                        line_ptr++;
                if (strchr("/\r\n\0", *line_ptr))
                        continue;
                add_line(vm, line_ptr);
        }
        fclose(in);
        file_lines* assembly = new_file_lines();
        char instruction_buffer[12];
        for (short i = 0; i < vm->length; ++i) {
                // Get all line arguments
                file_lines* arguments = new_file_lines();
                const char* argument_ptr = strtok(vm->line[i], " \r\n");
                while (argument_ptr) {
                        add_line(arguments, argument_ptr);
                        argument_ptr = strtok(NULL, " \r\n");
                }
                if (!strcmp(arguments->line[0], "push"))
                        push(assembly, arguments, instruction_buffer);
                else if (!strcmp(arguments->line[0], "pop"))
                        pop(assembly, arguments, instruction_buffer);
                else if (!strcmp(arguments->line[0], "add"))
                        double_arithmetic(assembly, '+', instruction_buffer);
                else if (!strcmp(arguments->line[0], "sub"))
                        double_arithmetic(assembly, '-', instruction_buffer);
                else if (!strcmp(arguments->line[0], "neg"))
                        single_arithmetic(assembly, '-', instruction_buffer);
                else if (!strcmp(arguments->line[0], "eq"))
                        stack_evaluation(assembly, "JNE", i, instruction_buffer);
                else if (!strcmp(arguments->line[0], "gt"))
                        stack_evaluation(assembly, "JLE", i, instruction_buffer);
                else if (!strcmp(arguments->line[0], "lt"))
                        stack_evaluation(assembly, "JGE", i, instruction_buffer);
                else if (!strcmp(arguments->line[0], "and"))
                        double_arithmetic(assembly, '&', instruction_buffer);
                else if (!strcmp(arguments->line[0], "or"))
                        double_arithmetic(assembly, '|', instruction_buffer);
                else if (!strcmp(arguments->line[0], "not"))
                        single_arithmetic(assembly, '!', instruction_buffer);
                free_file_lines(arguments);
        }
        free_file_lines(vm);
        strncpy(extension, ".asm\0", 5);
        FILE* out = fopen(filepath, "w");
        if (!out) {
                fprintf(stderr, "%s couldn't be written to.\n", filepath);
                free_file_lines(assembly);
                return 1;
        }
        char** out_ptr = assembly->line;
        char** out_end = assembly->line + assembly->length - 1;
        while (out_ptr != out_end)
                fprintf(out, "%s\n", *out_ptr++);
        fprintf(out, "%s", *out_ptr);
        fclose(out);
        free_file_lines(assembly);
        return 0;
}