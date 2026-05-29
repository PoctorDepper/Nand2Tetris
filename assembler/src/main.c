#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "file_lines.h"

#define JUMP_GREATER    0x0001
#define JUMP_EQUAL      0x0002
#define JUMP_LESS       0x0004
#define STORE_MEMORY    0x0008
#define STORE_DATA      0x0010
#define STORE_ADDRESS   0x0020
#define NEGATE_OUTPUT   0x0040
#define FUNCTION        0x0080
#define NEGATE_Y        0x0100
#define ZERO_Y          0x0200
#define NEGATE_X        0x0400
#define ZERO_X          0x0800
#define SWAP_Y          0x1000
#define C_INSTRUCTION   0xE000
#define A_MASK          0x7FFF

// A split C instruction for computation
struct instruction {
        char assignment[4];
        char operation;
        char operands[3];
        char jump[4];
};

// Start a symbol table with the predefined symbols of the Hack assembler language
symbol_table* initialize_table()
{
        symbol_table* table = new_table();
        char digits[4];
        // R0-R15
        for (short i = 0; i < 16; ++i) {
                sprintf(digits, "R%d", i);
                add_symbol(table, digits, i);
        }
        add_symbol(table, "SCREEN", 0x4000);
        add_symbol(table, "KBD", 0x6000);
        add_symbol(table, "SP", 0);
        add_symbol(table, "LCL", 1);
        add_symbol(table, "ARG", 2);
        add_symbol(table, "THIS", 3);
        add_symbol(table, "THAT", 4);
        return table;
}

// Splits the line of assembler into an instruction
struct instruction parse_instruction(const char* line)
{
        struct instruction out;
        const char* assignment = strchr(line, '=');
        const char* jump = strchr(line, ';');
        const char* start = line;
        const char* end = start + strcspn(line, " /");
        const char* operation = strpbrk(start, "-+!&|");
        out.operation = operation ? *operation : '\0';
        // Check that the line assigns to registers
        if (assignment) {
                unsigned short i = 0;
                while (start != assignment) {
                        out.assignment[i++] = *start++;
                }
                out.assignment[i] = '\0';
        }
        else
                out.assignment[0] = '\0';
        // Check that the line has a jump condition
        if (jump) {
                end = jump;
                strncpy(out.jump, ++jump, 3);
                out.jump[3] = '\0';
        }
        else
                out.jump[0] = '\0';
        // Find the operands in order
        char* operand_ptr = out.operands;
        while (start != end) {
                const char c = *start++;
                if (strchr("01ADM", c))
                        *operand_ptr++ = c;
        }
        *operand_ptr = '\0';
        return out;
}

// Return the value needed for proper assignment
unsigned short evaluate_assignment(const char* assignment)
{
        unsigned short out = 0;
        const char* ptr = assignment;
        while (*ptr) {
                const char c = *ptr++;
                if (c == 'A')
                        out |= STORE_ADDRESS;
                else if (c == 'D')
                        out |= STORE_DATA;
                else if (c == 'M')
                        out |= STORE_MEMORY;
        }
        return out;
}

// Returns the value needed for jump
unsigned short evaluate_jump(const char* jump)
{
        // No jump statement
        if (!jump[0])
                return 0;
        // Special requirements for NOT_EQUAL and JUMP
        if (!strcmp(jump, "JNE"))
                return JUMP_LESS | JUMP_GREATER;
        if (!strcmp(jump, "JMP"))
                return JUMP_LESS | JUMP_EQUAL | JUMP_GREATER;
        // Apply character meanings
        unsigned short out = 0;
        while (*jump) {
                const char c = *jump++;
                if (c == 'G')
                        out |= JUMP_GREATER;
                else if (c == 'L')
                        out |= JUMP_LESS;
                else if (c == 'E')
                        out |= JUMP_EQUAL;
        }
        return out;
}

// Returns the value needed to compute the equation
unsigned short evaluate_computation(const char operator, const char* operands)
{
        // If M is contained in the operands, replace and SWAP_Y
        char* find_m = strchr(operands, 'M');
        short swap = 0;
        if (find_m) {
                swap = SWAP_Y;
                *find_m = 'A';
        }
        if (operator == '+') {
                // D+1
                if (operands[0] == 'D' && operands[1] == '1')
                        return NEGATE_X | ZERO_Y | NEGATE_Y | FUNCTION | NEGATE_OUTPUT;
                // A+1
                if (operands[0] == 'A' && operands[1] == '1')
                        return swap | ZERO_X | NEGATE_X | NEGATE_Y | FUNCTION | NEGATE_OUTPUT;
                // D+A or A+D
                return swap | FUNCTION;
        }
        if (operator == '-') {
                if (operands[1] == '1') {
                        // D-1
                        if (operands[0] == 'D')
                                return NEGATE_Y | ZERO_Y | FUNCTION;
                        // A-1
                        if (operands[0] == 'A')
                                return swap | NEGATE_X | ZERO_X | FUNCTION;
                }
                // A-D
                if (operands[1] == 'D')
                        return swap | NEGATE_Y | FUNCTION | NEGATE_OUTPUT;
                // D-A
                if (operands[1] == 'A')
                        return swap | NEGATE_X | FUNCTION | NEGATE_OUTPUT;
                // -D
                if (operands[0] == 'D')
                        return ZERO_Y | NEGATE_Y | FUNCTION | NEGATE_OUTPUT;
                // -A
                if (operands[0] == 'A')
                        return swap | ZERO_X | NEGATE_X | FUNCTION | NEGATE_OUTPUT;
                // -1
                return ZERO_X | NEGATE_X | ZERO_Y | FUNCTION;
        }
        if (operator == '!') {
                // !D
                if (operands[0] == 'D')
                        return ZERO_Y | NEGATE_Y | NEGATE_OUTPUT;
                // !A
                if (operands[0] == 'A')
                        return swap | ZERO_X | NEGATE_X | NEGATE_OUTPUT;
        }
        // D|A
        if (operator == '|')
                return swap | NEGATE_X | NEGATE_Y | NEGATE_OUTPUT;
        // D&A
        if (operator == '&')
                return swap;
        // D
        if (operands[0] == 'D')
                return ZERO_Y | NEGATE_Y;
        // A
        if (operands[0] == 'A')
                return swap | ZERO_X | NEGATE_X;
        // 1
        if (operands[0] == '1')
                return ZERO_X | NEGATE_X | ZERO_Y | NEGATE_Y | FUNCTION | NEGATE_OUTPUT;
        // 0
        return ZERO_X | ZERO_Y | FUNCTION;
}

