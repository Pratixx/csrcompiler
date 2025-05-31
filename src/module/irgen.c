// [ INCLUDING ] //

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

// [ DEFINING ] //



// [ FUNCTIONS ] //

void unit_push(ir* pIR, unit_type type);
void unit_copy(ir* pIR, node* pNode, size_t index);

bool ir_resize(ir* pIR) { 
	
	// If the buffer can hold more elements, just return
	if (pIR->size < pIR->memSize) return true;
	
	// If there is no buffer, allocate a new buffer
	if (pIR->memSize == 0) {
		
		// Set default buffer size
		pIR->memSize = 8;
		pIR->size = 0;
		
		// Allocate a new buffer
		unit* newBuffer = calloc(pIR->memSize, sizeof(unit));
		if (!newBuffer) return false;
		
		// Assign the new buffer to the old one
		pIR->buffer = newBuffer;
		
		// Return success
		return true;
		
	}
	
	// Allocate a new buffer
	unit* newBuffer = calloc((pIR->memSize * 2), sizeof(unit));
	if (!newBuffer) return false;
	
	// Copy the new memory in and free the old buffer
	memcpy(newBuffer, pIR->buffer, (pIR->memSize * sizeof(unit)));
	free(pIR->buffer);
	
	// Assign the new buffer to the old one
	pIR->memSize *= 2;
	pIR->buffer = newBuffer;
	
	// Return success
	return true;
	
}

/*////////*/

unit_type eval_type_size(node* pNode, symbol_table* pSymbolTable) {
	
	if ((pNode) && (pNode->tokenCount > 0)) {
		
		if (pNode->type == NODE_TYPE_LITERAL) {
			
			switch (pNode->tokenList->type) {
				
				case (TOKEN_TYPE_LITERAL_INT) {
					
					size_t val = strtoull(pNode->tokenList->value, NULL, 0);
					
					if (val <= UINT8_MAX) return UNIT_TYPE_TP_S8;
					if (val <= UINT16_MAX) return UNIT_TYPE_TP_S16;
					if (val <= UINT32_MAX) return UNIT_TYPE_TP_S32;
					if (val <= UINT64_MAX) return UNIT_TYPE_TP_S64;
					
					return UNIT_TYPE_TP_UK;
					
				}
				
			}
			
			return UNIT_TYPE_TP_UK;
			
		}
		
		size_t index = 0;
		
		while ((token_isTypeQualifier(pNode->tokenList[index].type) || token_isTypeSpecifier(pNode->tokenList[index].type)) && !token_isIdentifier(pNode->tokenList[index].type)) index++;
		
		symbol* foundSymbol = symbol_find(pSymbolTable, pNode->tokenList[index].value, SYMBOL_CLASS_ALL);
		
		if (foundSymbol) {
			
			if (strcmp(foundSymbol->identifier, "void") == 0) return UNIT_TYPE_TP_VOID;
			
			if ((foundSymbol->size > 32) && (foundSymbol->size <= 64))
				return UNIT_TYPE_TP_S64;
			else if ((foundSymbol->size > 16) && (foundSymbol->size <= 32))
				return UNIT_TYPE_TP_S32;
			else if ((foundSymbol->size > 8) && (foundSymbol->size <= 16))
				return UNIT_TYPE_TP_S16;
			else if ((foundSymbol->size > 0) && (foundSymbol->size <= 8))
				return UNIT_TYPE_TP_S8;
			else
				return UNIT_TYPE_TP_UK;
			
		}
		
	}
	
	return UNIT_TYPE_TP_UK;
	
}

unit_type eval_arithmetic (node* pNode) {
	switch (pNode->tokenList->type) {
		case (TOKEN_TYPE_OP_ADD) return UNIT_TYPE_KW_ADD;
		case (TOKEN_TYPE_OP_SUB) return UNIT_TYPE_KW_SUB;
		case (TOKEN_TYPE_OP_MUL) return UNIT_TYPE_KW_MUL;
		case (TOKEN_TYPE_OP_DIV) return UNIT_TYPE_KW_DIV;
		default: return UNIT_TYPE_UNDEFINED;
	}
}

unit_type eval_register(size_t count) {
	switch (count) {
		case (1) return UNIT_TYPE_RG_RG1;
		case (2) return UNIT_TYPE_RG_RG2;
		default: return UNIT_TYPE_RG_UK;
	}
}

