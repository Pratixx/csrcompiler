// [ INCLUDING ] //

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// [ FUNCTIONS ] //

bool symbol_table_resize(symbol_table* pSymbolTable) {
	
	// If the buffer can hold more elements, just return
	if (pSymbolTable->size < pSymbolTable->memSize) return true;
	
	// If there is no buffer, allocate a new buffer
	if (pSymbolTable->memSize == 0) {
		
		// Set default buffer size
		pSymbolTable->memSize = 8;
		pSymbolTable->size = 0;
		
		// Allocate a new buffer
		symbol* newBuffer = calloc(pSymbolTable->memSize, sizeof(symbol));
		if (!newBuffer) return false;
		
		// Assign the new buffer to the old one
		pSymbolTable->buffer = newBuffer;
		
		// Return success
		return true;
		
	}
	
	// Allocate a new buffer
	symbol* newBuffer = calloc((pSymbolTable->memSize * 2), sizeof(symbol));
	if (!newBuffer) return false;
	
	// Copy the new memory in and free the old buffer
	memcpy(newBuffer, pSymbolTable->buffer, (pSymbolTable->memSize * sizeof(symbol)));
	free(pSymbolTable->buffer);
	
	// Assign the new buffer to the old one
	pSymbolTable->memSize *= 2;
	pSymbolTable->buffer = newBuffer;
	
	// Return success
	return true;
	
}

/*////////*/

symbol* symbol_add(symbol_table* pSymbolTable, char* identifier, symbol_type type, symbol_size size, uint16_t class) {
	
	// Resize if needed
	symbol_table_resize(pSymbolTable);
	
	// Add a new symbol to the buffer
	strcpy(pSymbolTable->buffer[pSymbolTable->size].identifier, identifier);
	pSymbolTable->buffer[pSymbolTable->size].type = type;
	pSymbolTable->buffer[pSymbolTable->size].size = size;
	pSymbolTable->buffer[pSymbolTable->size].class = class;
	(pSymbolTable->size)++;
	
	// Return a pointer to the new symbol
	return &pSymbolTable->buffer[pSymbolTable->size - 1];
	
}

symbol* symbol_find(symbol_table* pSymbolTable, char* identifier, uint16_t class) {
	
	// Search through the symbol table for the identifier
	for (size_t i = 0; i < pSymbolTable->size; i++) {
		
		// If the strings match, we found the identifier
		if (class == SYMBOL_CLASS_ALL) {
			
			if (strcmp(pSymbolTable->buffer[i].identifier, identifier) == 0) {
			
				// Return this pointer
				return &pSymbolTable->buffer[i];
				
			}
			
		} else {
			
			if ((pSymbolTable->buffer[i].class == class) && (strcmp(pSymbolTable->buffer[i].identifier, identifier) == 0)) {
			
				// Return this pointer
				return &pSymbolTable->buffer[i];
				
			}
			
		}
		
	}
	
	// Return NULL if we found nothing
	return NULL;
	
}

/*////////*/

void symbol_table_print(symbol_table* pSymbolTable) {
	
	// Iterate through and print all symbols
	for (size_t i = 0; i < pSymbolTable->size; i++) {
		
		symbol* thisSymbol = &pSymbolTable->buffer[i];
		
		print_utf8("Symbol \"%s\":\n", thisSymbol->identifier);
		print_utf8("Type: %s",
			(thisSymbol->type == SYMBOL_TYPE_FUNCTION) ? "FUNCTION" :
			(thisSymbol->type == SYMBOL_TYPE_VARIABLE) ? "VARIABLE" :
			(thisSymbol->type == SYMBOL_TYPE_MODULE) ? "MODULE" :
			(thisSymbol->type == SYMBOL_TYPE_HEADER) ? "MODULE" :
			(thisSymbol->type == SYMBOL_TYPE_TYPE) ? "TYPE" :
			(thisSymbol->type == SYMBOL_TYPE_LITERAL) ? "LITERAL" :
			"UNKNOWN"
		);
		print_utf8("\n");
		
	}
	
}

bool symbol_table_create(symbol_table* pSymbolTable) {
	
	// Allocate a buffer for the symbol table
	if (!symbol_table_resize(pSymbolTable)) return false;
	
	// Return success
	return true;
	
}

void symbol_table_destroy(symbol_table* pSymbolTable) {
	
	// Free memory
	free(pSymbolTable->buffer);
	pSymbolTable->memSize = 0;
	pSymbolTable->size = 0;
	
}
