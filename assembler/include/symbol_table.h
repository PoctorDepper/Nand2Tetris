#pragma once

// Data type for table entries
typedef struct symbol {
        char* key;
        short value;
} symbol;

// Data type for the table itself
typedef struct symbol_table {
        unsigned short length;
        unsigned short cap;
        symbol* symbol;
} symbol_table;

// Get the value of the symbol, or return -1
short get_symbol(const symbol_table* table, const char* key);

// Adds a symbol to table
void add_symbol(symbol_table* table, const char* key, short value);

// Creates a new table
symbol_table* new_table(void);

// Frees a table from memory
void free_table(symbol_table* table);