/*////////*/

void expr_flatten(node* pNode, char flat[], node* nodes[], size_t* index, bool left, bool first) {
	
	static size_t registers = 0;
	
	if (pNode->type == NODE_TYPE_OPERATION) {
		
		expr_flatten(pNode->firstChild, flat, nodes, index, true, false);
		expr_flatten(pNode->firstChild->nextSibling, flat, nodes, index, false, false);
		
		if (registers == 2) {
			
			flat[*index] = '^';
			(*index)++;
			
			flat[*index] = pNode->parent->tokenList->value[0];
			(*index)++;
			
			registers--;
			
		}
		
	} else if (pNode->type == NODE_TYPE_LITERAL) {
		
		if (left) {
			
			registers++;
			
			flat[*index] = '~';
			(*index)++;
			
		} else {
			
			flat[*index] = pNode->parent->tokenList->value[0];
			(*index)++;
			
		}
		
		flat[*index] = 'l';
		nodes[*index] = pNode;
		(*index)++;
		
	} else if (pNode->type == NODE_TYPE_IDENTIFIER) {
		
		if (left) {
			
			registers++;
			
			flat[*index] = '~';
			(*index)++;
			
		} else {
			
			flat[*index] = pNode->parent->tokenList->value[0];
			(*index)++;
			
		}
		
		flat[*index] = 'i';
		nodes[*index] = pNode;
		(*index)++;
		
	}
	
	if (first) {
		registers = 0;
	}
	
}

void expr_parse(ir* pIR, char flat[], node* nodes[], size_t index) {
	
	static size_t registers = 0;
	
	for (size_t i = 0; i < index; i++) {
		
		switch (flat[i]) {
			
			case ('~') {
				
				registers++;
				
				unit_push(pIR, UNIT_TYPE_KW_MOVE);
				unit_push(pIR, UNIT_TYPE_TP_S32);
				unit_push(pIR, eval_register(registers));
				unit_push(pIR, UNIT_TYPE_PT_COMMA);
				
			} break;
			
			case ('l') {
				
				unit_push(pIR, UNIT_TYPE_LITERAL);
				unit_copy(pIR, nodes[i], 0);
				unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
				
			} break;
			
			case ('i') {
				
				unit_push(pIR, UNIT_TYPE_IDENTIFIER);
				unit_copy(pIR, nodes[i], 0);
				unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
				
			} break;
			
			case ('+') {
				
				unit_push(pIR, UNIT_TYPE_KW_ADD);
				unit_push(pIR, UNIT_TYPE_TP_S32);
				unit_push(pIR, eval_register(registers));
				unit_push(pIR, UNIT_TYPE_PT_COMMA);
				
				if (flat[i - 1] == '^') {
					unit_push(pIR, UNIT_TYPE_TP_S32);
					unit_push(pIR, UNIT_TYPE_RG_RG2);
					unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
				}
				
			} break;
			
			case ('-') {
				
				unit_push(pIR, UNIT_TYPE_KW_SUB);
				unit_push(pIR, UNIT_TYPE_TP_S32);
				unit_push(pIR, eval_register(registers));
				unit_push(pIR, UNIT_TYPE_PT_COMMA);
				
				if (flat[i - 1] == '^') {
					unit_push(pIR, UNIT_TYPE_TP_S32);
					unit_push(pIR, UNIT_TYPE_RG_RG2);
					unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
				}
				
			} break;
			
			case ('*') {
				
				unit_push(pIR, UNIT_TYPE_KW_MUL);
				unit_push(pIR, UNIT_TYPE_TP_S32);
				unit_push(pIR, eval_register(registers));
				unit_push(pIR, UNIT_TYPE_PT_COMMA);
				
				if (flat[i - 1] == '^') {
					unit_push(pIR, UNIT_TYPE_TP_S32);
					unit_push(pIR, UNIT_TYPE_RG_RG2);
					unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
				}
				
			} break;
			
			case ('/') {
				
				unit_push(pIR, UNIT_TYPE_KW_DIV);
				unit_push(pIR, UNIT_TYPE_TP_S32);
				unit_push(pIR, eval_register(registers));
				unit_push(pIR, UNIT_TYPE_PT_COMMA);
				
				if (flat[i - 1] == '^') {
					unit_push(pIR, UNIT_TYPE_TP_S32);
					unit_push(pIR, UNIT_TYPE_RG_RG2);
					unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
				}
				
			} break;
			
			case ('^') {
				
				registers--;
				
			} break;
			
		}
		
	}

	registers = 0;
	
	// Set the last register to register 1
	pIR->lastRegister = UNIT_TYPE_RG_RG1;
	pIR->lastRegisterType = UNIT_TYPE_TP_S32;
	
}

