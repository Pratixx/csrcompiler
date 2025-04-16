// [ INCLUDING ] //

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// [ MACROS ] //

#define advance(x) ((pStream->index) += (x))

#define peek(x) (((pStream->index + (x)) >= pStream->size) ? (token){TOKEN_TYPE_EOF, ""} : (pStream->buffer[pStream->index + (x)]))
#define upeek(x) (pStream->buffer[pStream->index + (x)])
#define ppeek(x) ((pStream->index >= pStream->size + (x)) ? (token[]){(token){TOKEN_TYPE_EOF, ""}} : &(pStream->buffer[pStream->index + (x)]))

// #define panic(x) for (; (pStream->index < pStream->size) && (pStream->buffer[pStream->index].type != x); (pStream->index)++)

#define panic(...) \
	do { \
		int _found = 0; \
		while (pStream->index < pStream->size) { \
			int i; \
			for (i = 0; i < sizeof((int[]){__VA_ARGS__}) / sizeof(int); i++) { \
				if (pStream->buffer[pStream->index].type == ((int[]){__VA_ARGS__})[i]) { \
					_found = 1; \
					break; \
				} \
			} \
			if (_found) break; \
			(pStream->index)++; \
		} \
	} while (0)

// [ FUNCTIONS ] //

node* node_parse(stream* pStream, node* pParent, symbol_table* pSymbolTable, error_table* pErrorTable);

/*////////*/

bool token_isKeyword(token_type tokenType) {
	size_t index = 0;
	while (token_kw_table[index].type != TOKEN_TYPE_UNDEFINED) {
		if (token_kw_table[index].type == tokenType) return true;
		index++;
	}
	return false;
}

bool token_isLiteral(token_type tokenType) {
	switch (tokenType) {
		case (TOKEN_TYPE_LITERAL_CHAR) return true;
		case (TOKEN_TYPE_LITERAL_STR) return true;
		case (TOKEN_TYPE_LITERAL_INT) return true;
		case (TOKEN_TYPE_LITERAL_INT_HEX) return true;
		case (TOKEN_TYPE_LITERAL_FLOAT) return true;
		default: return false;
	}
}

bool token_isIdentifier(token_type tokenType) {
	
	// Immediately return true for actual identifiers
	if (tokenType == TOKEN_TYPE_IDENTIFIER) return true;
	return false;
	
}

bool token_isOperator(token_type tokenType) {
	size_t index = 0;
	while (token_op_table[index].type != TOKEN_TYPE_UNDEFINED) {
		if (token_op_table[index].type == tokenType) return true;
		index++;
	}
	return false;
}

bool token_isPunctuator(token_type tokenType) {
	size_t index = 0;
	while (token_pt_table[index].type != TOKEN_TYPE_UNDEFINED) {
		if (token_pt_table[index].type == tokenType) return true;
		index++;
	}
	return false;
}

bool token_isTypeSpecifier(token_type tokenType) {
	size_t index = 0;
	while (token_sp_table[index].type != TOKEN_TYPE_UNDEFINED) {
		if (token_sp_table[index].type == tokenType) return true;
		index++;
	}
	return false;
}

bool token_isTypeQualifier(token_type tokenType) {
	size_t index = 0;
	while (token_qu_table[index].type != TOKEN_TYPE_UNDEFINED) {
		if (token_qu_table[index].type == tokenType) return true;
		index++;
	}
	return false;
}

/*////////*/

bool node_isMath(node_type nodeType) {
	size_t index = 0;
	while (node_math_table[index].type != NODE_TYPE_UNDEFINED) {
		if (node_math_table[index].type == nodeType) return true;
		index++;
	}
	return false;
}

/*////////*/

