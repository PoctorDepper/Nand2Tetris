#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

// Find index of symbol[key], or -1 if key doesn't exist
short find(const symbol_table *table, const char *key)
{
	for (short i = 0; i < table->length; ++i) {
		if (!strcmp(table->symbol[i].key, key)) {
			return i;
		}
	}
	return -1;
}

// Get the value at symbol[key], or -1 if key doesn't exist
short get_symbol(const symbol_table *table, const char *key)
{
	const short index = find(table, key);
	return index < 0 ? -1 : table->symbol[index].value;
}

// Add a symbol to the table
void add_symbol(symbol_table *table, const char *key, short value)
{
	const short index = find(table, key);
	// If the symbol exists, just set the value
	if (index != -1) {
		table->symbol[index].value = value;
		return;
	}
	// Double the cap of the data structure and reallocate
	if (table->length == table->cap) {
		table->cap *= 2;
		table->symbol = realloc(table->symbol, table->cap * sizeof(symbol));
	}
	// Duplicate the key and value into the proper place in memory
	table->symbol[table->length].key = strdup(key);
	table->symbol[table->length].value = value;
	table->length++;
}

// Return a new symbol_table
symbol_table* new_table(void)
{
	const symbol_table proto = {0, 10, malloc(10 * sizeof(symbol))};
	symbol_table *data = malloc(sizeof(symbol_table));
	*data = proto;
	return data;
}

// Free memory of the object
void free_table(symbol_table *table)
{
	for (unsigned short i = 0; i < table->length; ++i) {
		free(table->symbol[i].key);
	}
	free(table->symbol);
	free(table);
}
