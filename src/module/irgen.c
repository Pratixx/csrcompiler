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

unit* unit_push(ir* pIR, unit_type type);
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

unit_type eval_type_size_from_num(size_t size) {
	
	if (size == 0) return UNIT_TYPE_TP_VOID;
	
	if ((size > 32) && (size <= 64))
		return UNIT_TYPE_TP_S64;
	else if ((size > 16) && (size <= 32))
		return UNIT_TYPE_TP_S32;
	else if ((size > 8) && (size <= 16))
		return UNIT_TYPE_TP_S16;
	else if ((size > 0) && (size <= 8))
		return UNIT_TYPE_TP_S8;
	else
		return UNIT_TYPE_TP_UK;
	
}

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
		
		// Skip to the variable name
		size_t index = 0;
		while ((token_isTypeQualifier(pNode->tokenList[index].type) || token_isTypeSpecifier(pNode->tokenList[index].type))) index++;
		index++;
		while ((token_isTypeQualifier(pNode->tokenList[index].type) || token_isTypeSpecifier(pNode->tokenList[index].type))) index++;
		
		// Check that this symbol exists
		symbol* foundSymbol = symbol_find(pSymbolTable, pNode->tokenList[index].value, SYMBOL_CLASS_ALL);
		if (foundSymbol) {
			
			return eval_type_size_from_num(foundSymbol->size);
			
		}
		
	}
	
	// Return an unknown type if we found no symbol
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
		
		switch (pNode->tokenList->type) {
			
			case (TOKEN_TYPE_OP_INC) {
				
				flat[*index] = 'I';
				nodes[*index] = pNode;
				(*index)++;
				
			} break;
			
			case (TOKEN_TYPE_OP_DEC) {
				
				flat[*index] = 'D';
				nodes[*index] = pNode;
				(*index)++;
				
			} break;
			
			default: {
				
				if (pNode->firstChild) {
					expr_flatten(pNode->firstChild, flat, nodes, index, true, false);
					if (pNode->firstChild->nextSibling)
						expr_flatten(pNode->firstChild->nextSibling, flat, nodes, index, false, false);
				}
				
				if (registers == 2) {
					
					flat[*index] = '^';
					(*index)++;
					
					flat[*index] = pNode->parent->tokenList->value[0];
					(*index)++;
					
					registers--;
					
				}
				
			} break;
			
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
		
		// Reset the register count for the next expression pass
		registers = 0;
		
	}
	
}