void token_resolve(token* pToken, node* pParent, symbol_table* pSymbolTable) {
	
	if (pToken->type == TOKEN_TYPE_INVALID) {
		
		// Set beforehand
		pToken->type = TOKEN_TYPE_IDENTIFIER;
		
		// If this follows a type specifier or a type qualifier, it is most likely a type
		if (token_isTypeSpecifier(pToken[-1].type) || token_isTypeQualifier(pToken[-1].type)) return;
		
		if (pParent->type == NODE_TYPE_CONDITION) return;
		
		if (pParent->type == NODE_TYPE_STATEMENT) {
			
			switch (pParent->tokenList[0].type) {
				
				// Intentional fallthrough
				case (TOKEN_TYPE_KW_IF)
				case (TOKEN_TYPE_KW_WHILE)
				
				return;
				
				default: break;
				
			}
			
		}
		
		// Check the symbol table to see if it's an identifier
		symbol* foundSymbol = symbol_find(pSymbolTable, pToken->value, SYMBOL_CLASS_ALL);
		if (foundSymbol) return;
		
		// Check surrounding tokens to see if it's an identifier
		foundSymbol = symbol_find(pSymbolTable, pToken[-1].value, SYMBOL_CLASS_TYPE);
		if (foundSymbol) return;
		
		if ((pToken[-1].type == TOKEN_TYPE_KW_MODULE) || (pToken[-1].type == TOKEN_TYPE_KW_HEADER)) return;
		
		// If nothing was valid, return to the original value
		pToken->type = TOKEN_TYPE_INVALID;
		
	} else if (pToken->type == TOKEN_TYPE_PT_AMPERSAND) {
		
		if (token_isIdentifier(pToken[-1].type) || (pToken[-1].type == TOKEN_TYPE_SP_PTR)) {
			
			pToken->type = TOKEN_TYPE_SP_PTR;
			
			return;
			
		} 
		
	}
	
}

/*////////*/

void expression_parse(stream* pStream, size_t tokenIndex, size_t tokenLen) {
	
	
	
}

/*////////*/

node* node_new(node_type type, node* parent) {
	
	// Allocate a new node
	node* newNode = (node*)calloc(1, sizeof(node));
	if (!newNode) return NULL;
	
	// Define node properties
	newNode->type = type;
	newNode->parent = parent;
	newNode->firstChild = NULL;
	newNode->nextSibling = NULL;
	
	// Return the node to the caller
	return newNode;
	
}

void node_delete(node* pNode) {
	
	// Check if the node has children
	if (pNode->firstChild) {
		node_delete(pNode->firstChild);
	}
	
	// Check if the node has siblings
	if (pNode->nextSibling) {
		node_delete(pNode->nextSibling);
	}
	
	// Free the node
	free(pNode);
	
}

void node_print(node* pNode, unsigned int depth) {
	
	if (depth > 0) {
		node* currentNode = pNode;
		unsigned int currentDepth = depth - 1;
		unsigned short treeGraph[depth + 1];
		treeGraph[depth] = L'\0';
		while (currentNode->parent) {
			if (currentDepth == (depth - 1)) {
				if (currentNode->nextSibling) {
					treeGraph[currentDepth] = L'├';
				} else {
					treeGraph[currentDepth] = L'└';
				}
			} else {
				if (currentNode->nextSibling) {
					treeGraph[currentDepth] = L'│';
				} else {
					treeGraph[currentDepth] = L' ';
				}
			}
			currentNode = currentNode->parent;
			currentDepth--;
		}
		print_utf16(treeGraph);
	}
	
	print_utf8("%s ",
		(pNode->type == NODE_TYPE_FILE) ? "FILE" :
		(pNode->type == NODE_TYPE_INVALID) ? "INVALID" :
		(pNode->type == NODE_TYPE_UNDEFINED) ? "UNDEFINED" :
		(pNode->type == NODE_TYPE_IDENTIFIER) ? "IDENTIFIER" :
		(pNode->type == NODE_TYPE_LITERAL_CHAR) ? "LITERAL_CHAR" :
		(pNode->type == NODE_TYPE_LITERAL_STR) ? "LITERAL_STR" :
		(pNode->type == NODE_TYPE_LITERAL_INT) ? "LITERAL_INT" :
		(pNode->type == NODE_TYPE_LITERAL_FLOAT) ? "LITERAL_FLOAT" :
		(pNode->type == NODE_TYPE_DECL_FUNCTION) ? "DECL_FUNC" :
		(pNode->type == NODE_TYPE_DECL_VARIABLE) ? "DECL_VAR" :
		(pNode->type == NODE_TYPE_DECL_PARAMETER) ? "DECL_PARAM" :
		(pNode->type == NODE_TYPE_MATH_ADD) ? "MATH_ADD" :
		(pNode->type == NODE_TYPE_SCOPE) ? "SCOPE" :
		(pNode->type == NODE_TYPE_CONDITION) ? "CONDITION" :
		(pNode->type == NODE_TYPE_CONDITION_ELSE) ? "CONDITION_ELSE" :
		(pNode->type == NODE_TYPE_ASSIGN) ? "ASSIGN" :
		(pNode->type == NODE_TYPE_STATEMENT) ? "STATEMENT" :
		"EOF"
	);
	
	if (pNode->tokenCount > 0) {
		print_utf8("[");
		for (size_t i = 0; i < pNode->tokenCount; i++) {
			print_utf8("%s", pNode->tokenList[i].value);
			if (i < pNode->tokenCount - 1) print_utf8(" ");
		}
		print_utf8("]");
	}
	
	print_utf8("\n");
	
	if (pNode->firstChild) {
		node_print(pNode->firstChild, depth + 1);
	}
	
	if (pNode->nextSibling) {
		node_print(pNode->nextSibling, depth);
	}
	
}