const char* parse_token(const char* line)
{
        const char* token = strtok(strdup(&line[1]), ")\n");
        return token;
}

// Convert number into binary string
const char* to_binary(char* buffer, const unsigned short number)
{
        const char* start = buffer;
        for (unsigned short i = 0x8000; i > 0; i >>= 1) {
                *buffer++ = i & number ? '1' : '0';
        }
        *buffer = '\0';
        return start;
}

// Returns the value required for the C instruction
unsigned short c_instruction(const char* line)
{
        const struct instruction c_inst = parse_instruction(line);
        unsigned short out = C_INSTRUCTION;
        out |= evaluate_assignment(c_inst.assignment);
        out |= evaluate_jump(c_inst.jump);
        out |= evaluate_computation(c_inst.operation, c_inst.operands);
        return out;
}

// Returns the value for A to be set to
unsigned short a_instruction(const char* line, short* line_number, symbol_table* table)
{
        char* end;
        const long number = strtol(&line[1], &end, 10);
        // Plain old number
        if (*end == '\n')
                return number;
        // Symbol already contained?
        const char* symbol = parse_token(line);
        short value = get_symbol(table, symbol);
        // If not, add it
        if (value < 0) {
                add_symbol(table, symbol, *line_number);
                value = (*line_number)++;
        }
        return value;
}

int main(const int argc, char* argv[])
{
        // Needs a file
        if (argc <= 1) {
                fprintf(stderr, "You must pass a file.\n");
                return 1;
        }
        // Checking file ending, as a precautionary layer
        const unsigned short filepath_length = strlen(argv[1]);
        char filepath[filepath_length + 2];
        char* extension = strcpy(filepath, argv[1]) + filepath_length - 4;
        if (!extension || strcmp(extension, ".asm")) {
                fprintf(stderr, "%s is not an assembly file.\n", filepath);
                return 1;
        }
        // Open the file
        FILE* in = fopen(filepath, "r");
        if (!in) {
                fprintf(stderr, "%s couldn't be opened.\n", filepath);
                return 1;
        }
        // Read all lines of the file into a buffer
        file_lines* assembly = new_file_lines();
        while (!feof(in)) {
                char line[255];
                const char* line_ptr = fgets(line, 255, in);
                // Ignore all indentation
                while (isspace(*line_ptr))
                        line_ptr++;
                if (strchr("/\n\0", *line_ptr))
                        continue;
                add_line(assembly, line_ptr);
        }
        // TODO Symbol pass
        symbol_table* table = initialize_table();
        for (short i = 0, assembly_i = 0; i < assembly->length; ++i) {
                const char* line = assembly->line[i];
                if (line[0] != '(') {
                        ++assembly_i;
                        continue;
                }
                add_symbol(table, parse_token(line), assembly_i);
        }

        // Computation pass
        file_lines* binary = new_file_lines();
        char binary_buffer[17];
        for (short i = 0, table_i = 16; i < assembly->length; ++i) {
                const char* line = assembly->line[i];
                // Skip over labels
                if (line[0] == '(')
                        continue;
                // Compute the proper instruction
                unsigned short value;
                if (line[0] == '@') {
                        value = A_MASK & a_instruction(line, &table_i, table);
                }
                else {
                        value = C_INSTRUCTION | c_instruction(line);
                }
                add_line(binary, to_binary(binary_buffer, value));
        }
        free_table(table);
        free_file_lines(assembly);
        // Redirect the extension and open the file
        strncpy(extension, ".hack\0", 6);
        FILE* out = fopen(filepath, "w");
        if (!out) {
                fprintf(stderr, "%s couldn't be written to.\n", filepath);
                free_file_lines(binary);
                return 1;
        }
        // Write the binary to the output file
        char** out_ptr = binary->line;
        char** out_end = binary->line + binary->length - 1;
        while (out_ptr != out_end) {
                fprintf(out, "%s\n", *out_ptr++);
        }
        fprintf(out, "%s", *out_ptr);
        fclose(out);
        free_file_lines(binary);
        return 0;
}
