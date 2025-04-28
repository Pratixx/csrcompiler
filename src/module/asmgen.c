// [ INCLUDING ] //

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

// [ MACROS ] //

#define advance(x) ((pIR->index) += (x))
#define jump(x) (pIR->index) = (x)

#define peek(x) (((pIR->index + (x)) >= pIR->size) ? (unit){UNIT_TYPE_KW_END, ""} : (pIR->buffer[pIR->index + (x)]))
#define upeek(x) (pIR->buffer[pIR->index + (x)])
#define ppeek(x) ((pIR->index >= pIR->size + (x)) ? (unit[]){(unit){UNIT_TYPE_KW_END, ""}} : &(pIR->buffer[pIR->index + (x)]))

// [ DEFINING ] //



// [ FUNCTIONS ] //

bool asm_resize(asm* pAsm, size_t needed) { 
	
	// If the buffer can hold more elements, just return
	if ((pAsm->size + needed) < pAsm->memSize) return true;
	
	// If there is no buffer, allocate a new buffer
	if (pAsm->memSize == 0) {
		
		// Set default buffer size
		pAsm->memSize = 8;
		pAsm->size = 0;
		
		// Allocate a new buffer
		char* newBuffer = calloc(pAsm->memSize, 1);
		if (!newBuffer) return false;
		
		// Assign the new buffer to the old one
		pAsm->buffer = newBuffer;
		
		// Return success
		return true;
		
	}
	
	// Allocate a new buffer
	char* newBuffer = calloc(((pAsm->memSize * 2) + needed), 1);
	if (!newBuffer) return false;
	
	// Copy the new memory in and free the old buffer
	memcpy(newBuffer, pAsm->buffer, pAsm->memSize);
	free(pAsm->buffer);
	
	// Assign the new buffer to the old one
	pAsm->memSize *= 2;
	pAsm->memSize += needed;
	pAsm->buffer = newBuffer;
	
	// Return success
	return true;
	
}

/*////////*/

char* to_reg(unit* pUnit) {
	switch (pUnit[0].type) {
		case (UNIT_TYPE_RG_RG1) {
			switch (pUnit[-1].type) {
				case (UNIT_TYPE_TP_I32) return "r10d";
				case (UNIT_TYPE_TP_I64) return "r10";
				default: return "r10";
			}
		}
		case (UNIT_TYPE_RG_RG2) {
			switch (pUnit[-1].type) {
				case (UNIT_TYPE_TP_I32) return "r11d";
				case (UNIT_TYPE_TP_I64) return "r11";
				default: return "r10";
			}
		}
		case (UNIT_TYPE_RG_RETVAL) {
			switch (pUnit[-1].type) {
				case (UNIT_TYPE_TP_I32) return "eax";
				case (UNIT_TYPE_TP_I64) return "rax";
				default: return "rax";
			}
		};
		case (UNIT_TYPE_LITERAL) return pUnit[0].value;	
		default: "";
	}
}

char* to_arith(unit* pUnit) {
	switch (pUnit[0].type) {
		case (UNIT_TYPE_KW_MOVE) return "mov";
		case (UNIT_TYPE_KW_ADD) return "add";
		case (UNIT_TYPE_KW_SUB) return "sub";
		case (UNIT_TYPE_KW_MUL) return "imul";
		case (UNIT_TYPE_KW_DIV) return "div";
		default: return "UNKNOWN";
	}
}

void instruction_push(asm* pAsm, char* msg, ...) {
	
	// String to be appended to
	char str[256];
	
	// Appending additional arguments
	va_list args;
	va_start(args, msg);
	size_t len = vsnprintf(str, sizeof(str), msg, args);
	va_end(args);
	
	// Resize if needed
	asm_resize(pAsm, len + 1);
	
	// Copy the string into the Assembly
	memcpy(&pAsm->buffer[pAsm->size], str, len);
	
	// Add len to the size
	pAsm->size += len;
	
}

