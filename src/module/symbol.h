#pragma once

// [ DECLARING ] //

typedef struct token token;

// [ DEFINING ] //

typedef enum {
	SYMBOL_TYPE_FUNCTION,
	SYMBOL_TYPE_VARIABLE,
	SYMBOL_TYPE_MODULE,
	SYMBOL_TYPE_HEADER,
	SYMBOL_TYPE_TYPE,
	SYMBOL_TYPE_LITERAL,
} symbol_type;

typedef enum {
	SYMBOL_LINKAGE_GLOBAL,
	SYMBOL_LINKAGE_LOCAL,
	SYMBOL_LINKAGE_NONE,
} symbol_linkage;

typedef enum {
	SYMBOL_LOCATION_EXTERNAL,
	SYMBOL_LOCATION_INTERNAL,
} symbol_location;

typedef enum {
	SYMBOL_SIZE_BITS_0 = 0,
	SYMBOL_SIZE_BITS_8 = 8,
	SYMBOL_SIZE_BITS_16 = 16,
	SYMBOL_SIZE_BITS_32 = 32,
	SYMBOL_SIZE_BITS_64 = 64,
	SYMBOL_SIZE_BITS_128 = 128,
} symbol_size;

typedef enum {
	SYMBOL_CLASS_ALL,
	SYMBOL_CLASS_TYPE,
	SYMBOL_CLASS_VARIABLE,
	SYMBOL_CLASS_FUNCTION,
	SYMBOL_CLASS_MODULE,
	SYMBOL_CLASS_HEADER,
	SYMBOL_CLASS_LITERAL,
} symbol_class;

typedef struct {
	
	char identifier[MAX_VALUE_LEN];
	
	uint8_t class;
	
	symbol_size size;
	symbol_type type;
	symbol_linkage linkage;
	symbol_location location;
	
	token* typeTokenList;
	token* paramTokenList;
	
} symbol;

/*////////*/

typedef struct {
	size_t memSize;
	size_t size;
	symbol* buffer;
} symbol_table;

// [ FUNCTIONS ] //

symbol* symbol_add(symbol_table* pSymbolTable, char* identifier, symbol_type type, symbol_size size, uint16_t class);
symbol* symbol_find(symbol_table* pSymbolTable, char* identifier, uint16_t class);

/*////////*/

void symbol_table_print(symbol_table* pSymbolTable);

bool symbol_table_create(symbol_table* pSymbolTable);
void symbol_table_destroy(symbol_table* pSymbolTable);