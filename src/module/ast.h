#pragma once

// [ DEFINING ] //

typedef enum {
	
	// General nodes
	NODE_TYPE_UNDEFINED,
	NODE_TYPE_INVALID,
	
	NODE_TYPE_IDENTIFIER,
	
	NODE_TYPE_FILE,
	
	NODE_TYPE_SCOPE,
	
	// Literal nodes
	NODE_TYPE_LITERAL_CHAR,
	NODE_TYPE_LITERAL_STR,
	NODE_TYPE_LITERAL_INT,
	NODE_TYPE_LITERAL_INT_HEX,
	NODE_TYPE_LITERAL_FLOAT,
	
	// Declaration nodes
	NODE_TYPE_DECL_FUNCTION,
	NODE_TYPE_DECL_VARIABLE,
	NODE_TYPE_DECL_PARAMETER,
	
	// Math nodes
	NODE_TYPE_MATH_ADD,
	NODE_TYPE_MATH_SUB,
	NODE_TYPE_MATH_MUL,
	NODE_TYPE_MATH_DIV,
	NODE_TYPE_MATH_MOD,
	
	// Keyword nodes
	NODE_TYPE_STATEMENT,
	
} node_type;

typedef struct node {
	
	struct node* parent;
	struct node* firstChild;
	struct node* nextSibling;
	
	node_type type;
	
	size_t tokenCount;
	token* tokenList;
	
} node;

/*////////*/

typedef struct {
	stream* pStream;
	symbol_table* pSymbolTable;
	error_table* pErrorTable;
} ast_info;

typedef struct {
	size_t size;
	node* root;
} ast;

// [ FUNCTIONS ] //

void ast_print(ast* pAST);

bool ast_create(ast* pAST, ast_info* pInfo);
void ast_destroy(ast* pAST);