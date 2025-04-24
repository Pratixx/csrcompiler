// [ INCLUDING ] //

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// [ DEFINING ] //



// [ FUNCTIONS ] //


static bool token_isKeyword(token_type tokenType) {
	size_t index = 0;
	while (token_kw_table[index].type != TOKEN_TYPE_UNDEFINED) {
		if (token_kw_table[index].type == tokenType) return true;
		index++;
	}
	return false;
}

static bool token_isLiteral(token_type tokenType) {
	switch (tokenType) {
		case (TOKEN_TYPE_LITERAL_CHAR) return true;
		case (TOKEN_TYPE_LITERAL_STR) return true;
		case (TOKEN_TYPE_LITERAL_INT) return true;
		case (TOKEN_TYPE_LITERAL_INT_HEX) return true;
		case (TOKEN_TYPE_LITERAL_FLOAT) return true;
		default: return false;
	}
}

static bool token_isIdentifier(token_type tokenType) {
	
	// Immediately return true for actual identifiers
	if (tokenType == TOKEN_TYPE_IDENTIFIER) return true;
	return false;
	
}

static bool token_isOperator(token_type tokenType) {
	size_t index = 0;
	while (token_op_table[index].type != TOKEN_TYPE_UNDEFINED) {
		if (token_op_table[index].type == tokenType) return true;
		index++;
	}
	return false;
}

static bool token_isPunctuator(token_type tokenType) {
	size_t index = 0;
	while (token_pt_table[index].type != TOKEN_TYPE_UNDEFINED) {
		if (token_pt_table[index].type == tokenType) return true;
		index++;
	}
	return false;
}

static bool token_isTypeSpecifier(token_type tokenType) {
	size_t index = 0;
	while (token_sp_table[index].type != TOKEN_TYPE_UNDEFINED) {
		if (token_sp_table[index].type == tokenType) return true;
		index++;
	}
	return false;
}

static bool token_isTypeQualifier(token_type tokenType) {
	size_t index = 0;
	while (token_qu_table[index].type != TOKEN_TYPE_UNDEFINED) {
		if (token_qu_table[index].type == tokenType) return true;
		index++;
	}
	return false;
}

/*////////*/

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
					
					size_t val = atoi(pNode->tokenList->value);
					
					if (val <= UINT8_MAX) return UNIT_TYPE_TP_I8;
					if (val <= UINT16_MAX) return UNIT_TYPE_TP_I16;
					if (val <= UINT32_MAX) return UNIT_TYPE_TP_I32;
					if (val <= UINT64_MAX) return UNIT_TYPE_TP_I64;
					
					return UNIT_TYPE_TP_UK;
					
				}
				
			}
			
			return UNIT_TYPE_TP_UK;
			
		}
		
		size_t index = 0;
		
		while ((token_isTypeQualifier(pNode->tokenList[index].type) || token_isTypeSpecifier(pNode->tokenList[index].type)) && !token_isIdentifier(pNode->tokenList[index].type)) index++;
		
		symbol* foundSymbol = symbol_find(pSymbolTable, pNode->tokenList[index].value, SYMBOL_CLASS_ALL);
		
		if (foundSymbol) {
			
			if ((foundSymbol->size > 32) && (foundSymbol->size <= 64))
				return UNIT_TYPE_TP_I64;
			else if ((foundSymbol->size > 16) && (foundSymbol->size <= 32))
				return UNIT_TYPE_TP_I32;
			else if ((foundSymbol->size > 8) && (foundSymbol->size <= 16))
				return UNIT_TYPE_TP_I16;
			else if ((foundSymbol->size > 0) && (foundSymbol->size <= 8))
				return UNIT_TYPE_TP_I8;
			else
				return UNIT_TYPE_TP_UK;
			
		}
		
	}
	
	return UNIT_TYPE_TP_UK;
	
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

void unit_parse(ir* pIR, node* pNode, symbol_table* pSymbolTable) {
	
	switch (pNode->type) {
		
		case (NODE_TYPE_SCOPE)
		case (NODE_TYPE_FILE) {
			
			node* thisNode = pNode->firstChild;
			while (thisNode) {
				
				unit_parse(pIR, thisNode, pSymbolTable);
				
				thisNode = thisNode->nextSibling;
				
			}
			
			if (pNode->type == NODE_TYPE_SCOPE) unit_push(pIR, UNIT_TYPE_KW_END);
			
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
			
			unit_push(pIR, UNIT_TYPE_PT_DOLLAR);
			unit_push(pIR, UNIT_TYPE_IDENTIFIER);
			strcpy(pIR->buffer[pIR->size - 1].value, pNode->tokenList[index].value);
			
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
				
				unit_push(pIR, UNIT_TYPE_PT_POUND);
				unit_push(pIR, UNIT_TYPE_IDENTIFIER);
				strcpy(pIR->buffer[pIR->size - 1].value, pNode->tokenList[index].value);
				
				thisNode = thisNode->nextSibling;
				
				// Emit a comma delimiter
				if (thisNode->type != NODE_TYPE_SCOPE) unit_push(pIR, UNIT_TYPE_PT_COMMA);
				
			}
			
			// Emit a semicolon to end this statement
			unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
			
			// Emit the function scope
			unit_parse(pIR, thisNode, pSymbolTable);
			
		} break;
		
		case (NODE_TYPE_LITERAL) {
			
			// If we are the child of a return statement, search backwards for the function declaration and emit its type
			if ((pNode->parent->type == NODE_TYPE_STATEMENT) && pNode->parent->tokenList->type == TOKEN_TYPE_KW_RETURN) {
				node* thisNode = pNode->parent;
				while (thisNode->type != NODE_TYPE_DECL_FUNCTION) thisNode = thisNode->parent;
				unit_push(pIR, eval_type_size(thisNode, pSymbolTable));
			} else
				unit_push(pIR, eval_type_size(pNode, pSymbolTable));
			
			// Emit the literal
			unit_push(pIR, UNIT_TYPE_LITERAL);
			strcpy(pIR->buffer[pIR->size - 1].value, pNode->tokenList->value);
			
			// Emit a semicolon
			unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
			
		}
		
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
					unit_push(pIR, UNIT_TYPE_KW_RETURN);
					unit_parse(pIR, pNode->firstChild, pSymbolTable);
				}
				
			}
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
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_PUSH) ? "KW_PUSH" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_POP) ? "KW_POP" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_CALL) ? "KW_CALL" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_RETURN) ? "KW_RETURN" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_JUMP) ? "KW_JUMP" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_EXPORT) ? "KW_EXPORT" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_IMPORT) ? "KW_IMPORT" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_IDENTIFIER) ? strcat(strcat(strcpy(value, "IDENTIFIER ["), pIR->buffer[pIR->index].value), "]") :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_PT_DOLLAR) ? "PT_DOLLAR" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_PT_POUND) ? "PT_POUND" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_PT_COMMA) ? "PT_COMMA" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_PT_SEMICOLON) ? "PT_SEMICOLON" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_PT_COLON) ? "PT_COLON" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_LITERAL) ? strcat(strcat(strcpy(value, "LITERAL ["), pIR->buffer[pIR->index].value), "]") :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_I8) ? "TP_I8" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_I16) ? "TP_I16" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_I32) ? "TP_I32" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_I64) ? "TP_I64" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_U8) ? "TP_U8" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_U16) ? "TP_U16" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_U32) ? "TP_U32" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_U64) ? "TP_U64" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_F32) ? "TP_F32" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_F64) ? "TP_F64" :
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