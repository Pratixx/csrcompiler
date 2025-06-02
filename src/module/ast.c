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
#define jump(x) (pStream->index) = (x)

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

// [ DEFINING ] //

static struct {
	
	bool inExpr;
	
} resolve;

// [ FUNCTIONS ] //

static node* node_parse(stream* pStream, node* pParent, symbol_table* pSymbolTable, error_table* pErrorTable, size_t* pScopeIndex);

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

static void token_resolve(token* pToken, node* pParent, symbol_table* pSymbolTable) {
	
	if (pToken->type == TOKEN_TYPE_INVALID) {
		
		// Set beforehand
		pToken->type = TOKEN_TYPE_IDENTIFIER;
		
		// Check if this is under a function call
		if (pParent->type == NODE_TYPE_CALL_FUNCTION) return;
		
		// Check if this is a function call
		if (pToken[1].type == TOKEN_TYPE_PT_OPEN_PAREN) return;
		
		// If this follows a type specifier or a type qualifier, it is most likely a type
		if (token_isTypeSpecifier(pToken[-1].type) || token_isTypeQualifier(pToken[-1].type)) return;
		
		if (pParent->type == NODE_TYPE_CONDITION) return;
		
		if (pParent->type == NODE_TYPE_STATEMENT) {
			
			switch (pParent->tokenList[0].type) {
				
				// Intentional fallthrough
				case (TOKEN_TYPE_KW_IF)
				case (TOKEN_TYPE_KW_WHILE)
				case (TOKEN_TYPE_KW_NAMESPACE)
				case (TOKEN_TYPE_KW_JUMP)
				case (TOKEN_TYPE_KW_SWITCH)
				case (TOKEN_TYPE_KW_CASE)
				case (TOKEN_TYPE_KW_RETURN)
				case (TOKEN_TYPE_KW_USING)
				case (TOKEN_TYPE_KW_ENUM)
				
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
		printf("%d\n", resolve.inExpr);
		
		if (token_isLiteral(pToken[-1].type) || token_isOperator(pToken[-1].type) || resolve.inExpr) {
			
			pToken->type = TOKEN_TYPE_OP_MUL;
			
		} else if (token_isIdentifier(pToken[-1].type) || (pToken[-1].type == TOKEN_TYPE_SP_PTR)) {
			
			pToken->type = TOKEN_TYPE_SP_PTR;
			
			
			
		}
		
	}
	
}

/*////////*/

static node* node_new(node_type type, node* parent) {
	
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

static void node_delete(node* pNode) {
	
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

static void node_print(node* pNode, unsigned int depth) {
	
	if (depth > 0) {
		node* currentNode = pNode;
		int currentDepth = depth - 1;
		unsigned short treeGraph[depth + 1] = {};
		treeGraph[depth] = L'\0';
		while ((currentNode->parent) && (currentDepth >= 0)) {
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
		(pNode->type == NODE_TYPE_LITERAL) ? "LITERAL" :
		(pNode->type == NODE_TYPE_DECL_FUNCTION) ? "DECL_FUNC" :
		(pNode->type == NODE_TYPE_DECL_VARIABLE) ? "DECL_VAR" :
		(pNode->type == NODE_TYPE_DECL_PARAMETER) ? "DECL_PARAM" :
		(pNode->type == NODE_TYPE_SCOPE) ? "SCOPE" :
		(pNode->type == NODE_TYPE_CONDITION) ? "CONDITION" :
		(pNode->type == NODE_TYPE_CONDITION_ELSE) ? "CONDITION_ELSE" :
		(pNode->type == NODE_TYPE_OPERATION) ? "OPERATION" :
		(pNode->type == NODE_TYPE_STATEMENT) ? "STATEMENT" :
		(pNode->type == NODE_TYPE_EXPRESSION) ? "EXPRESSION" :
		(pNode->type == NODE_TYPE_CALL_FUNCTION) ? "CALL_FUNC" :
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

static void expr_parse_merge_un_prefix(node** nodeList, bool* nodeConsumed, size_t i) {
	
	// Reference the operands
	node* right = nodeList[i + 1];
	
	// Assign the operands to the operation node
	nodeList[i]->firstChild = right;
	
	right->parent = nodeList[i];
	
	// Consume the nodes
	nodeConsumed[i + 1] = true;
	nodeConsumed[i] = true;
	
	// While we continue to encounter nodes of the same type, replace them
	for (size_t j = i + 1; nodeList[j] == right; j++) {
		nodeList[j] = nodeList[i];
		nodeConsumed[j] = true;
	}
	
}

static void expr_parse_merge_un_postfix(node** nodeList, bool* nodeConsumed, size_t i) {
	
	// Reference the operands
	node* left = nodeList[i - 1];
	
	// Assign the operands to the operation node
	nodeList[i]->firstChild = left;
	
	left->parent = nodeList[i];
	
	// Consume the nodes
	nodeConsumed[i - 1] = true;
	nodeConsumed[i] = true;
	
	// While we continue to encounter nodes of the same type, replace them
	for (size_t j = i - 1; nodeList[j] == left; j--) {
		nodeList[j] = nodeList[i];
		nodeConsumed[j] = true;
	}
	
}

static void expr_parse_merge_bi(node** nodeList, bool* nodeConsumed, size_t i) {
	
	// Reference the operands
	node* left = nodeList[i - 1];
	node* right = nodeList[i + 1];
	
	// Assign the operands to the operation node
	nodeList[i]->firstChild = left;
	nodeList[i]->firstChild->nextSibling = right;
	
	left->parent = nodeList[i];
	right->parent = nodeList[i];
	
	// Consume the nodes
	nodeConsumed[i - 1] = true;
	nodeConsumed[i + 1] = true;
	nodeConsumed[i] = true;
	
	// While we continue to encounter nodes of the same type, replace them
	for (size_t j = i - 1; nodeList[j] == left; j--) {
		nodeList[j] = nodeList[i];
		nodeConsumed[j] = true;
	}
	
	for (size_t j = i + 1; nodeList[j] == right; j++) {
		nodeList[j] = nodeList[i];
		nodeConsumed[j] = true;
	}
	
}

static node* expr_parse(stream* pStream, node* pParent, symbol_table* pSymbolTable, error_table* pErrorTable) {
	
	// Mark that we're in an expression
	resolve.inExpr = true;
	
	// Attempt to resolve the token in case it is unresolved
	token_resolve(ppeek(0), pParent, pSymbolTable);
	
	// Get the range of the expression
	size_t range = 0;
	int64_t currentDepth = 0;
	while (token_isOperator(peek(range).type) || token_isLiteral(peek(range).type) || token_isIdentifier(peek(range).type) || (peek(range).type == TOKEN_TYPE_PT_OPEN_PAREN) || (peek(range).type == TOKEN_TYPE_PT_CLOSE_PAREN)) {
		
		if (peek(range).type == TOKEN_TYPE_PT_OPEN_PAREN) {
			currentDepth++;
		} else if (peek(range).type == TOKEN_TYPE_PT_CLOSE_PAREN) {
			currentDepth--;
		}
		
		if (currentDepth < 0) {
			break;
		};
		
		range++;
		
		// Attempt to resolve the token in case it is unresolved
		token_resolve(ppeek(range), pParent, pSymbolTable);
		
	}
	
	for (size_t i = 0; i < range; i++) printf("%s ", peek(i).value);
	print_utf8("\n");
	
	// Placeholder open and close paren operators
	node* openParen = node_new(NODE_TYPE_UNDEFINED, NULL);
	if (!openParen) return NULL;
	node* closeParen = node_new(NODE_TYPE_UNDEFINED, NULL);
	if (!closeParen) return NULL;
	
	// Create a node for every operator and operand
	node* nodeList[range];
	bool nodeConsumed[range] = {};
	for (size_t i = 0; i < range; i++) {
		
		if (peek(i).type == TOKEN_TYPE_PT_OPEN_PAREN) {
			
			// We should leave these blank, and the parser can recognize them
			openParen->tokenCount = 1;
			openParen->tokenList = ppeek(i);
			
			nodeList[i] = openParen;
			
		} else if (peek(i).type == TOKEN_TYPE_PT_CLOSE_PAREN) {
			
			// We should leave these blank, and the parser can recognize them
			closeParen->tokenCount = 1;
			closeParen->tokenList = ppeek(i);
			
			nodeList[i] = closeParen;
			
		} else {
			
			// Create a new node
			nodeList[i] = node_new(NODE_TYPE_UNDEFINED, NULL);
			if (!nodeList[i]) return NULL;
			
			// Assign the token to the node
			nodeList[i]->tokenCount = 1;
			nodeList[i]->tokenList = ppeek(i);
			
			// Resolve the token
			if (token_isIdentifier(peek(i).type)) nodeList[i]->type = NODE_TYPE_IDENTIFIER;
			if (token_isLiteral(peek(i).type)) nodeList[i]->type = NODE_TYPE_LITERAL;
			if (token_isOperator(peek(i).type)) nodeList[i]->type = NODE_TYPE_OPERATION;
			
		}
		
	}
	
	// Parse all subexpressions in advance
	size_t startIndex = (pStream->index);
	for (size_t i = 0; i < range; i++) {
		
		if (nodeList[i] == openParen) {
			
			// Jump to the start of the expression
			advance(i + 1);
			
			// Parse the expression
			node* subExprNode = expr_parse(pStream, pParent, pSymbolTable, pErrorTable);
			
			// Get the range of the subexpression
			size_t subExprRange = (pStream->index + 1) - (startIndex + i);
			
			// Assign the subexpression to the node
			for (size_t j = i; j < i + subExprRange; j++) {
				nodeList[j] = subExprNode;
				nodeConsumed[j] = true;
			}
			
			jump(startIndex);
			
		}
		
	}
	
	bool merged = true;
	
	while (merged) {
		
		merged = false;
		
		// Evaluate unary not
		currentDepth = 0;
		for (size_t i = 0; i < range; i++) {
			
			if (nodeList[i] == openParen)
				currentDepth++;
			else if (nodeList[i] == closeParen)
				currentDepth--;
			
			if ((nodeList[i]->tokenList->type == TOKEN_TYPE_OP_CMP_NOT)) {
				if ((nodeList[i]->type == NODE_TYPE_OPERATION) && (!nodeConsumed[i]) && (currentDepth == 0)) {
					
					// Merge the operands into the operator
					expr_parse_merge_un_prefix(nodeList, nodeConsumed, i);
					
					// Note that a merge occurred
					merged = true;
					
				}
			}
			
		}
		
		// Evaluate unary negate
		currentDepth = 0;
		for (size_t i = 0; i < range; i++) {
			
			if (nodeList[i] == openParen)
				currentDepth++;
			else if (nodeList[i] == closeParen)
				currentDepth--;
			
			if ((nodeList[i]->tokenList->type == TOKEN_TYPE_OP_SUB)) {
				if ((nodeList[i]->type == NODE_TYPE_OPERATION) && (!nodeConsumed[i]) && (currentDepth == 0)) {
					if (((i > 0) && (nodeList[i - 1]->type != NODE_TYPE_LITERAL) && (nodeList[i - 1]->type != NODE_TYPE_IDENTIFIER)) || (i == 0)) {
						
						// Merge the operands into the operator
						expr_parse_merge_un_prefix(nodeList, nodeConsumed, i);
						
						// Note that a merge occurred
						merged = true;
						
					}
				}
			}
			
		}
		
		// Evaluate binary multiplication and division
		currentDepth = 0;
		for (size_t i = 0; i < range; i++) {
			
			if (nodeList[i] == openParen)
				currentDepth++;
			else if (nodeList[i] == closeParen)
				currentDepth--;
			
			if ((nodeList[i]->tokenList->type == TOKEN_TYPE_OP_MUL) || (nodeList[i]->tokenList->type == TOKEN_TYPE_OP_DIV)) {
				if ((nodeList[i]->type == NODE_TYPE_OPERATION) && (!nodeConsumed[i]) && (currentDepth == 0)) {
					
					// Merge the operands into the operator
					expr_parse_merge_bi(nodeList, nodeConsumed, i);
					
					// Note that a merge occurred
					merged = true;
					
				}
			}
			
		}
	
		// Evaluate binary addition and subtraction
		currentDepth = 0;
		for (size_t i = 0; i < range; i++) {
			
			if (nodeList[i] == openParen)
				currentDepth++;
			else if (nodeList[i] == closeParen)
				currentDepth--;
			
			if ((nodeList[i]->tokenList->type == TOKEN_TYPE_OP_ADD) || (nodeList[i]->tokenList->type == TOKEN_TYPE_OP_SUB)) {
				if ((nodeList[i]->type == NODE_TYPE_OPERATION) && (!nodeConsumed[i]) && (currentDepth == 0)) {
					
					// Merge the operands into the operator
					expr_parse_merge_bi(nodeList, nodeConsumed, i);
					
					// Note that a merge occurred
					merged = true;
					
				}
			}
			
		}
		
	}
	
	merged = true;
	
	while (merged) {
		
		merged = false;
		
		// Evaluate conditional and
		currentDepth = 0;
		for (size_t i = 0; i < range; i++) {
			
			if (nodeList[i] == openParen)
				currentDepth++;
			else if (nodeList[i] == closeParen)
				currentDepth--;
			
			if (nodeList[i]->tokenList->type == TOKEN_TYPE_OP_CMP_AND) {
				if ((nodeList[i]->type == NODE_TYPE_OPERATION) && (!nodeConsumed[i]) && (currentDepth == 0)) {
					
					// Merge the operands into the operator
					expr_parse_merge_bi(nodeList, nodeConsumed, i);
					
					// Note that a merge occurred
					merged = true;
					
				}
			}
			
		}
		
		// Evaluate conditional or
		currentDepth = 0;
		for (size_t i = 0; i < range; i++) {
			
			if (nodeList[i] == openParen)
				currentDepth++;
			else if (nodeList[i] == closeParen)
				currentDepth--;
			
			if (nodeList[i]->tokenList->type == TOKEN_TYPE_OP_CMP_OR) {
				if ((nodeList[i]->type == NODE_TYPE_OPERATION) && (!nodeConsumed[i]) && (currentDepth == 0)) {
					
					// Merge the operands into the operator
					expr_parse_merge_bi(nodeList, nodeConsumed, i);
					
					// Note that a merge occurred
					merged = true;
					
				}
			}
			
		}
		
	}
	
	// Search up the hierarchy for the root node
	node* rootNode = nodeList[0];
	while (rootNode->parent) rootNode = rootNode->parent;
	
	advance(range);
	
	node_print(rootNode, 0);
	
	// Free the temporary open and closing paren nodes
	node_delete(openParen);
	node_delete(closeParen);
	
	// Parent the current node to the passed parent
	rootNode->parent = pParent;
	
	// Mark that we're out of an expression
	resolve.inExpr = false;
	
	// Return the expression
	return rootNode;
	
}

static node* node_parse(stream* pStream, node* pParent, symbol_table* pSymbolTable, error_table* pErrorTable, size_t* pScopeIndex) {
	
	// This current node
	node* currentNode = node_new(NODE_TYPE_UNDEFINED, pParent);
	currentNode->scopeIndex = *pScopeIndex;
	if (!currentNode) return NULL;
	
	// Define the current token
	token* startToken = ppeek(0);
	
	// Immediately attempt to resolve the token in case it is unresolved
	token_resolve(startToken, pParent, pSymbolTable);
	
	// Assign the token to the node
	currentNode->tokenCount = 1;
	currentNode->tokenList = startToken;
	
	// Resolve parentheses
	if (startToken->type == TOKEN_TYPE_PT_OPEN_PAREN) {
		
		if (token_isLiteral(peek(1).type) || token_isIdentifier(peek(1).type)) {
			
			// Parse this expression
			node* exprNode = expr_parse(pStream, pParent, pSymbolTable, pErrorTable);
			
			// We are part of an expression and can be discarded
			node_delete(currentNode);
			
			return exprNode;
			
		}
		
	}
	
	// Resolve literals
	if (token_isLiteral(startToken->type)) {
		
		currentNode->type = NODE_TYPE_LITERAL;
		
		// Resolve the next token to see if its a multiplication operator
		token_resolve(ppeek(1), pParent, pSymbolTable);
		
		// Before we advance, check if we are part of an expression; if not, advance past the literal
		if (token_isOperator(peek(1).type)) {
			
			// Parse this expression
			node* exprNode = expr_parse(pStream, pParent, pSymbolTable, pErrorTable);
			
			// We are part of an expression and can be discarded
			node_delete(currentNode);
			
			return exprNode;
			
		} else
			advance(1);
		
		return currentNode;
		
	}
	
	// Resolve expressions
	if (token_isOperator(startToken->type)) {
		
		switch (startToken->type) {
			
			case (TOKEN_TYPE_OP_ASSIGN) {
				
				currentNode->type = NODE_TYPE_OPERATION;
				currentNode->tokenCount = 1;
				currentNode->tokenList = ppeek(0);
				
				// Advance to the first token in the expression
				advance(1);
				
				// Parse the expression
				currentNode->firstChild = expr_parse(pStream, currentNode, pSymbolTable, pErrorTable);
				
				// Return this to the caller
				return currentNode;
				
			} break;
			
			case (TOKEN_TYPE_OP_INC)
			case (TOKEN_TYPE_OP_DEC) {
				
				currentNode->type = NODE_TYPE_OPERATION;
				currentNode->tokenCount = 1;
				currentNode->tokenList = ppeek(0);
				
				// Advance past the semicolon
				advance(1);
				
				// Return this to the caller
				return currentNode;
				
			}
			
		}
		
	}
	
	// Handle declarations
	if (token_isTypeQualifier(startToken->type) || token_isTypeSpecifier(startToken->type) || token_isIdentifier(startToken->type)) {
		
		// Check if this is a simple, unmodified identifier
		if (pParent->type == NODE_TYPE_STATEMENT) {
			
			switch (pParent->tokenList[0].type) {
				
				// Case fallthrough
				case (TOKEN_TYPE_KW_MODULE)
				case (TOKEN_TYPE_KW_NAMESPACE)
				case (TOKEN_TYPE_KW_JUMP)
				case (TOKEN_TYPE_KW_SWITCH)
				case (TOKEN_TYPE_KW_CASE)
				case (TOKEN_TYPE_KW_RETURN) {
					
					currentNode->type = NODE_TYPE_IDENTIFIER;
					
					// Advance past the identifier
					advance(1);
					
					return currentNode;
					
				} break;
				
				// Using has a special case; we need to differentiate between whether we are the identifier or what we are using
				case (TOKEN_TYPE_KW_USING) {
					
					// If we're followed by a colon, we are the identifier
					if (peek(1).type == TOKEN_TYPE_PT_COLON) {
						
						currentNode->type = NODE_TYPE_IDENTIFIER;
						
						// Advance past the identifier
						advance(1);
						
						return currentNode;
						
					}
					
				}
				
				// Enums are also unique; they can either have a value or not
				case (TOKEN_TYPE_KW_ENUM) {
					
					currentNode->type = NODE_TYPE_IDENTIFIER;
					
					// Advance past the identifier
					advance(1);
					
					// We expect a semicolon; in the case there isn't one, push an error
					if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
						error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
					else
						advance(1);
					
					return currentNode;
					
				}
				
				// In the case this does not meet the above
				default: break;
				
			}
			
		}
		
		// Check if this is under a condition
		if (pParent->type == NODE_TYPE_CONDITION) {
			
			currentNode->type = NODE_TYPE_IDENTIFIER;
			
			// Advance past the identifier
			advance(1);
			
			return currentNode;
			
		}
		
		// Check if this is under a function call
		if (pParent->type == NODE_TYPE_CALL_FUNCTION) {
			
			currentNode->type = NODE_TYPE_IDENTIFIER;
			
			// Advance past the identifier
			advance(1);
			
			return currentNode;
			
		}
		
		// Check if this is a function call
		if (peek(1).type == TOKEN_TYPE_PT_OPEN_PAREN) {
			
			currentNode->type = NODE_TYPE_CALL_FUNCTION;
			
			// Advance past the identifier and the open paren
			advance(2);
			
			node* thisNode = NULL;
				
			// Get the function arguments; if there are none, skip the closing paren
			if (peek(0).type != TOKEN_TYPE_PT_CLOSE_PAREN) {
				
				// Get the first child of this node
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				thisNode = currentNode->firstChild;
				
				// Get the rest of the arguments
				while (peek(0).type != TOKEN_TYPE_PT_CLOSE_PAREN) {
					
					// We expect a comma; in the case it isnt, push an error
					if (peek(0).type != TOKEN_TYPE_PT_COMMA)
						error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_COMMA, currentNode);
					else
						advance(1);
					
					// Get the next sibling
					thisNode->nextSibling = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
					thisNode = thisNode->nextSibling;
					
				}
				
			}
			
			// Advance past the closing paren
			advance(1);
			
			// We expect a semicolon; in the case it isn't, push an error
			if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
				error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
			else
				advance(1);
			
			return currentNode;
			
		}
		
		// Resolve the next token in the case its a multiplication operator
		token_resolve(ppeek(1), pParent, pSymbolTable);
		
		// See if we're modifying a variable rather than defining one
		if (token_isOperator(peek(1).type)) {
			
			// Check if we are the lvalue or rvalue of an assignment
			switch (peek(1).type) {
				
				case (TOKEN_TYPE_OP_INC)
				case (TOKEN_TYPE_OP_DEC)
				case (TOKEN_TYPE_OP_ASSIGN) {
					
					currentNode->type = NODE_TYPE_IDENTIFIER;
					
					// Advance past this identifier
					advance(1);
					
					// Create the expression
					currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
					
					// We expect a semicolon; in the case that it isn't, push an error
					if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
						error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
					else
						advance(1);
					
					// Return this node
					return currentNode;
					
				} break;
				
				default: {
					
					currentNode->type = NODE_TYPE_IDENTIFIER;
					
					// Parse this expression
					node* exprNode = expr_parse(pStream, pParent, pSymbolTable, pErrorTable);
					
					// We are part of an expression and can be discarded
					node_delete(currentNode);
					
					// Return the expression node
					return exprNode;
					
				} break;
				
			}
			
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
		
		// It is at this point we need to check for unnamed variables
		if (peek(0).type == TOKEN_TYPE_PT_SEMICOLON) {
			
			currentNode->type = NODE_TYPE_DECL_VARIABLE;
			
			// Advance past the semicolon
			advance(1);
			
			// Jump to the resolver
			goto resolve;
			
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
		
		// Confirm that this was actually resolved into an identifier; in the case we didn't, push an error and panic
		if (token_isIdentifier(peek(0).type)) {
			
			// Advance past the identifier
			advance(1);
			
			// Check for the appropriate assignment
			if (peek(0).type == TOKEN_TYPE_PT_OPEN_PAREN) {
				
				// Advance past the identifier
				advance(1);
				
				currentNode->type = NODE_TYPE_DECL_FUNCTION;
				
				node* thisNode = NULL;
				
				// Get the function parameters; if there are none, simply skip the closing paren
				if (peek(0).type != TOKEN_TYPE_PT_CLOSE_PAREN) {
					
					// Get the first child of this node
					currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
					thisNode = currentNode->firstChild;
					
					// Get the rest of the parameters
					while (peek(0).type != TOKEN_TYPE_PT_CLOSE_PAREN) {
						
						// We expect a comma; in the case it isnt, push an error
						if (peek(0).type != TOKEN_TYPE_PT_COMMA)
							error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_COMMA, currentNode);
						else
							advance(1);
						
						// Get the next sibling
						thisNode->nextSibling = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
						thisNode = thisNode->nextSibling;
						
					}
					
				}
				
				// Advance past the closing paren
				advance(1);
				
				// Get the function body
				if (peek(0).type == TOKEN_TYPE_PT_OPEN_BRACE) {
					
					// Create a new scope node
					node* scopeNode = node_new(NODE_TYPE_SCOPE, currentNode);
					
					// Increment the scope index
					(*pScopeIndex)++;
					scopeNode->scopeIndex = *pScopeIndex;
					
					// If there is a previous node, link it to this one
					if (thisNode)
						thisNode->nextSibling = scopeNode;
					else
						currentNode->firstChild = scopeNode;
					
					// Advannce past the open brace
					advance(1);
					
					// Get the first child of this node
					scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
					thisNode = scopeNode->firstChild;
					
					// Get the remaining children
					while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
						
						thisNode->nextSibling = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
						thisNode = thisNode->nextSibling;
						
					}
					
					// Advance past the closing brace
					advance(1);
					
				} else if ((peek(0).type == TOKEN_TYPE_PT_SEMICOLON) && (pParent->type == NODE_TYPE_STATEMENT) && (pParent->tokenList[0].type == TOKEN_TYPE_KW_IMPORT)) {
					
					// Advance past the semicolon
					advance(1);
					
					// We're an external import from some other module, so we don't have a definition
					
					// Proceed to symbol resolution
					
				} else {
					
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_BRACE, currentNode);
					
				}
				
			} else if (peek(0).type == TOKEN_TYPE_PT_SEMICOLON) {
				
				currentNode->type = NODE_TYPE_DECL_VARIABLE;
				
				// Advance past the semicolon
				advance(1);
				
			} else if (token_isOperator(peek(0).type)) {
				
				currentNode->type = NODE_TYPE_DECL_VARIABLE;
				
				// Get the expression
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				
				// We expect a semicolon; in the case that it isn't, push an error
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
			
			// Resolution skip point
			resolve:
			
			// Add this symbol to the symbol table
			if ((currentNode->type == NODE_TYPE_DECL_FUNCTION) || (currentNode->type == NODE_TYPE_DECL_VARIABLE) || (currentNode->type == NODE_TYPE_DECL_PARAMETER)) {
				
				symbol_size symbolSize = SYMBOL_SIZE_BITS_0;
				
				// Evaluate the function type to calculate its size
				size_t count_long = 0;
				size_t count_short = 0;
				
				size_t typeOffset = 0;
				
				while (token_isTypeQualifier(currentNode->tokenList[typeOffset].type) || token_isTypeSpecifier(currentNode->tokenList[typeOffset].type)) {
					typeOffset++;
				}
				
				for (size_t i = 0; i < currentNode->tokenCount; i++) {
					if (currentNode->tokenList[i].type == TOKEN_TYPE_SP_LONG) count_long++;
					if (currentNode->tokenList[i].type == TOKEN_TYPE_SP_SHORT) count_short++;
				}
				
				if (strcmp(currentNode->tokenList[typeOffset].value, "int") == 0) {
					if ((count_long + count_short) == 1) {
						if (count_long == 1) symbolSize = SYMBOL_SIZE_BITS_64;
						if (count_short == 1) symbolSize = SYMBOL_SIZE_BITS_16;
					} else {
						symbolSize = SYMBOL_SIZE_BITS_32;
					}
				} else if (strcmp(currentNode->tokenList[typeOffset].value, "float") == 0) {
					if ((count_long + count_short) == 1) {
						if (count_long == 1) symbolSize = SYMBOL_SIZE_BITS_128;
						if (count_short == 1) symbolSize = SYMBOL_SIZE_BITS_32;
					} else {
						symbolSize = SYMBOL_SIZE_BITS_64;
					}
				} else if (strcmp(currentNode->tokenList[typeOffset].value, "decimal") == 0) {
					if ((count_long + count_short) == 1) {
						if (count_long == 1) symbolSize = SYMBOL_SIZE_BITS_128;
						if (count_short == 1) symbolSize = SYMBOL_SIZE_BITS_32;
					} else {
						symbolSize = SYMBOL_SIZE_BITS_64;
					}
				} else if (strcmp(currentNode->tokenList[typeOffset].value, "byte") == 0) {
					symbolSize = SYMBOL_SIZE_BITS_8;
				}
				
				symbol* currentSymbol = NULL;
				
				if (currentNode->type == NODE_TYPE_DECL_FUNCTION) {
					currentSymbol = symbol_add(pSymbolTable, currentNode->tokenList[currentNode->tokenCount - 1].value, SYMBOL_TYPE_FUNCTION, symbolSize, *pScopeIndex, SYMBOL_CLASS_FUNCTION);
				} else if (currentNode->type == NODE_TYPE_DECL_VARIABLE) {
					currentSymbol = symbol_add(pSymbolTable, currentNode->tokenList[currentNode->tokenCount - 1].value, SYMBOL_TYPE_VARIABLE, symbolSize, *pScopeIndex, SYMBOL_CLASS_VARIABLE);
				} else if (currentNode->type == NODE_TYPE_DECL_PARAMETER) {
					currentSymbol = symbol_add(pSymbolTable, currentNode->tokenList[currentNode->tokenCount - 1].value, SYMBOL_TYPE_VARIABLE, symbolSize, *pScopeIndex, SYMBOL_CLASS_VARIABLE);
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
			
			// Advance past the last token
			advance(1);
			
			// Return this invalid node
			return currentNode;
			
		}
		
		return currentNode;
		
	}
	
	// Handle keywords
	if (token_isKeyword(startToken->type)) {
		
		currentNode->type = NODE_TYPE_STATEMENT;
		
		switch (startToken->type) {
			
			case (TOKEN_TYPE_KW_RETURN) {
				
				// Advance past the keyword
				advance(1);
				
				// Get the expression associated with this keyword
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				
				// We expect a semicolon; in the case it isn't, push an error
				if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
				else
					advance(1);
				
			} break;
			
			case (TOKEN_TYPE_KW_EXPORT) {
				
				// Advance past the keyword
				advance(1);
				
				// Get the function, variable, or module associated with this keyword
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				
				// We only expect a semicolon in the case of modules; functions and variables handle themselves
				if (currentNode->firstChild->tokenList[0].type == TOKEN_TYPE_KW_MODULE) {
					if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
						error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
					else
						advance(1);
				}
				
			} break;
			
			case (TOKEN_TYPE_KW_IMPORT) {
				
				// Advance past the keyword
				advance(1);
				
				// Get the function, variable, or module associated with this keyword
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				
				// We expect a semicolon; in the case it isn't, push an error
				if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
				else
					advance(1);
				
			} break;
			
			case (TOKEN_TYPE_KW_MODULE) {
				
				// Advance past the keyword
				advance(1);
			
				// Get the function, variable, or module associated with this keyword
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				
			} break;
			
			case (TOKEN_TYPE_KW_WHILE) {
				
				// Advance past the keyword
				advance(1);
				
				// We expect an open paren; in the case it isn't, push an error and panic
				if (peek(0).type != TOKEN_TYPE_PT_OPEN_PAREN) {
					
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_PAREN, currentNode);
					
					currentNode->type = NODE_TYPE_INVALID;
					
					panic(TOKEN_TYPE_PT_CLOSE_PAREN);
					
					// Advance past the closing paren
					advance(1);
					
					// We hope that the scope exists and that it properly gets parsed
					
					// Return this invalid node
					return currentNode;
					
				} else
					advance(1);
				
				// Create a new condition node
				node* conditionNode = node_new(NODE_TYPE_CONDITION, currentNode);
				currentNode->firstChild = conditionNode;
				
				// Get the condition
				conditionNode->firstChild = node_parse(pStream, conditionNode, pSymbolTable, pErrorTable, pScopeIndex);
				
				// We expect a closing paren; in the case it isn't, push an error and panic
				if (peek(0).type != TOKEN_TYPE_PT_CLOSE_PAREN) {
					
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_PAREN, currentNode);
					
					currentNode->type = NODE_TYPE_INVALID;
					
				} else
					advance(1);
				
				// Create a new scope node
				node* scopeNode = node_new(NODE_TYPE_SCOPE, currentNode);
				conditionNode->nextSibling = scopeNode;
				
				// Increment the scope index
				(*pScopeIndex)++;
				currentNode->scopeIndex = *pScopeIndex;
				
				// Check if this is a one-liner; if not, parse a whole body until the closing brace
				if (peek(0).type == TOKEN_TYPE_PT_OPEN_BRACE) {
					
					// Advance past the open brace
					advance(1);
					
					// Make sure that there is a populated body; in the case that there isn't, push an error
					if (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
						
						// Get the first child of this node
						scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
						node* bodyNode = scopeNode->firstChild;
						
						// Get the rest of the children
						while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
							
							bodyNode->nextSibling = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
							bodyNode = bodyNode->nextSibling;
							
						}
						
					} else {
						
						error_table_push(pErrorTable, ERROR_SEMANTIC_UNPOPULATED_BODY, currentNode);
					
						currentNode->type = NODE_TYPE_INVALID;
						
						// No further logic occurs other than advancing past the closing brace
						
					}
					
					// Advance past the closing brace
					advance(1);
					
				} else {
					
					scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
					
					// The statement we just parsed should have resolved its semicolon, and the error is already pushed if it was not present. Semicolon checks can be omitted here
					
				}
				
			} break;
			
			case (TOKEN_TYPE_KW_IF) {
				
				// Advance past the keyword
				advance(1);
				
				// Create a new condition node
				node* conditionNode = node_new(NODE_TYPE_CONDITION, currentNode);
				currentNode->firstChild = conditionNode;
				
				// We run a loop to check for multiple conditions until we encounter the last one
				while (1) {
					
					// Be sure this isn't an else condition; if it is, then it doesn't have a condition and this can be skipped
					if (conditionNode->type != NODE_TYPE_CONDITION_ELSE) {
						
						// We expect an open paren; in the case that it isn't, push an error and panic
						if (peek(0).type != TOKEN_TYPE_PT_OPEN_PAREN) {
						
							error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_PAREN, currentNode);
							
							currentNode->type = NODE_TYPE_INVALID;
							
							panic(TOKEN_TYPE_PT_CLOSE_PAREN);
							
							// Advance past the closing paren
							advance(1);
							
							// We hope that a scope exists and that it properly gets parsed
							
							// Return the invalid node
							return currentNode;
							
						} else
							advance(1);
						
						// Get the condition
						conditionNode->firstChild = node_parse(pStream, conditionNode, pSymbolTable, pErrorTable, pScopeIndex);
						
						// We expect a closing paren; in the case that it isn't, push an error and panic
						if (peek(0).type != TOKEN_TYPE_PT_CLOSE_PAREN) {
							
							error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_PAREN, currentNode);
							
							currentNode->type = NODE_TYPE_INVALID;
							
						}
						
						// Advance past the closing paren
						advance(1);
						
					}
					
					// Create a new scope node
					node* scopeNode = node_new(NODE_TYPE_SCOPE, currentNode);
					conditionNode->nextSibling = scopeNode;
					
					// Increment the scope index
					(*pScopeIndex)++;
					currentNode->scopeIndex = *pScopeIndex;
					
					// Check if this is a one-liner; if not, parse a whole body until the closing brace
					if (peek(0).type == TOKEN_TYPE_PT_OPEN_BRACE) {
						
						// Advance past the open brace
						advance(1);
						
						// Make sure that there is a populated body; in the case that there isn't, push an error
						if (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
							
							// Get the first child of this node
							scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
							node* bodyNode = scopeNode->firstChild;
							
							// Get the rest of the children
							while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
								
								bodyNode->nextSibling = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
								bodyNode = bodyNode->nextSibling;
								
							}
							
						} else {
							
							error_table_push(pErrorTable, ERROR_SEMANTIC_UNPOPULATED_BODY, currentNode);
						
							currentNode->type = NODE_TYPE_INVALID;
							
							// No further logic occurs other than advancing past the closing brace
							
						}
						
						// Advance past the closing brace
						advance(1);
						
					} else {
						
						scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
						
						// The statement we just parsed should have resolved its semicolon, and the error is already pushed if it was not present. Semicolon checks can be omitted here
						
					}
					
					// Check if there is another condition to resolve; if not, we're done parsing this if statement
					if (peek(0).type == TOKEN_TYPE_KW_ELSE) {
						
						// Advance past the keyword
						advance(1);
						
						// Create a new condition node
						scopeNode->nextSibling = node_new(NODE_TYPE_CONDITION, currentNode);
						conditionNode = scopeNode->nextSibling;
						
						// Check if this is an else if or an else
						if (peek(0).type == TOKEN_TYPE_KW_IF)
							advance(1);
						else
							conditionNode->type = NODE_TYPE_CONDITION_ELSE;
						
					} else {
						
						// Break out of the loop
						break;
						
					}
					
				}
				
			} break;
			
			case (TOKEN_TYPE_KW_SWITCH) {
				
				// Advance past the keyword
				advance(1);
				
				// We expect ann open paren; in the case that it isn't, push an error and panic
				if (peek(0).type != TOKEN_TYPE_PT_OPEN_PAREN) {
					
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_PAREN, currentNode);
					
					currentNode->type = NODE_TYPE_INVALID;
					
					panic(TOKEN_TYPE_PT_CLOSE_PAREN);
					
					// Advance past the closing paren
					advance(1);
					
					// We hope that a scope still exists and that it properly gets parsed
					
					// Return the invalid node
					return currentNode;
					
				} else
					advance(1);
				
				// Create a new identifier node
				node* expressionNode = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				currentNode->firstChild = expressionNode;
				
				advance(1);
				
				// Create a new scope node
				node* scopeNode = node_new(NODE_TYPE_SCOPE, currentNode);
				expressionNode->nextSibling = scopeNode;
				
				// Increment the scope index
				(*pScopeIndex)++;
				currentNode->scopeIndex = *pScopeIndex;
				
				// Get all the cases under this switch statement; if there are none, push an error
				if (peek(0).type == TOKEN_TYPE_PT_OPEN_BRACE) {
					
					// Advance past the open brace
					advance(1);
					
					// Make sure that there is a populated body; in the case that there isn't, push an error
					if (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
						
						// Get the first child of this node
						scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
						node* thisNode = scopeNode->firstChild;
						
						// Get the rest of the children
						while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
							
							thisNode->nextSibling = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
							thisNode = thisNode->nextSibling;
							
						}
						
					} else {
						
						error_table_push(pErrorTable, ERROR_SEMANTIC_UNPOPULATED_BODY, currentNode);
					
						currentNode->type = NODE_TYPE_INVALID;
						
						// No further logic occurs other than advancing past the closing brace
						
					}
					
					// Advance past the closing brace
					advance(1);
					
				} else {
					
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_BRACE, currentNode);
					
				}
				
			} break;
			
			case (TOKEN_TYPE_KW_CASE) {
				
				// Advance past the keyword
				advance(1);
				
				// We expect an open paren; in the case that it isnt, push an error and panic
				if (peek(0).type != TOKEN_TYPE_PT_OPEN_PAREN) {
					
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_PAREN, currentNode);
					
					currentNode->type = NODE_TYPE_INVALID;
					
					panic(TOKEN_TYPE_PT_CLOSE_PAREN);
					
					// Advance past the closing paren
					advance(1);
					
					// We hope that a scope still exists and that it properly gets parsed
					
					// Return the invalid node
					return currentNode;
					
				} else
					advance(1);
				
				// Parse the expression node
				node* expressionNode = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				currentNode->firstChild = expressionNode;
				
				// Advance past the closing paren
				advance(1);
				
				// Create a new scope node
				node* scopeNode = node_new(NODE_TYPE_SCOPE, currentNode);
				currentNode->firstChild = scopeNode;
				
				// Increment the scope index
				(*pScopeIndex)++;
				currentNode->scopeIndex = *pScopeIndex;
				
				// Check if this is a one-liner; if not, parse a whole body until the closing brace
				if (peek(0).type == TOKEN_TYPE_PT_OPEN_BRACE) {
					
					// Advance past the open brace
					advance(1);
					
					// Make sure that there is a populated body; in the case that there isn't, push an error
					if (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
						
						// Get the first child of this node
						scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
						node* bodyNode = scopeNode->firstChild;
						
						// Get the rest of the children
						while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
							
							bodyNode->nextSibling = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
							bodyNode = bodyNode->nextSibling;
							
						}
						
					} else {
						
						error_table_push(pErrorTable, ERROR_SEMANTIC_UNPOPULATED_BODY, currentNode);
					
						currentNode->type = NODE_TYPE_INVALID;
						
						// No further logic occurs other than advancing past the closing brace
						
					}
					
					// Advance past the closing brace
					advance(1);
					
				} else {
					
					scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
					
					// The statement we just parsed should have resolved its semicolon, and the error is already pushed if it was not present. Semicolon checks can be omitted here
					
				}
				
			} break;
			
			case (TOKEN_TYPE_KW_ELSE) {
				
				// The only time we realistically should ever end up here is if we are in a switch statement
				
				// Advance past the keyword
				advance(1);
				
				// Create a new scope node
				node* scopeNode = node_new(NODE_TYPE_SCOPE, currentNode);
				currentNode->firstChild = scopeNode;
				
				// Increment the scope index
				(*pScopeIndex)++;
				currentNode->scopeIndex = *pScopeIndex;
				
				// Check if this is a one-liner; if not, parse a whole body until the closing brace
				if (peek(0).type == TOKEN_TYPE_PT_OPEN_BRACE) {
					
					// Advance past the open brace
					advance(1);
					
					// Make sure that there is a populated body; in the case that there isn't, push an error
					if (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
						
						// Get the first child of this node
						scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
						node* bodyNode = scopeNode->firstChild;
						
						// Get the rest of the children
						while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
							
							bodyNode->nextSibling = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
							bodyNode = bodyNode->nextSibling;
							
						}
						
					} else {
						
						error_table_push(pErrorTable, ERROR_SEMANTIC_UNPOPULATED_BODY, currentNode);
					
						currentNode->type = NODE_TYPE_INVALID;
						
						// No further logic occurs other than advancing past the closing brace
						
					}
					
					// Advance past the closing brace
					advance(1);
					
				} else {
					
					scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
					
					// The statement we just parsed should have resolved its semicolon, and the error is already pushed if it was not present. Semicolon checks can be omitted here
					
				}
				
			} break;
			
			case (TOKEN_TYPE_KW_NAMESPACE) {
				
				// Advance past the keyword
				advance(1);
				
				// Get the identifier for this namespace
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				
				// We expect an opening brace; in the case that it isn't, push an error and panic
				if (peek(0).type == TOKEN_TYPE_PT_OPEN_BRACE) {
					
					// Advance past the open brace
					advance(1);
					
					// Make sure that there is a populated body; in the case that there isn't, push an error
					if (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
						
						// Get the first child of this node
						node* bodyNode = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
						currentNode->firstChild = bodyNode;
						
						// Get the rest of the children
						while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
							
							bodyNode->nextSibling = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
							bodyNode = bodyNode->nextSibling;
							
						}
						
					} else {
						
						error_table_push(pErrorTable, ERROR_SEMANTIC_UNPOPULATED_BODY, currentNode);
						
						currentNode->type = NODE_TYPE_INVALID;
						
						// Remaining logic is handled outside this scope
						
					}
					
					// Advance past the closing brace
					advance(1);
					
				} else {
				
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_BODY, currentNode);
					
					panic(TOKEN_TYPE_PT_CLOSE_BRACE);
					
					// No further logic occurs other than advancing past the closing brace
					
				}
				
			} break;
			
			case (TOKEN_TYPE_KW_JUMP) {
				
				// Advance past the keyword
				advance(1);
				
				// We expect an identifier node; it is checked later in semantic analysis
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				
				// Identifiers do not advance past their semicolons, so it's up to us to check it; if there isn't one, push an error
				if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
				else
					advance(1);
				
			} break;
			
			case (TOKEN_TYPE_KW_FOR) {
				
				// Advance past the keyword
				advance(1);
				
				// We expect an open paren; in the case that it isn't, push an error and panic
				if (peek(0).type != TOKEN_TYPE_PT_OPEN_PAREN) {
					
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_PAREN, currentNode);
					
					currentNode->type = NODE_TYPE_INVALID;
					
					panic(TOKEN_TYPE_PT_CLOSE_PAREN);
					
					// Advance past the closing paren
					advance(1);
					
					// From here, we hope that a scope will be resolved; it is simply now detached from a parenting for loop
					
					// Return this invalid node
					return currentNode;
					
				} else
					advance(1);
				
				// The this node tracks the current node we are on throughout parsing the for loop
				node* thisNode = NULL;
				
				// We expect an initializer node; in the case there isn't one, just skip it
				if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON) {
					
					currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
					thisNode = currentNode->firstChild;
					
					// The initializer we just parsed should have resolved its semicolon, and the error is already pushed if it was not present. Semicolon checks can be omitted here
					
				} else
					advance(1);
				
				// We expect a condition node; in the case there isn't one, just skip it
				if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON) {
					
					// Create a new condition node
					node* conditionNode = node_new(NODE_TYPE_CONDITION, currentNode);
					
					// If there is a previous node, link it to this one
					if (thisNode) {
						thisNode->nextSibling = conditionNode;
						thisNode = thisNode->nextSibling;
					} else {
						currentNode->firstChild = conditionNode;
						thisNode = currentNode->firstChild;
					}
					
					// Parse the condition
					conditionNode->firstChild = node_parse(pStream, conditionNode, pSymbolTable, pErrorTable, pScopeIndex);
					
					// Conditions do not advance past their semicolons, so it's up to us to check it; if there isn't one, push an error
					if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON) {
						
						error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
						
						currentNode->type = NODE_TYPE_INVALID;
						
					} else
						advance(1);
					
				} else
					advance(1);
				
				// Get the variable modification if there is one
				if (peek(0).type != TOKEN_TYPE_PT_CLOSE_PAREN) {
					
					// We expect an operation node; in the case there isn't one, just skip it; if there is a previous node, link it to this one
					if (thisNode) {
						thisNode->nextSibling = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
						thisNode = thisNode->nextSibling;
					} else {
						currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
						thisNode = currentNode->firstChild;
					}
					
					// Operation nodes do not advance past their semicolons, so it's up to us to check it; if there isn't one, push an error
					if (peek(0).type != TOKEN_TYPE_PT_CLOSE_PAREN) {
					
						error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_PAREN, currentNode);
						
						currentNode->type = NODE_TYPE_INVALID;
						
					} else
						advance(1);
					
				}
				
				// Create a new scope node
				node* scopeNode = node_new(NODE_TYPE_SCOPE, currentNode);
				
				// Increment the scope index
				(*pScopeIndex)++;
				currentNode->scopeIndex = *pScopeIndex;
				
				// If there is a previous node, link it to this one
				if (thisNode) {
					thisNode->nextSibling = scopeNode;
					thisNode = thisNode->nextSibling;
				} else {
					currentNode->firstChild = scopeNode;
					thisNode = currentNode->firstChild;
				}
				
				// Check if this is a one-liner; if not, parse a whole body until the closing brace
				if (peek(0).type == TOKEN_TYPE_PT_OPEN_BRACE) {
					
					// Advance past the open brace
					advance(1);
					
					// Make sure that there is a populated body; in the case that there isn't, push an error
					if (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
						
						// Get the first child of this node
						scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
						node* bodyNode = scopeNode->firstChild;
						
						// Get the rest of the children
						while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
							
							bodyNode->nextSibling = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
							bodyNode = bodyNode->nextSibling;
							
						}
						
					} else {
						
						error_table_push(pErrorTable, ERROR_SEMANTIC_UNPOPULATED_BODY, currentNode);
					
						currentNode->type = NODE_TYPE_INVALID;
						
						// No further logic occurs other than advancing past the closing brace
						
					}
					
					// Advance past the closing brace
					advance(1);
					
				} else {
					
					scopeNode->firstChild = node_parse(pStream, scopeNode, pSymbolTable, pErrorTable, pScopeIndex);
					
					// The statement we just parsed should have resolved its semicolon, and the error is already pushed if it was not present. Semicolon checks can be omitted here
					
				}
				
			} break;
			
			case (TOKEN_TYPE_KW_USING) {
				
				// Advance past the keyword
				advance(1);
				
				// Get the identifier that we are using
				currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				node* thisNode = currentNode->firstChild;
				
				// We should end up on a colon, as it separates the identifier from what we are using; in case that it isn't, push an error and panic
				if (peek(0).type != TOKEN_TYPE_PT_COLON) {
					
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_COLON, currentNode);
					
					currentNode->type = NODE_TYPE_INVALID;
					
					panic(TOKEN_TYPE_PT_SEMICOLON);
					
					// Advance past the semicolon
					advance(1);
					
					// Return this invalid node
					return currentNode;
					
				} else
					advance(1);
				
				// Now get the thing we are using
				thisNode->nextSibling = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				
				// The thing we are using should have resolved its semicolon, and the error is already pushed if it was not present. Semicolon checks can be omitted here
				
			} break;
			
			// Handle structs, unions, and enums
			case (TOKEN_TYPE_KW_STRUCT)
			case (TOKEN_TYPE_KW_ENUM)
			case (TOKEN_TYPE_KW_UNION) {
				
				// Advance past the keyword
				advance(1);
				
				// We expect an open brace; if there isn't one, push an error and panic
				if (peek(0).type != TOKEN_TYPE_PT_OPEN_BRACE) {
					
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_BRACE, currentNode);
					
					currentNode->type = NODE_TYPE_INVALID;
					
					panic(TOKEN_TYPE_PT_CLOSE_BRACE);
					
					// We expect a semicolon; if there isn't one, push an error
					if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
						error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
					else
						advance(1);
					
					// Return this invalid node
					return currentNode;
					
				} else
					advance(1);
				
				// Make sure that there is a populated body; in the case that there isn't, push an error
				if (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
					
					// Get the first child of this node
					node* bodyNode = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
					currentNode->firstChild = bodyNode;
					
					// Get the rest of the children
					while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
						
						bodyNode->nextSibling = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
						bodyNode = bodyNode->nextSibling;
						
					}
					
				} else {
					
					error_table_push(pErrorTable, ERROR_SEMANTIC_UNPOPULATED_BODY, currentNode);
					
					currentNode->type = NODE_TYPE_INVALID;
					
					// Remaining logic is handled outside this scope
					
				}
				
				// Advance past the closing brace
				advance(1);
				
				// We expect a semicolon; if there isn't one, push an error
				if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
				else
					advance(1);
				
			} break;
			
			// The single-line statements
			case (TOKEN_TYPE_KW_BREAK)
			case (TOKEN_TYPE_KW_CONTINUE)
			case (TOKEN_TYPE_KW_EXIT) {
				
				// Advance past the keyword
				advance(1);
				
				// We expect a semicolon; if there isn't one, push an error
				if (peek(0).type != TOKEN_TYPE_PT_SEMICOLON)
					error_table_push(pErrorTable, ERROR_SYNTACTIC_MISSING_SEMICOLON, currentNode);
				else
					advance(1);
				
			} break;
			
		}
		
		return currentNode;
		
	}
	
	// Handle scopes
	if (startToken->type == TOKEN_TYPE_PT_OPEN_BRACE) {
		
		// Advance past the open brace
		advance(1);
		
		// Create a new scope node
		currentNode->type = NODE_TYPE_SCOPE;
		
		// Increment the scope index
		(*pScopeIndex)++;
		currentNode->scopeIndex = *pScopeIndex;
		
		// Make sure that there is a populated body; in the case that there isn't, push an error
		if (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
			
			// Get the first child of this node
			currentNode->firstChild = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
			node* bodyNode = currentNode->firstChild;
			
			// Get the rest of the children
			while (peek(0).type != TOKEN_TYPE_PT_CLOSE_BRACE) {
				
				bodyNode->nextSibling = node_parse(pStream, currentNode, pSymbolTable, pErrorTable, pScopeIndex);
				bodyNode = bodyNode->nextSibling;
				
			}
			
		} else {
			
			error_table_push(pErrorTable, ERROR_SEMANTIC_UNPOPULATED_BODY, currentNode);
			
			currentNode->type = NODE_TYPE_INVALID;
			
			// Remaining logic is handled outside this scope
			
		}
		
		// Advance past the closing brace
		advance(1);
		
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
	if (!fileNode) return false;
	
	// Parse the stream into the AST
	pInfo->pStream->index = 0;
	pAST->scopeIndex = 0;
	
	// Parse the first node
	node* thisNode = node_parse(pInfo->pStream, fileNode, pInfo->pSymbolTable, pInfo->pErrorTable, &pAST->scopeIndex);
	if (!thisNode) return false;
	fileNode->firstChild = thisNode;
	
	// Parse every other node
	while (pInfo->pStream->index < pInfo->pStream->size) {
		thisNode->nextSibling = node_parse(pInfo->pStream, fileNode, pInfo->pSymbolTable, pInfo->pErrorTable, &pAST->scopeIndex);
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