void expr_parse(ir* pIR, char flat[], node* nodes[], size_t index, node* pNode) {
	
	static size_t registers = 0;
	unit* outReg = NULL;
	
	for (size_t i = 0; i < index; i++) {
		
		switch (flat[i]) {
			
			case ('~') {
				
				registers++;
				
				unit_push(pIR, UNIT_TYPE_KW_MOVE);
				unit_push(pIR, UNIT_TYPE_TP_S32);
				outReg = unit_push(pIR, eval_register(registers));
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
			
			case ('I') {
				unit_push(pIR, UNIT_TYPE_KW_INC);
				unit_push(pIR, UNIT_TYPE_TP_S32);
				unit_push(pIR, UNIT_TYPE_IDENTIFIER);
				unit_copy(pIR, pNode->parent, 0);
				unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
			} break;

			case ('D') {
				unit_push(pIR, UNIT_TYPE_KW_DEC);
				unit_push(pIR, UNIT_TYPE_IDENTIFIER);
				unit_copy(pIR, pNode->parent, 0);
				unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
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
	
	// If we loaded an identifier into a register at the start of this expression, then reassign it this value
	if ((pNode->parent->type == NODE_TYPE_IDENTIFIER) && (index > 1)) {
		
		unit_push(pIR, UNIT_TYPE_KW_MOVE);
		outReg = unit_push(pIR, UNIT_TYPE_IDENTIFIER);
		unit_copy(pIR, pNode->parent, 0);
		unit_push(pIR, UNIT_TYPE_PT_COMMA);
		unit_push(pIR, UNIT_TYPE_TP_S32);
		unit_push(pIR, eval_register(registers));
		unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
		
	}
	
	registers = 0;
	
	// Set the last register; the result of an expression is always in AR1
	pIR->info.pRet = outReg;
	
}

void emit_expr(ir* pIR, node* pNode, unit_type reg) {
	
	// IMPORTANT NOTE / TODO: Make these realloc instead
	
	char flat[256] = {};
	node* nodes[256] = {};
	size_t index = 0;
	
	// Turn our expression into a register-like array
	expr_flatten(pNode, flat, nodes, &index, false, true);
	
	for (size_t i = 0; i < index; i++) print_utf8("%c ", flat[i]);
	print_utf8("\n");
	
	// Parse the expression into IR instructions
	expr_parse(pIR, flat, nodes, index, pNode);
	
}

/*////////*/

unit* unit_push(ir* pIR, unit_type type) {
	
	// Allocate a buffer for the stream
	if (pIR->size == pIR->memSize) if (ir_resize(pIR) == false) return NULL;
	
	// Add a new unit to the buffer
	pIR->buffer[pIR->size].type = type;
	pIR->buffer[pIR->size].value[0] = '\0';
	
	// Increment the size
	(pIR->size)++;
	
	// Return the unit
	return &pIR->buffer[pIR->size - 1];
	
}

void unit_copy(ir* pIR, node* pNode, size_t index) {
	strcpy(pIR->buffer[pIR->size - 1].value, pNode->tokenList[index].value);
}

void unit_parse(ir* pIR, node* pNode, symbol_table* pSymbolTable) {
	
	switch (pNode->type) {
		
		case (NODE_TYPE_SCOPE)
		case (NODE_TYPE_FILE) {
			
			// If a stack frame should be allocated, then allocate one
			bool allocFrame = pIR->info.allocFrame;
			if (allocFrame) {
				
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
				
			}
			
			// Parse all of our children
			for (node* thisNode = pNode->firstChild; thisNode; thisNode = thisNode->nextSibling)
				unit_parse(pIR, thisNode, pSymbolTable);
			
			if (pNode->type == NODE_TYPE_FILE) {
				unit_push(pIR, UNIT_TYPE_KW_END);
			} else if (allocFrame) {
				unit_push(pIR, UNIT_TYPE_KW_FREE);
				unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
			}
			
		} break;
		
		case (NODE_TYPE_DECL_FUNCTION) {
			
			// Get the return type of this function
			unit_type retType = eval_type_size(pNode, pSymbolTable);
			
			// Emit a function declaration node
			unit_push(pIR, UNIT_TYPE_KW_FUNC);
			
			// Emit the function return type
			unit_push(pIR, retType);
			
			// Emit the function name; we need to skip the type to get the variable name
			size_t index = 0;
			while (!token_isIdentifier(pNode->tokenList[index].type)) index++;
			index++;
			while (!token_isIdentifier(pNode->tokenList[index].type)) index++;
			
			unit_push(pIR, UNIT_TYPE_IDENTIFIER);
			unit_copy(pIR, pNode, index);
			
			// Set the function info for later reference
			pIR->info.thisFunc.name = pNode->tokenList[index].value;
			pIR->info.thisFunc.retType = retType;
			
			// Emit a colon delimiter
			unit_push(pIR, UNIT_TYPE_PT_COLON);
			
			// Emit the function parameters; if it has none, this loop will not run
			node* thisNode = pNode->firstChild;
			while ((thisNode) && (thisNode->type == NODE_TYPE_DECL_PARAMETER)) {
				
				// Emit the parameter type
				unit_push(pIR, eval_type_size(thisNode, pSymbolTable));
				
				// Emit the variable name; we need to skip the type to get the variable name
				
				size_t index = 0;
				while (!token_isIdentifier(thisNode->tokenList[index].type)) index++;
				index++;
				while (!token_isIdentifier(thisNode->tokenList[index].type)) index++;
				
				// Emit the identifier
				unit_push(pIR, UNIT_TYPE_IDENTIFIER);
				unit_copy(pIR, thisNode, index);
				
				// Advance to the next node
				thisNode = thisNode->nextSibling;
				
				// Emit the appropriate syntax
				if (thisNode)
					if (thisNode->type == NODE_TYPE_SCOPE)
						unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
					else
						unit_push(pIR, UNIT_TYPE_PT_COMMA);
				
			}
			
			// Check if this function has a scope and emit it
			if ((thisNode) && (thisNode->type == NODE_TYPE_SCOPE)) {
				pIR->info.allocFrame = true;
				unit_parse(pIR, thisNode, pSymbolTable);
			}
			
		} break;
		
		case (NODE_TYPE_LITERAL) {
			
			// Create a dummy unit to set the return register to
			static unit temp;
			temp.type = UNIT_TYPE_LITERAL;
			strcpy(temp.value, pNode->tokenList[0].value);
			
			// Set the return register to this literal
			pIR->info.pRet = &temp;
			
			pIR->info.pRetNode = pNode;
			pIR->info.pRetNodeIndex = 0;
			
		} break;
		
		case (NODE_TYPE_IDENTIFIER) {
			
			// Get the index of the identifier name
			size_t index = 0;
			while (!token_isIdentifier(pNode->tokenList[index].type)) index++;
			index++;
			while (!token_isIdentifier(pNode->tokenList[index].type)) index++;
			
			// Create a dummy unit to set the return register to
			static unit temp;
			temp.type = UNIT_TYPE_IDENTIFIER;
			strcpy(temp.value, pNode->tokenList[index].value);
			
			// Set the return register to this literal
			pIR->info.pRet = &temp;
			
			pIR->info.pRetNode = pNode;
			pIR->info.pRetNodeIndex = index;
			
			// If this has a child, then emit that operation
			if (pNode->firstChild) {
				
				unit_parse(pIR, pNode->firstChild, pSymbolTable);
				
			}
			
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
					
					// Parse what is being returned
					unit_parse(pIR, pNode->firstChild, pSymbolTable);
					
					// Check if this is a register
					bool isRegister = false;
					if ((pIR->info.pRet[0].type != UNIT_TYPE_IDENTIFIER) && (pIR->info.pRet[0].type != UNIT_TYPE_LITERAL)) isRegister = true;
					
					// Move the last register into retval
					unit_push(pIR, UNIT_TYPE_KW_MOVE);
					unit_push(pIR, pIR->info.thisFunc.retType);
					unit_push(pIR, UNIT_TYPE_RG_RETVAL);
					unit_push(pIR, UNIT_TYPE_PT_COMMA);
					if (isRegister) unit_push(pIR, pIR->info.thisFunc.retType);
					unit_push(pIR, pIR->info.pRet[0].type);
					if (!isRegister) unit_copy(pIR, pIR->info.pRetNode, pIR->info.pRetNodeIndex);
					
					unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
					
					// Emit the return op
					unit_push(pIR, UNIT_TYPE_KW_RETURN);
					unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
					
				} break;
				
				case (TOKEN_TYPE_KW_WHILE) {
					
					// Increment and save the label index
					pIR->info.lblIndex++;
					size_t lblIndex = pIR->info.lblIndex;
					
					// Emit the start label
					unit_push(pIR, UNIT_TYPE_LABEL);
					sprintf(pIR->buffer[pIR->size - 1].value, "func_%s_wloops_%u", pIR->info.thisFunc.name, lblIndex);
					unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
					
					// Emit the condition
					unit_push(pIR, UNIT_TYPE_KW_IF);
					
					// Check if there is only one identifier or literal; in that case, emit a zero check
					if (pNode->firstChild)
						if (pNode->firstChild->firstChild->type == NODE_TYPE_IDENTIFIER) {
							
							unit_push(pIR, UNIT_TYPE_IDENTIFIER);
							unit_copy(pIR, pNode->firstChild->firstChild, 0);
							unit_push(pIR, UNIT_TYPE_KW_CMP_Z);
							
						} else if (pNode->firstChild->firstChild->type == NODE_TYPE_LITERAL) {
							
							unit_push(pIR, UNIT_TYPE_LITERAL);
							unit_copy(pIR, pNode->firstChild->firstChild, 0);
							unit_push(pIR, UNIT_TYPE_KW_CMP_Z);
							
						} else {
							
							// IMPORTANT NOTE: Implement me!
							
						}
					
					unit_push(pIR, UNIT_TYPE_PT_COLON);
					unit_push(pIR, UNIT_TYPE_LABEL);
					sprintf(pIR->buffer[pIR->size - 1].value, "func_%s_wloope_%u", pIR->info.thisFunc.name, lblIndex);
					unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
					
					// Note that we shouldn't make a new stack frame
					pIR->info.allocFrame = false;
					
					// Parse the body
					node* thisNode = pNode->firstChild;
					while (thisNode->type != NODE_TYPE_SCOPE) thisNode = thisNode->nextSibling;
					unit_parse(pIR, thisNode, pSymbolTable);
					
					// Make sure to jump back to the top!
					unit_push(pIR, UNIT_TYPE_KW_JUMP);
					unit_push(pIR, UNIT_TYPE_LABEL);
					sprintf(pIR->buffer[pIR->size - 1].value, "func_%s_wloops_%u", pIR->info.thisFunc.name, lblIndex);
					unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
					
					// Emit the end label
					unit_push(pIR, UNIT_TYPE_LABEL);
					sprintf(pIR->buffer[pIR->size - 1].value, "func_%s_wloope_%u", pIR->info.thisFunc.name, lblIndex);
					unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
					
				} break;
				
			}
			
		} break;
		
		case (NODE_TYPE_OPERATION) {
			
			// This is very likely an expression
			emit_expr(pIR, pNode, UNIT_TYPE_UNDEFINED);
			
		} break;
		
		case (NODE_TYPE_DECL_VARIABLE) {
			
			// Check if this variable is being assigned to a string literal; if it is, it should be emitted statically
			bool literal = false;
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
			
			// If this should be emitted statically, then directly assign the value to the variable
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
			
			// If this has a child, then emit that operation; otherwise, don't emit anything
			if (pNode->firstChild) {
				
				unit_parse(pIR, pNode->firstChild, pSymbolTable);
				
				// Move the result of the last used register into the variable
				unit_push(pIR, UNIT_TYPE_KW_MOVE);
				unit_push(pIR, UNIT_TYPE_IDENTIFIER);
				unit_copy(pIR, pNode, index);
				unit_push(pIR, UNIT_TYPE_PT_COMMA);
				unit_push(pIR, pIR->info.pRet[-1].type);
				unit_push(pIR, pIR->info.pRet[0].type);
				unit_push(pIR, UNIT_TYPE_PT_SEMICOLON);
				
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
	
	while (pIR->index < pIR->size) {
		
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
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_IF) ? "KW_IF" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_INC) ? "KW_INC" :
			(pIR->buffer[pIR->index].type == UNIT_TYPE_KW_DEC) ? "KW_DEC" :
			
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
			(pIR->buffer[pIR->index].type == UNIT_TYPE_LABEL) ? strcat(strcat(strcpy(value, "LABEL ["), pIR->buffer[pIR->index].value), "]") :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_VOID) ? "TP_VOID" :
			
			(pIR->buffer[pIR->index].type == UNIT_TYPE_TP_UK) ? "TP_UK" :
			
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
	
	// Initialize some things
	pIR->info.lblIndex = 0;
	pIR->info.thisFunc.name = "";
	pIR->info.allocFrame = true;
	
	// Parse the file node and let it handle the rest
	unit_parse(pIR, pInfo->pAST->root, pInfo->pSymbolTable);
	
	// Return success
	return true;
	
}