void emit_expr(ir* pIR, node* pNode, unit_type reg) {
	
	// IMPORTANT NOTE / TODO: Make these realloc instead
	
	char flat[256] = {};
	node* nodes[256] = {};
	size_t index = 0;
	
	// Turn our expression into a register-like array
	expr_flatten(pNode, flat, nodes, &index, false, true);
	
	for (size_t i = 0; i < index; i++) {
		
		printf("%c ", flat[i]);
		
	}
	
	// Parse the expression into IR instructions
	expr_parse(pIR, flat, nodes, index);
	
}

/*////////*/

void unit_push(ir* pIR, unit_type type) {
	
	// Allocate a buffer for the stream
	if (pIR->size == pIR->memSize) if (ir_resize(pIR) == false) return;
	
	// Add a new unit to the buffer
	pIR->buffer[pIR->size].type = type;
	pIR->buffer[pIR->size].value[0] = '\0';
	
	// Increment the size
	(pIR->size)++;
	
}

void unit_copy(ir* pIR, node* pNode, size_t index) {
	strcpy(pIR->buffer[pIR->size - 1].value, pNode->tokenList[index].value);
}

void unit_parse(ir* pIR, node* pNode, symbol_table* pSymbolTable) {
	
	switch (pNode->type) {
		
		case (NODE_TYPE_SCOPE)
		case (NODE_TYPE_FILE) {
			
			if (pNode->type == NODE_TYPE_SCOPE) {
				
				// Check how many bytes of memory we're going to need to allocate for this scope based on its scope index
				unit_push(pIR, UNIT_TYPE_KW_ALLOC);
				unit_push(pIR, UNIT_TYPE_LITERAL);
				
				// Align up to the nearest 16 bytes, as expected by the ABI
				uint64_t count = (((pSymbolTable->indicesBuffer[pNode->scopeIndex] + 7) >> 3) + 15) & ~15;
				char buf[64];
				itoa(count, buf, 10);
				strcpy(pIR->buffer[pIR->size - 1].value, buf);
				
				unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
				
			}
			
			node* thisNode = pNode->firstChild;
			while (thisNode) {
				
				unit_parse(pIR, thisNode, pSymbolTable);
				
				thisNode = thisNode->nextSibling;
				
			}
			
			if (pNode->type == NODE_TYPE_SCOPE)
				unit_push(pIR, UNIT_TYPE_KW_END);
			else {
				unit_push(pIR, UNIT_TYPE_KW_FREE);
				unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
			}
			
		} break;
		
		case (NODE_TYPE_DECL_FUNCTION) {
			
			// Emit a function declaration node
			unit_push(pIR, UNIT_TYPE_KW_FUNC);
			
			// Emit the function return type
			unit_push(pIR, eval_type_size(pNode, pSymbolTable));
			
			// Emit the function name; we need to skip the type to get the variable name
			size_t index = 0;
			while (!token_isIdentifier(pNode->tokenList[index].type)) index++;
			index++;
			while (!token_isIdentifier(pNode->tokenList[index].type)) index++;
			
			unit_push(pIR, UNIT_TYPE_IDENTIFIER);
			unit_copy(pIR, pNode, index);
			
			// Emit a colon delimiter
			unit_push(pIR, UNIT_TYPE_PT_COLON);
			
			// Emit the function parameters
			node* thisNode = pNode->firstChild;
			while ((thisNode) && (thisNode->type != NODE_TYPE_SCOPE)) {
				
				// Emit the parameter type
				unit_push(pIR, eval_type_size(thisNode, pSymbolTable));
				
				// Emit the function name; we need to skip the type to get the variable name
				index++;
				while (!token_isIdentifier(pNode->tokenList[index].type)) index++;
				index++;
				while (!token_isIdentifier(pNode->tokenList[index].type)) index++;
				
				unit_push(pIR, UNIT_TYPE_IDENTIFIER);
				unit_copy(pIR, pNode, index);
				
				// If we've hit the end of the function declaration, break
				if (!thisNode->nextSibling) break;
				
				thisNode = thisNode->nextSibling;
				
				// Emit a comma delimiter
				if (thisNode->type != NODE_TYPE_SCOPE) unit_push(pIR, UNIT_TYPE_PT_COMMA);
				
			}
			
			// Emit a semicolon to end this statement
			unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
			
			// Check if this function has a scope and emit it
			if (thisNode->type == NODE_TYPE_SCOPE)
				unit_parse(pIR, thisNode, pSymbolTable);
			
		} break;
		
		case (NODE_TYPE_LITERAL) {
			
			// Emit the literal
			unit_push(pIR, UNIT_TYPE_LITERAL);
			unit_copy(pIR, pNode, 0);
			
			// Emit a semicolon
			unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
			
		} break;
		
		case (NODE_TYPE_IDENTIFIER) {
			
			// Just emit the name
			unit_push(pIR, UNIT_TYPE_IDENTIFIER);
			unit_copy(pIR, pNode, 0);
			
		} break;
		
		case (NODE_TYPE_STATEMENT) {
			
			switch (pNode->tokenList->type) {
				
				case (TOKEN_TYPE_KW_EXPORT) {
					unit_push(pIR, UNIT_TYPE_KW_EXPORT);
					unit_parse(pIR, pNode->firstChild, pSymbolTable);
				} break;
				
				case (TOKEN_TYPE_KW_IMPORT) {
					unit_push(pIR, UNIT_TYPE_KW_IMPORT);
					unit_parse(pIR, pNode->firstChild, pSymbolTable);
				} break;
				
				case (TOKEN_TYPE_KW_RETURN) {
					unit_parse(pIR, pNode->firstChild, pSymbolTable);
					unit_push(pIR, UNIT_TYPE_KW_RETURN);
					unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
				} break;
				
			}
			
		} break;
		
		case (NODE_TYPE_OPERATION) {
			
			emit_expr(pIR, pNode, UNIT_TYPE_UNDEFINED);
			
		} break;
		
		case (NODE_TYPE_DECL_VARIABLE) {
			
			bool literal = false;
			
			// Check if this variable is being assigned to a string literal; if it is, it needs unique handling
			if ((pNode->firstChild) && (pNode->firstChild->firstChild) && (pNode->firstChild->firstChild->tokenList->type == TOKEN_TYPE_LITERAL_STR)) {
				
				unit_push(pIR, UNIT_TYPE_KW_STATIC);
				
				literal = true;
				
			} else {
				
				// Emit the local keyword
				unit_push(pIR, UNIT_TYPE_KW_LOCAL);
				
			}
			
			// Skip to the variable's type and emit its size
			size_t index = 0;
			while (!token_isIdentifier(pNode->tokenList[index].type)) index++;
			unit_push(pIR, eval_type_size(pNode, pSymbolTable));
			
			// Advance past the type and to the name
			index++;
			while (!token_isIdentifier(pNode->tokenList[index].type)) index++;
			unit_push(pIR, UNIT_TYPE_IDENTIFIER);
			unit_copy(pIR, pNode, index);
			
			// If this is a literal, then directly assign it to the variable
			if (literal) {
				
				// Emit a syntactic colon
				unit_push(pIR, UNIT_TYPE_PT_COLON);
				
				// Emit the literal
				unit_push(pIR, UNIT_TYPE_LITERAL);
				unit_copy(pIR, pNode->firstChild->firstChild, 0);
				
				// Emit a semicolon
				unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
				
				// Break to prevent further execution
				break;
				
			}
			
			// Emit a semicolon
			unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
			
			// Emit the appropriate operation based on the operator
			if ((pNode->firstChild) && (pNode->firstChild->firstChild)) {
				
				// Emit the assignment operator
				unit_push(pIR, UNIT_TYPE_KW_MOVE);
				unit_push(pIR, UNIT_TYPE_IDENTIFIER);
				unit_copy(pIR, pNode, index);
				unit_push(pIR, UNIT_TYPE_PT_COMMA);
				unit_push(pIR, UNIT_TYPE_LITERAL);
				unit_copy(pIR, pNode->firstChild->firstChild, 0);
				unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
				
				// Emit the value we are assigning to this variable
				
			}
			
		} break;
		
		case (NODE_TYPE_CALL_FUNCTION) {
			
			// Push the arguments onto the stack
			unit_push(pIR, UNIT_TYPE_KW_ARG_PUSH);
			
			// Emit the function arguments into the appropriate registers
			size_t index = 0;
			node* thisNode = pNode->firstChild;
			while (thisNode) {
				
				// Get the appropriate argument register
				unit_type out = UNIT_TYPE_RG_ARG1 + index;
				
				// Emit the movement into the argument register
				unit_push(pIR, UNIT_TYPE_KW_MOVE);
				unit_push(pIR, eval_type_size(thisNode, pSymbolTable));
				unit_push(pIR, out);
				unit_push(pIR, UNIT_TYPE_PT_COMMA);
				unit_push(pIR, UNIT_TYPE_LITERAL);
				unit_copy(pIR, thisNode, 0);
				unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
				
				// Advance to the next sibling
				thisNode = thisNode->nextSibling;
				
			}
			
			// Emit the call keyword
			unit_push(pIR, UNIT_TYPE_KW_CALL);
			
			// Emit the function name
			unit_push(pIR, UNIT_TYPE_IDENTIFIER);
			unit_copy(pIR, pNode, 0);
			
			// Pop the arguments from the stack
			unit_push(pIR, UNIT_TYPE_KW_ARG_POP);
			
		} break;
		
	}
	
}