// There are two principles this function should follow: it starts on the
// token it will read, and it will end on the token afterwards in preparation
// for the next node to be made. For example, "int var = 5;" should start on
// "int" and end on the token after ";", always.

node* node_parse(stream* pStream, node* pParent, symbol_table* pSymbolTable, error_table* pErrorTable) {
	
	// This current node
	node* currentNode = node_new(NODE_TYPE_UNDEFINED, pParent);
	if (!currentNode) return NULL;
	
	// Define the current token
	token* startToken = ppeek(0);
	
	// Immediately attempt to resolve the token in case it is unresolved
	token_resolve(startToken, pParent, pSymbolTable);
	
	// Assign the token to the node
	currentNode->tokenCount = 1;
	currentNode->tokenList = startToken;
	
	// Resolve literals
	if (token_isLiteral(startToken->type)) {
		
		// Get the length of the first token's value
		size_t valueLen = strlen(currentNode->tokenList[0].value);
		
		char* value = currentNode->tokenList[0].value;
		
		// Check if this is a char, string, int, or float literal
		if (value[0] == '\"') {
			currentNode->type = NODE_TYPE_LITERAL_STR;
		} else if ((value[0] == '0') && (value[1] == 'x')) {
			currentNode->type = NODE_TYPE_LITERAL_INT_HEX;
		} else if (value[0] == '\'') {
			currentNode->type = NODE_TYPE_LITERAL_CHAR;
		} else if (isdigit(value[0])) {
			currentNode->type = NODE_TYPE_LITERAL_INT;
		} else if (value[valueLen - 1] == 'f') {
			currentNode->type = NODE_TYPE_LITERAL_FLOAT;
		} else {
			currentNode->type = NODE_TYPE_INVALID;
		}
		
		advance(1);
		
		return currentNode;
		
	}
	
	// Handle identifiers
	if (token_isTypeQualifier(startToken->type) || token_isTypeSpecifier(startToken->type) || token_isIdentifier(startToken->type)) {
		
		// Check if this is just a regular identifier attached to something else, or if it is really a function or variable
		if (pParent->type == NODE_TYPE_STATEMENT) {
			
			switch (pParent->tokenList[0].type) {
				
				// Case fallthrough
				case (TOKEN_TYPE_KW_MODULE)
				case (TOKEN_TYPE_KW_HEADER)
				case (TOKEN_TYPE_KW_RETURN)
				
				{
					
					currentNode->type = NODE_TYPE_IDENTIFIER;
					
					advance(1);
					
					return currentNode;
					
				};
				
				// In the case this does not meet the above
				default: break;
				
			}
			
		}
		
		// Check if this is under a condition
		if (pParent->type == NODE_TYPE_CONDITION) {
			
			currentNode->type = NODE_TYPE_IDENTIFIER;
			
			advance(1);
			
			return currentNode;
			
		}
		
		// Linkage and location for this symbol
		symbol_linkage linkage = SYMBOL_LINKAGE_LOCAL;
		symbol_location location = SYMBOL_LOCATION_INTERNAL;
		
		// Check if this follows the "export" keyword
		if (peek(-1).type == TOKEN_TYPE_KW_EXPORT) linkage = SYMBOL_LINKAGE_GLOBAL;
		
		// Check if this follows the "import" keyword
		if (peek(-1).type == TOKEN_TYPE_KW_IMPORT) location = SYMBOL_LOCATION_EXTERNAL;
		
		// Skip all of this if this is an automatic variable
		if (peek(0).type == TOKEN_TYPE_QU_AUTO) goto auto_skip;
		
		// Resolve the remaining qualifiers and specifiers if this isn't an identifier
		while (token_isTypeQualifier(peek(0).type) || token_isTypeSpecifier(peek(0).type)) {
			(currentNode->tokenCount)++;
			advance(1);
		}
		
		// Resolve the type of the declaration
		token_resolve(ppeek(0), pParent, pSymbolTable);
		
		// Confirm that this was actually resolved into an identifier
		if (token_isIdentifier(peek(0).type)) {
			advance(1);
			(currentNode->tokenCount)++;
		} else {
			
			error_table_push(pErrorTable, ERROR_SYNTACTIC_EXPECTED_TYPE, currentNode);
			
			currentNode->type = NODE_TYPE_INVALID;
			
			// Panic relevant to this being a function or variable, and if it is a function, check if it is a declaration or definition
			if (peek(0).type == TOKEN_TYPE_PT_OPEN_PAREN)
				if (location == SYMBOL_LOCATION_EXTERNAL)
					panic(TOKEN_TYPE_PT_SEMICOLON);
				else
					panic(TOKEN_TYPE_PT_CLOSE_BRACE);
			else
				panic(TOKEN_TYPE_PT_SEMICOLON);
			
			advance(1);
			
			return currentNode;
			
		}
		
		// Auto skip point
		auto_skip:
		if (peek(0).type == TOKEN_TYPE_QU_AUTO) {
			advance(1);
			(currentNode->tokenCount)++;
		}
		
		// Resolve the name of the definition
		token_resolve(ppeek(0), pParent, pSymbolTable);
		
		// Check for specifiers like pointers and skip them
		while (peek(0).type == TOKEN_TYPE_SP_PTR) {
			
			(currentNode->tokenCount)++;
			advance(1);
			
			token_resolve(ppeek(0), pParent, pSymbolTable);
			
		}
		
		// Confirm that this was actually resolved into an identifier
		if (token_isIdentifier(peek(0).type)) {
			
			advance(1);
			
			// Check for a function
			if (peek(0).type == TOKEN_TYPE_PT_OPEN_PAREN) {
				
				currentNode->type = NODE_TYPE_DECL_FUNCTION;
				
				node* thisNode = NULL;
				
				advance(1);
				
				// Get the function parameters
				if (peek(0).type != TOKEN_TYPE_PT_CLOSE_PAREN) {
					
					currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable);
					thisNode = currentNode->firstChild;
					
					while (peek(0).type != TOKEN_TYPE_PT_CLOSE_PAREN) {
						
						if (peek(0).type != TOKEN_TYPE_PT_COMMA)
							error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_COMMA, currentNode);
						else
							advance(1);
						
						thisNode->nextSibling = node_parse(pStream, currentNode, pSymbolTable, pErrorTable);
						thisNode = thisNode->nextSibling;
						
					}
					
					advance(1);
					
				} else
					advance(2);
				
				// Get the function body
				if (peek(0).type == TOKEN_TYPE_PT_OPEN_BRACE) {
					
					// Create a new scope node
					node* scopeNode = node_new(NODE_TYPE_SCOPE, currentNode);
					
					if (thisNode)
						thisNode->nextSibling = scopeNode;
					else
						currentNode->firstChild = scopeNode;
					
					// Get the first child
					advance(1);
					
					scopeNode->firstChild = node_parse(pStream, thisNode, pSymbolTable, pErrorTable);
					thisNode = scopeNode->firstChild;
					
					while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
						
						thisNode->nextSibling = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable);
						thisNode = thisNode->nextSibling;
						
					}
					
					advance(1);
					
				} else if ((peek(0).type == TOKEN_TYPE_PT_SEMICOLON) && (pParent->type == NODE_TYPE_STATEMENT) && (pParent->tokenList[0].type == TOKEN_TYPE_KW_IMPORT)) {
					
					advance(1);
					
					// Proceed to symbol resolution
					
				} else {
					
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_BRACE, currentNode);
					
				}
				
			} else if (peek(0).type == TOKEN_TYPE_PT_SEMICOLON) {
				currentNode->type = NODE_TYPE_DECL_VARIABLE;
				advance(1);
			} else if (peek(0).type == TOKEN_TYPE_OP_ASSIGN) {
				
				currentNode->type = NODE_TYPE_DECL_VARIABLE;
				
				// Make an assignnment node
				node* thisNode = node_new(NODE_TYPE_ASSIGN, currentNode);
				thisNode->tokenCount = 1;
				thisNode->tokenList = ppeek(0);
				currentNode->firstChild = thisNode;
				
				// Assign the expression to this variable
				advance(1);
				
				thisNode->firstChild = node_parse(pStream, thisNode, pSymbolTable, pErrorTable);
				
				if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
				else
					advance(1);
				
			} else if ((peek(0).type == TOKEN_TYPE_PT_COMMA) || (peek(0).type == TOKEN_TYPE_PT_CLOSE_PAREN)) {
				if (pParent->type == NODE_TYPE_DECL_FUNCTION) {
					
					currentNode->type = NODE_TYPE_DECL_PARAMETER;
					
				} else {
					
					currentNode->type = NODE_TYPE_INVALID;
					
					return currentNode;
					
				}
			} else {
				
				currentNode->type = NODE_TYPE_INVALID;
				
				return currentNode;
			}
			
			// Add this symbol to the symbol table
			if ((currentNode->type == NODE_TYPE_DECL_FUNCTION) || (currentNode->type == NODE_TYPE_DECL_VARIABLE)) {
				
				symbol_size symbolSize = SYMBOL_SIZE_BITS_0;
				
				// Evaluate the function type to calculate its size
				size_t count_long = 0;
				size_t count_short = 0;
				
				for (size_t i = 0; i < currentNode->tokenCount; i++) {
					if (currentNode->tokenList[i].type == TOKEN_TYPE_SP_LONG) count_long++;
					if (currentNode->tokenList[i].type == TOKEN_TYPE_SP_SHORT) count_short++;
				}
				
				if (strcmp(currentNode->tokenList[currentNode->tokenCount - 2].value, "int") == 0) {
					if ((count_long + count_short) == 1) {
						if (count_long == 1) symbolSize = SYMBOL_SIZE_BITS_64;
						if (count_short == 1) symbolSize = SYMBOL_SIZE_BITS_16;
					}
				} else if (strcmp(currentNode->tokenList[currentNode->tokenCount - 2].value, "float") == 0) {
					if ((count_long + count_short) == 1) {
						if (count_long == 1) symbolSize = SYMBOL_SIZE_BITS_128;
						if (count_short == 1) symbolSize = SYMBOL_SIZE_BITS_32;
					}
				} else if (strcmp(currentNode->tokenList[currentNode->tokenCount - 2].value, "decimal") == 0) {
					if ((count_long + count_short) == 1) {
						if (count_long == 1) symbolSize = SYMBOL_SIZE_BITS_128;
						if (count_short == 1) symbolSize = SYMBOL_SIZE_BITS_32;
					}
				}
				
				symbol* currentSymbol = NULL;
				
				if (currentNode->type == NODE_TYPE_DECL_FUNCTION) {
					currentSymbol = symbol_add(pSymbolTable, currentNode->tokenList[currentNode->tokenCount - 1].value, SYMBOL_TYPE_FUNCTION, symbolSize, SYMBOL_CLASS_FUNCTION);
				} else if (currentNode->type == NODE_TYPE_DECL_VARIABLE) {
					currentSymbol = symbol_add(pSymbolTable, currentNode->tokenList[currentNode->tokenCount - 1].value, SYMBOL_TYPE_VARIABLE, symbolSize, SYMBOL_CLASS_VARIABLE);
				}
				
				// Set the symbol linkage and location appropriately
				currentSymbol->linkage = linkage;
				currentSymbol->location = location;
				
			}
			
		} else {
			
			error_table_push(pErrorTable, ERROR_SYNTACTIC_EXPECTED_IDENTIFIER, currentNode);
			
			currentNode->type = NODE_TYPE_INVALID;
			
			// Panic relevant to this being a function or variable, and if it is a function, check if it is a declaration or definition
			if (peek(0).type == TOKEN_TYPE_PT_OPEN_PAREN)
				if (location == SYMBOL_LOCATION_EXTERNAL)
					panic(TOKEN_TYPE_PT_SEMICOLON);
				else
					panic(TOKEN_TYPE_PT_CLOSE_BRACE);
			else
				panic(TOKEN_TYPE_PT_SEMICOLON);
			
			advance(1);
			
			return currentNode;
			
		}
		
		return currentNode;
		
	}
	
	// Handle all keywords
	if (token_isKeyword(startToken->type)) {
		
		currentNode->type = NODE_TYPE_STATEMENT;
		
		switch (startToken->type) {
			
			case (TOKEN_TYPE_KW_RETURN) {
				
				advance(1);
				
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable);
				
				if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
				else
					advance(1);
				
			} break;
			
			case (TOKEN_TYPE_KW_EXPORT) {
				
				advance(1);
				
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable);
				
				if (currentNode->firstChild->type != NODE_TYPE_DECL_FUNCTION)
					if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
						error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
					else
						advance(1);
				
			} break;
			
			case (TOKEN_TYPE_KW_IMPORT) {
				
				advance(1);
			
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable);
				
				if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
				else
					advance(1);
				
			} break;
			
			case (TOKEN_TYPE_KW_MODULE) {
				
				advance(1);
			
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable);
				
			} break;
			
			case (TOKEN_TYPE_KW_WHILE) {
				
				advance(1);
				
				if (peek(0).type != TOKEN_TYPE_PT_OPEN_PAREN) {
					
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_PAREN, currentNode);
					
					currentNode->type = NODE_TYPE_INVALID;
					
					panic(TOKEN_TYPE_PT_CLOSE_PAREN);
					
					return currentNode;
					
				} else
					advance(1);
				
				// Create a new condition node
				node* conditionNode = node_new(NODE_TYPE_CONDITION, currentNode);
				currentNode->firstChild = conditionNode;
				
				// Get the condition
				conditionNode->firstChild = node_parse(pStream, conditionNode, pSymbolTable, pErrorTable);
				node* thisNode = conditionNode->firstChild;
				
				while (peek(0).type != TOKEN_TYPE_PT_CLOSE_PAREN) {
					
					thisNode->nextSibling = node_parse(pStream, conditionNode, pSymbolTable, pErrorTable);
					thisNode = thisNode->nextSibling;
					
				}
				
				advance(1);
				
				// Create a new scope node
				node* scopeNode = node_new(NODE_TYPE_SCOPE, currentNode);
				conditionNode->nextSibling = scopeNode;
				
				// Check if this is a one-liner
				if (peek(0).type == TOKEN_TYPE_PT_OPEN_BRACE) {
					
					advance(1);
					
					if (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
						
						scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable);
						node* thisNode = scopeNode->firstChild;
						
						while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
							
							thisNode->nextSibling = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable);
							thisNode = thisNode->nextSibling;
							
						}
						
					}
					
					advance(1);
					
				} else {
					
					scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable);
					
				}
				
			} break;
			
			case (TOKEN_TYPE_KW_IF) {
				
				// Create a new condition node
				node* conditionNode = node_new(NODE_TYPE_CONDITION, currentNode);
				currentNode->firstChild = conditionNode;
				
				advance(1);
				
				while (1) {
					
					// Be sure this isn't an "else" condition
					if (conditionNode->type != NODE_TYPE_CONDITION_ELSE) {
						
						if (peek(0).type != TOKEN_TYPE_PT_OPEN_PAREN) {
						
							error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_PAREN, currentNode);
							
							currentNode->type = NODE_TYPE_INVALID;
							
							panic(TOKEN_TYPE_PT_CLOSE_PAREN);
							
							return currentNode;
							
						} else
							advance(1);
						
						// Get the condition
						conditionNode->firstChild = node_parse(pStream, conditionNode, pSymbolTable, pErrorTable);
						node* thisNode = conditionNode->firstChild;
						
						while (peek(0).type != TOKEN_TYPE_PT_CLOSE_PAREN) {
							
							thisNode->nextSibling = node_parse(pStream, conditionNode, pSymbolTable, pErrorTable);
							thisNode = thisNode->nextSibling;
							
						}
						
					}
					
					advance(1);
					
					// Create a new scope node
					node* scopeNode = node_new(NODE_TYPE_SCOPE, currentNode);
					conditionNode->nextSibling = scopeNode;
					
					// Check if this is a one-liner
					if (peek(0).type == TOKEN_TYPE_PT_OPEN_BRACE) {
						
						advance(1);
						
						if (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
							
							scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable);
							node* thisNode = scopeNode->firstChild;
							
							while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
								
								thisNode->nextSibling = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable);
								thisNode = thisNode->nextSibling;
								
							}
							
						}
						
						advance(1);
						
					} else {
						
						scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable);
						
					}
					
					// Check if there is another condition to resolve
					if (peek(0).type == TOKEN_TYPE_KW_ELSE) {
						
						// Create a new condition node
						scopeNode->nextSibling = node_new(NODE_TYPE_CONDITION, currentNode);
						conditionNode = scopeNode->nextSibling;
						
						// Advance past the "else" keyword
						advance(1);
						
						if (peek(0).type == TOKEN_TYPE_KW_IF)
							advance(1);
						else
							conditionNode->type = NODE_TYPE_CONDITION_ELSE;
						
					} else {
						
						advance(1);
						
						break;
						
					}
					
				}
				
			} break;
			
		}
		
		return currentNode;
		
	}
	
	// If nothing matched, simply prevent the compiler hanging
	advance(1);
	
	// Return the current node
	return currentNode;
	
}

/*////////*/

void ast_print(ast* pAST) {
	
	node_print(pAST->root, 0);
	
}

bool ast_create(ast* pAST, ast_info* pInfo) {
	
	// Create the file node, or the root node
	node* fileNode = node_new(NODE_TYPE_FILE, NULL);
	
	// Parse the stream into the AST
	pInfo->pStream->index = 0;
	
	// Parse the first node
	node* thisNode = node_parse(pInfo->pStream, fileNode, pInfo->pSymbolTable, pInfo->pErrorTable);
	if (!thisNode) return false;
	fileNode->firstChild = thisNode;
	
	// Parse every other node
	while (pInfo->pStream->index < pInfo->pStream->size) {
		thisNode->nextSibling = node_parse(pInfo->pStream, fileNode, pInfo->pSymbolTable, pInfo->pErrorTable);
		if (!thisNode->nextSibling) return false;
		thisNode = thisNode->nextSibling;
	}
	
	// Assign the file node to the AST
	pAST->root = fileNode;
	
	// Return success
	return true;
	
}

void ast_destroy(ast* pAST) {
	
	// Free memory
	node_delete(pAST->root);
	pAST->size = 0;
	
}