void instruction_parse(asm* pAsm, ir* pIR) {
	
	static struct {
		
		bool funcVisible;
		bool inFunc;
		
	} percall;
	
	// Get the current unit
	unit* currentUnit = &pIR->buffer[pIR->index];
	
	if (pAsm->mode == ASM_MODE_SCAN) {
		
		// For all functions and variables, move them into the appropriate code sections
		switch (currentUnit->type) {
			
			case (UNIT_TYPE_KW_FUNC) {
				
				// Check if this is being imported or exported
				if ((pIR->index) > 0) {
					
					if (peek(-1).type == UNIT_TYPE_KW_EXPORT) {
						instruction_push(pAsm, ".global ");
						percall.funcVisible = true;
					} else if (peek(-1).type == UNIT_TYPE_KW_IMPORT) {
						instruction_push(pAsm, "extern ");
						percall.funcVisible = true;
					}
					
					// Advance past this keyword
					advance(1);
					
				} else {
					
					// Advance regardless to not hang the compiler
					advance(1);
					
				}
				
			} break;
			
			case (UNIT_TYPE_PT_DOLLAR) {
				
				if (percall.funcVisible) {
					
					// Emit the function name only if it is being imported or exported
					instruction_push(pAsm, "%s\n", peek(1).value);
					
				}
				
				percall.funcVisible = false;
				percall.inFunc = true;
				
				if (strcmp(peek(1).value, "main") == 0) pAsm->foundMain = true;
				
				// Advance past this and the function name
				advance(2);
				
			} break;
			
			case (UNIT_TYPE_KW_RETURN) {
				
				percall.inFunc = false;
				
			}
			
			default: {
				
				advance(1);
				
			} break;
			
		}
		
	}
	
	if (pAsm->mode == ASM_MODE_PARSE) {
		
		// Check the current unit and emit the appropriate instruction
		switch (currentUnit->type) {
			
			case (UNIT_TYPE_KW_RETURN) {
				
				// Advance past the return keyword
				advance(1);
				
				// Return
				instruction_push(pAsm, "ret\n");
				
			} break;
			
			// Functions are handled in bulk
			case (UNIT_TYPE_KW_FUNC) {
				
				// Advance past function keyword and dollar
				advance(3);
				
				// Push the identifier
				instruction_push(pAsm, "%s:\n", peek(0).value);
				
				// Advance past the identifier
				advance(1);
				
				// For the time being, skip all function parameters
				while (peek(0).type != UNIT_TYPE_PT_SEMICOLON) advance(1);
				advance(1);
				
			} break;
			
			case (UNIT_TYPE_PT_COLON) {
				
				// We can skip this; it's just syntactic sugar
				advance(1);
				
			} break;
			
			// Moving into and out of registers
			case (UNIT_TYPE_KW_ADD)
			case (UNIT_TYPE_KW_SUB)
			case (UNIT_TYPE_KW_MUL)
			case (UNIT_TYPE_KW_MOVE) {
				
				// Emit this operation
				instruction_push(pAsm, "%s ", to_arith(ppeek(0)));
				
				// Advance past this keyword and the type of the register
				advance(2);
				
				// Push the first register
				instruction_push(pAsm, "%s, ", to_reg(ppeek(0)));
				
				// Advance past the register, comma, and the type
				advance(3);
				
				// Push the second register or literal
				instruction_push(pAsm, "%s\n", to_reg(ppeek(0)));
				
				// Advance past this second register or literal and the semicolon
				advance(2);
				
			} break;
			
			default: {
				
				print_utf8("Unknown unit\n");
				
				// Advance past this unit
				advance(1);
				
			}
			
		}
		
	}
	
}

/*////////*/

void asm_print(asm* pAsm) {
	
	print_utf8(pAsm->buffer);
	print_utf8("\n");
	
}

bool asm_generate(asm* pAsm, asm_info* pInfo) {
	
	// Allocate a buffer for the Assembly
	if (asm_resize(pAsm, 0) == false) return false;
	
	// Add new instructions until we reach the end of the unit stream
	(pAsm->mode) = ASM_MODE_SCAN;
	(pInfo->pIR->index) = 0;
	while (1) {
		
		if ((pAsm->size) >= pAsm->memSize)  if (!asm_resize(pAsm, 0)) return false;
		instruction_parse(pAsm, pInfo->pIR);
		if (pInfo->pIR->buffer[pInfo->pIR->index].type == UNIT_TYPE_KW_END) break;
		if (pInfo->pIR->index > pInfo->pIR->size) break;
	}
	
	// Push ExitProcess
	if (pAsm->foundMain) {
		instruction_push(pAsm, "extern ExitProcess\n");
	}
	
	// Add new instructions until we reach the end of the unit stream
	(pAsm->mode) = ASM_MODE_PARSE;
	(pInfo->pIR->index) = 0;
	while (1) {
		if ((pAsm->size) >= pAsm->memSize)  if (!asm_resize(pAsm, 0)) return false;
		instruction_parse(pAsm, pInfo->pIR);
		if (pInfo->pIR->buffer[pInfo->pIR->index].type == UNIT_TYPE_KW_END) break;
		if (pInfo->pIR->index > pInfo->pIR->size) break;
	}
	
	// Generate the _start entry point and have it call main
	if (pAsm->foundMain) {
		instruction_push(pAsm, "_start:\ncall main\n");
		instruction_push(pAsm, "mov ecx, eax\n");
		instruction_push(pAsm, "call ExitProcess\n");
	}
	
	// Null terminate the string
	pAsm->buffer[pAsm->size] = '\0';
	
	// Return success
	return true;
	
}