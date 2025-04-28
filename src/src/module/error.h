#pragma once

// [ DEFINING ] //

typedef struct node node;

/*////////*/

typedef enum {
	
	// No error
	ERROR_NONE,
	
	// Syntactic errors
	ERROR_SYNTACTIC_MISSING_SEMICOLON,
	ERROR_SYNTACTIC_MISSING_COLON,
	ERROR_SYNTACTIC_MISSING_COMMA,
	
	ERROR_SYNTACTIC_MISSING_BRACE,
	ERROR_SYNTACTIC_MISSING_PAREN,
	ERROR_SYNTACTIC_MISSING_BRACKET,
	
	ERROR_SYNTACTIC_MALFORMED_LITERAL,
	ERROR_SYNTACTIC_MALFORMED_COMMENT,
	
	ERROR_SYNTACTIC_EXPECTED_IDENTIFIER,
	ERROR_SYNTACTIC_EXPECTED_TYPE,
	ERROR_SYNTACTIC_EXPECTED_OPERATOR,
	
	ERROR_SYNTACTIC_MISSING_BODY,
	
	// Semantic errors
	ERROR_SEMANTIC_TYPE_MISMATCH,
	ERROR_SEMANTIC_REDECLARATION,
	ERROR_SEMANTIC_ARG_MISMATCH,
	ERROR_SEMANTIC_CONST_MODIFY,
	
	ERROR_SEMANTIC_UNPOPULATED_BODY,
	
} error;

/*////////*/

typedef struct {
	size_t memSize;
	size_t size;
	error* errorBuffer;
	node** nodeBuffer;
} error_table;

// [ FUNCTIONS ] //

void error_table_push(error_table* pErrorTable, error thisError, node* pNode);

/*////////*/

void error_table_print(error_table* pErrorTable);

bool error_table_create(error_table* pErrorTable);
void error_table_destroy(error_table* pErrorTable);