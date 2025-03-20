#pragma once

// [ DEFINING ] //

typedef enum {
	
	// General nodes
	NODE_TYPE_UNDEFINED,
	NODE_TYPE_INVALID,
	
	// Type nodes
	
	// Operator nodes
	
	
	// Keyword nodes
	
	
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

typedef enum {
	AST_SUCCESS,
	AST_ERROR_MEM_ALLOC,
} ast_error;

typedef struct {
	stream* pStream;
} ast_info;

typedef struct {
	size_t size;
	node* buffer;
} ast;

// [ FUNCTIONS ] //

void ast_print(ast* pAST);

ast_error ast_create(ast* pAST, ast_info* pInfo);
void ast_destroy(ast* pAST);