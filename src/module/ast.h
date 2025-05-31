#pragma once

// [ DEFINING ] //

typedef enum {
	
	// General nodes
	NODE_TYPE_UNDEFINED,
	NODE_TYPE_INVALID,
	
	// General nodes
	NODE_TYPE_FILE,
	NODE_TYPE_SCOPE,
	NODE_TYPE_CONDITION,
	NODE_TYPE_CONDITION_ELSE,
	NODE_TYPE_OPERATION,
	NODE_TYPE_IDENTIFIER,
	NODE_TYPE_LITERAL,
	NODE_TYPE_EXPRESSION,
	NODE_TYPE_STATEMENT,
	
	// Declaration nodes
	NODE_TYPE_DECL_FUNCTION,
	NODE_TYPE_DECL_VARIABLE,
	NODE_TYPE_DECL_PARAMETER,
	
	// Call nodes
	NODE_TYPE_CALL_FUNCTION,
	
} node_type;

typedef struct node {
	
	struct node* parent;
	struct node* firstChild;
	struct node* nextSibling;
	
	node_type type;
	
	size_t tokenCount;
	token* tokenList;
	
	size_t scopeIndex;
	
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
	size_t scopeIndex;
} ast;

// [ FUNCTIONS ] //

void ast_print(ast* pAST);

bool ast_create(ast* pAST, ast_info* pInfo);
void ast_destroy(ast* pAST);