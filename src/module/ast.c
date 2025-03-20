// [ INCLUDING ] //

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// [ FUNCTIONS ] //



/*////////*/

node* node_new(node_type type, node* parent) {
	
	// Allocate a new node
	node* newNode = (node*)calloc(1, sizeof(node));
	if (!newNode) return NULL;
	
	// Define node properties
	newNode->type = type;
	newNode->parent = parent;
	
	// Return the node to the caller
	return newNode;
	
}

node* node_parse(node* pNode) {
	
	
	
}

/*////////*/

void ast_print(ast* pAST) {
	
	
	
}

ast_error ast_create(ast* pAST, ast_info* pInfo) {
	
	
	
	// Return success
	return AST_SUCCESS;
	
}

void ast_destroy(ast* pAST) {
	
	// Free memory
	free(pAST->buffer);
	
}