/*////////*/

void ir_print(ir* pIR) {
	
	(pIR->index) = 0;
	
	while (pIR->index <= pIR->size) {
		
		// This is probably the most disgusting thing I've ever written
		char value[MAX_VALUE_LEN * 2] = {};
		print_utf8("%s ",
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_FUNC) ? "KW_FUNC" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_MOVE) ? "KW_MOVE" :
						
			(pIR->buffer[pIR->index].type == UNIT_TYPE_RG_ARG1) ? "RG_ARG1" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_RG_ARG2) ? "RG_ARG2" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_RG_ARG3) ? "RG_ARG3" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_RG_ARG4) ? "RG_ARG4" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_RG_RG1) ? "RG_RG1" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_RG_RG2) ? "RG_RG2" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_RG_RETVAL) ? "RG_RETVAL" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_CALL) ? "KW_CALL" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_RETURN) ? "KW_RETURN" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_JUMP) ? "KW_JUMP" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_LOCAL) ? "KW_LOCAL" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_STATIC) ? "KW_STATIC" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_ALLOC) ? "KW_ALLOC" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_FREE) ? "KW_FREE" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_ARG_PUSH) ? "KW_ARG_PUSH" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_ARG_POP) ? "KW_ARG_POP" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_ADD) ? "KW_ADD" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_SUB) ? "KW_SUB" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_MUL) ? "KW_MUL" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_DIV) ? "KW_DIV" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_EXPORT) ? "KW_EXPORT" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_IMPORT) ? "KW_IMPORT" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_PT_COMMA) ? "PT_COMMA" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_PT_SEMICOLON) ? "PT_SEMICOLON" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_PT_COLON) ? "PT_COLON" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_IDENTIFIER) ? strcat(strcat(strcpy(value, "IDENTIFIER ["), pIR->buffer[pIR->index].value), "]") :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_LITERAL) ? strcat(strcat(strcpy(value, "LITERAL ["), pIR->buffer[pIR->index].value), "]") :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_VOID) ? "TP_VOID" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_S8) ? "TP_S8" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_S16) ? "TP_S16" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_S32) ? "TP_S32" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_S64) ? "TP_S64" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_U8) ? "TP_U8" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_U16) ? "TP_U16" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_U32) ? "TP_U32" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_U64) ? "TP_U64" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_F32) ? "TP_F32" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_F64) ? "TP_F64" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_END) ? "KW_END" :
			
			"EOF"
			
		);
		
		(pIR->index)++;
		
	}
	print_utf8("\n");
	
}

bool ir_generate(ir* pIR, ir_info* pInfo) {
	
	
	// Allocate a buffer for the stream
	if (ir_resize(pIR) == false) return false;
	
	// Parse the file node and let it handle the rest
	unit_parse(pIR, pInfo->pAST->root, pInfo->pSymbolTable);
	
	// Return success
	return true;
	
}