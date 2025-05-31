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

static bool assm_resize(assm* pAssm, size_t needed) { 
	
	// If the buffer can hold more elements, just return
	if ((pAssm->size + needed) < pAssm->memSize) return true;
	
	// If there is no buffer, allocate a new buffer
	if (pAssm->memSize == 0) {
		
		// Set default buffer size
		pAssm->memSize = 8;
		pAssm->size = 0;
		
		// Allocate a new buffer
		char* newBuffer = calloc(pAssm->memSize, 1);
		if (!newBuffer) return false;
		
		// Assign the new buffer to the old one
		pAssm->buffer = newBuffer;
		
		// Return success
		return true;
		
	}
	
	// Allocate a new buffer
	char* newBuffer = calloc(((pAssm->memSize * 2) + needed), 1);
	if (!newBuffer) return false;
	
	// Copy the new memory in and free the old buffer
	memcpy(newBuffer, pAssm->buffer, pAssm->memSize);
	free(pAssm->buffer);
	
	// Assign the new buffer to the old one
	pAssm->memSize *= 2;
	pAssm->memSize += needed;
	pAssm->buffer = newBuffer;
	
	// Return success
	return true;
	
}

/*////////*/

static bool unit_isUnsigned(unit_type type) {
	switch (type) {
		case (UNIT_TYPE_TP_U8)
		case (UNIT_TYPE_TP_U16)
		case (UNIT_TYPE_TP_U32)
		case (UNIT_TYPE_TP_U64)
			return true;
		default:
			return false;
	}
}

static char* to_word(size_t size) {
	switch (size) {
		case (1) return "byte";
		case (2) return "word";
		case (4) return "dword";
		case (8) return "qword";
		default: return "dword";
	}
}

static size_t to_size(unit* pUnit) {
	
	switch (pUnit->type) {
		case (UNIT_TYPE_TP_U8)
		case (UNIT_TYPE_TP_S8) return 1;
		case (UNIT_TYPE_TP_U16)
		case (UNIT_TYPE_TP_S16) return 2;
		case (UNIT_TYPE_TP_U32)
		case (UNIT_TYPE_TP_F32)
		case (UNIT_TYPE_TP_S32) return 4;
		case (UNIT_TYPE_TP_U64)
		case (UNIT_TYPE_TP_F64)
		case (UNIT_TYPE_TP_S64) return 8;
		case (UNIT_TYPE_TP_F128) return 16;
	}
	
}

static char* to_reg(assm* pAssm, unit* pUnit) {
	switch (pUnit[0].type) {
		
		// General purpose registers
		case (UNIT_TYPE_RG_RG1) {
			switch (pUnit[-1].type) {
				case (UNIT_TYPE_TP_S32) return "r10d";
				case (UNIT_TYPE_TP_S64) return "r10";
				default: return "r10";
			}
		}
		case (UNIT_TYPE_RG_RG2) {
			switch (pUnit[-1].type) {
				case (UNIT_TYPE_TP_S32) return "r11d";
				case (UNIT_TYPE_TP_S64) return "r11";
				default: return "r10";
			}
		}
		
		// Arithmetic registers
		case (UNIT_TYPE_RG_AR1) {
			switch (pUnit[-1].type) {
				case (UNIT_TYPE_TP_S32) return "eax";
				case (UNIT_TYPE_TP_S64) return "rax";
				default: return "rax";
			}
		}
		case (UNIT_TYPE_RG_AR2) {
			switch (pUnit[-1].type) {
				case (UNIT_TYPE_TP_S32) return "ecx";
				case (UNIT_TYPE_TP_S64) return "rcx";
				default: return "rcx";
			}
		}
		
		// Argument registers
		case (UNIT_TYPE_RG_ARG1) {
			switch (pUnit[-1].type) {
				case (UNIT_TYPE_TP_S32) return "ecx";
				case (UNIT_TYPE_TP_S64) return "rcx";
				default: return "rcx";
			}
		}
		case (UNIT_TYPE_RG_ARG2) {
			switch (pUnit[-1].type) {
				case (UNIT_TYPE_TP_S32) return "edx";
				case (UNIT_TYPE_TP_S64) return "rdx";
				default: return "rdx";
			}
		}
		case (UNIT_TYPE_RG_ARG3) {
			switch (pUnit[-1].type) {
				case (UNIT_TYPE_TP_S32) return "r8d";
				case (UNIT_TYPE_TP_S64) return "r8";
				default: return "r8";
			}
		}
		case (UNIT_TYPE_RG_ARG4) {
			switch (pUnit[-1].type) {
				case (UNIT_TYPE_TP_S32) return "r9d";
				case (UNIT_TYPE_TP_S64) return "r9";
				default: return "r9";
			}
		}
		
		// Return value register
		case (UNIT_TYPE_RG_RETVAL) {
			switch (pUnit[-1].type) {
				case (UNIT_TYPE_TP_S32) return "eax";
				case (UNIT_TYPE_TP_S64) return "rax";
				default: return "rax";
			}
		}
		
		// For literals, just return its value
		case (UNIT_TYPE_LITERAL) return pUnit[0].value;	
		
		// For variables, convert it to a base pointer offset
		case (UNIT_TYPE_IDENTIFIER) {
			
			symbol* pSym = symbol_find(&pAssm->offsetTable, pUnit->value, SYMBOL_CLASS_ALL);
			
			if (pSym) {
				
				static char buf[256] = {};
				
				// Scope index is reused as a variable offset here
				sprintf(buf, "%s [rbp - %u]", to_word(pSym->size), pSym->scopeIndex);
				
				return buf;
				
			}
			
			return "";
			
		}
		
		default: "";
		
	}
}

static char* to_arith(unit* pUnit) {
	switch (pUnit[0].type) {
		case (UNIT_TYPE_KW_MOVE) return "mov";
		case (UNIT_TYPE_KW_ADD)  return "add";
		case (UNIT_TYPE_KW_SUB)  return "sub";
		case (UNIT_TYPE_KW_MUL)  return "imul";
		case (UNIT_TYPE_KW_DIV)  return "div";
		default: return "UNKNOWN";
	}
}

static void instruction_push(assm* pAssm, char* msg, ...) {
	
	// String to be appended to
	char str[256];
	
	// Appending additional arguments
	va_list args;
	va_start(args, msg);
	size_t len = vsnprintf(str, sizeof(str), msg, args);
	va_end(args);
	
	// Resize if needed
	assm_resize(pAssm, len + 1);
	
	// Copy the string into the Assembly
	memcpy(&pAssm->buffer[pAssm->size], str, len);
	
	// Add len to the size
	pAssm->size += len;
	
}

static void instruction_parse(assm* pAssm, ir* pIR) {
	
	static struct {
		
		bool funcVisible;
		bool inFunc;
		
	} percall;
	
	// Get the current unit
	unit* currentUnit = &pIR->buffer[pIR->index];
	
	if (pAssm->mode == ASM_MODE_VISIBILITY) {
		
		// For all functions and variables, move them into the appropriate code sections
		switch (currentUnit->type) {
			
			case (UNIT_TYPE_KW_FUNC) {
				
				// Check if this is being imported or exported
				if ((pIR->index) > 0) {
					
					unit_type linkage = peek(-1).type;
					
					// Advance past the type and onto the name
					while (peek(0).type != UNIT_TYPE_IDENTIFIER) advance(1);
					
					if (strcmp(peek(0).value, "main") == 0) {
						
						pAssm->foundMain = true;
						
					} else {
						
						if (linkage == UNIT_TYPE_KW_EXPORT) {
							instruction_push(pAssm, "global ");
							percall.funcVisible = true;
						} else if (linkage == UNIT_TYPE_KW_IMPORT) {
							instruction_push(pAssm, "extern ");
							percall.funcVisible = true;
						}
						
						// Emit the identifier
						instruction_push(pAssm, "%s\n", peek(0).value);
						
					}
					
				} else {
					
					// Advance regardless to not hang the compiler
					advance(1);
					
				}
				
			} break;
			
			case (UNIT_TYPE_KW_RETURN) {
				
				percall.inFunc = false;
				
				advance(1);
				
			} break;
			
			default: {
				
				advance(1);
				
			} break;
			
		}
		
	}
	
	if (pAssm->mode == ASM_MODE_LITERALS) {
		
		// For all literals in the program, assign them to the data section
		switch (currentUnit->type) {
			
			case (UNIT_TYPE_KW_STATIC) {
				
				// Advance to the identifier
				while (peek(0).type != UNIT_TYPE_IDENTIFIER) advance(1);
				
				// Emit the variable name
				instruction_push(pAssm, "%s ", peek(0).value);
				
				// Advance to the colon
				advance(1);
				
				// Emit the word size
				instruction_push(pAssm, "db ");
				
				// Advance past this keyword
				advance(1);
				
				// Emit the string
				instruction_push(pAssm, "%s, 0\n", peek(0).value);
				
				// Advance past the string
				advance(1);
				
			} break;
			
			default: {
				
				// Advance past this unit
				advance(1);
				
			}
			
		}
		
	}
	
	if (pAssm->mode == ASM_MODE_PARSE) {
		
		// Check the current unit and emit the appropriate instruction
		switch (currentUnit->type) {
			
			case (UNIT_TYPE_KW_RETURN) {
				
				// Destroy the stack frame
				if (strcmp(pAssm->currentFunc, "main") != 0) {
					
					instruction_push(pAssm, "mov rsp, rbp\n");
					instruction_push(pAssm, "pop rbp\n");
					
				}
				
				// Advance past the return keyword
				advance(1);
				
				// Return
				if (strcmp(pAssm->currentFunc, "main") == 0) {
					
					instruction_push(pAssm, "mov ecx, r10d\n");
					instruction_push(pAssm, "call ExitProcess\n");
					
				} else {
					
					instruction_push(pAssm, "mov eax, r10d\n");
					instruction_push(pAssm, "ret\n");
					
				}
				
			} break;
			
			case (UNIT_TYPE_KW_ARG_PUSH) {
				
				// Save our registers
				instruction_push(pAssm, "mov [rsp+0], rcx\n");
				instruction_push(pAssm, "mov [rsp+8], rdx\n");
				instruction_push(pAssm, "mov [rsp+16], r8\n");
				instruction_push(pAssm, "mov [rsp+24], r9\n");
				
				// Advance
				advance(1);
				
			} break;
			
			case (UNIT_TYPE_KW_ARG_POP) {
				
				// Return the state of our registers
				instruction_push(pAssm, "mov rcx, [rsp+0]\n");
				instruction_push(pAssm, "mov rdx, [rsp+8]\n");
				instruction_push(pAssm, "mov r8, [rsp+16]\n");
				instruction_push(pAssm, "mov r9, [rsp+24]\n");
				
				// Advance
				advance(1);
				
			} break;
			
			case (UNIT_TYPE_KW_CALL) {
				
				// Call the function
				instruction_push(pAssm, "call %s\n", peek(1).value);
				
				// Advance
				advance(1);
				
			} break;
			
			// Variables!
			case (UNIT_TYPE_KW_LOCAL) {
				
				// Add this variable to the offset table
				symbol* pSym = symbol_add(&pAssm->offsetTable, peek(2).value, SYMBOL_TYPE_VARIABLE, to_size(ppeek(1)), pAssm->offset, SYMBOL_CLASS_VARIABLE);
				
				// Add to the offset
				pAssm->offset += to_size(ppeek(1));
				
				// Advance past this, the size, and the semicolon
				advance(3);
				
			} break;
			
			// Handle stack frame setup and destruction
			case (UNIT_TYPE_KW_ALLOC) {
				
				// Create a new offset table for storing variable offsets
				symbol_table_create(&pAssm->offsetTable);
				pAssm->offset = 0;
				
				// Create the stack frame
				instruction_push(pAssm, "push rbp\n");
				instruction_push(pAssm, "mov rbp, rsp\n");
				
				// Advance to the literal holding the stack allocation size
				advance(1);
				
				// Subtract the stack pointer by X bytes, including 32 bytes of shadow space
				instruction_push(pAssm, "sub rsp, 32 ; This should be optimized later to merge with below\n");
				instruction_push(pAssm, "sub rsp, %s\n", peek(0).value);
				
				// Set the stack space size
				pAssm->stackSize = peek(0).value;
				
				// Advance past this and the semicolon
				advance(2);
				
			} break;
			
			case (UNIT_TYPE_KW_FREE) {
				
				// Destroy the offset table, as we're done with our variables
				symbol_table_destroy(&pAssm->offsetTable);
				
				// Add back X bytes to the stack pointer, including the 32 bytes of shadow space
				instruction_push(pAssm, "add rsp, 32 ; Same thing\n");
				instruction_push(pAssm, "add rsp, %s\n", pAssm->stackSize);
				
				// Advance past this and the semicolon
				advance(2);
				
			} break;
			
			// Functions are handled in bulk
			case (UNIT_TYPE_KW_FUNC) {
				
				// If this is an import, it doesn't have a body
				if (peek(-1).type == UNIT_TYPE_KW_IMPORT) {
					
					advance(1);
					
					break;
					
				}
				
				// Advance past function keyword and the type
				advance(2);
				
				// Windows expects 16-byte alignment, so emit that to comply
				instruction_push(pAssm, "align 16\n");
				
				// Push the identifier
				instruction_push(pAssm, "%s:\n", peek(0).value);
				
				// Set the current function
				pAssm->currentFunc = peek(0).value;
				
				// Advance past the identifier
				advance(1);
				
				// For the time being, skip all function parameters
				while (peek(0).type != UNIT_TYPE_PT_SEMICOLON) advance(1);
				advance(1);
				
			} break;
			
			case (UNIT_TYPE_KW_MOVE) {
				
				// Check if we're moving in a literal
				instruction_push(pAssm, "mov ");
				
				// Advance past this keyword
				if (peek(1).type == UNIT_TYPE_IDENTIFIER)
					advance(1);
				else
					advance(2);
				
				// Push the first register
				instruction_push(pAssm, "%s, ", to_reg(pAssm, ppeek(0)));
				
				// Advance past the register and the comma
				advance(2);
				
				// Push the second register or literal
				instruction_push(pAssm, "%s\n", to_reg(pAssm, ppeek(0)));
				
				// Advance past this second register or literal and the semicolon
				advance(2);
				
			} break;
			
			// Arithmetic
			case (UNIT_TYPE_KW_ADD)
			case (UNIT_TYPE_KW_SUB) {
				
				// Emit this operation
				instruction_push(pAssm, "%s ", to_arith(ppeek(0)));
				
				// Advance past this keyword and the type of the register
				if (peek(1).type == UNIT_TYPE_IDENTIFIER)
					advance(1);
				else
					advance(2);
				
				// Push the first register
				instruction_push(pAssm, "%s, ", to_reg(pAssm, ppeek(0)));
				
				// Advance past the register and the comma
				advance(2);
				
				// Push the second register or literal
				instruction_push(pAssm, "%s\n", to_reg(pAssm, ppeek(0)));
				
				// Advance past this second register or literal and the semicolon
				advance(2);
				
			} break;
			
			case (UNIT_TYPE_KW_MUL) {
				
				// Advance past this keyword
				advance(1);
				
				// Depending on the type, we need to handle some special x86 quirks
				if (unit_isUnsigned(peek(0).type)) {
					
					// Hold the register we're pulling from to move back into later
					char* reg = to_reg(pAssm, ppeek(0));
					
					// We need to load the multiplicand into the accumulator register
					instruction_push(pAssm, "mov eax, %s\n", to_reg(pAssm, ppeek(0)));
					
					// Advance past the register, comma, and the type
					advance(3);
					
					// Multiply the multiplier on the accumulator register
					instruction_push(pAssm, "mul %s\n", to_reg(pAssm, ppeek(0)));
					
					// Move this back into the register we pulled from
					instruction_push(pAssm, "mov %s, eax\n", reg);
					
					// Advance past this second register or literal and the semicolon
					advance(2);
					
				} else {
					
					// There are no quirks for signed division
					
					// Advance past the type
					advance(1);

					// Emit this operation
					instruction_push(pAssm, "imul ");
					
					// Push the first register
					instruction_push(pAssm, "%s, ", to_reg(pAssm, ppeek(0)));
					
					// Advance past the register, comma, and the type
					advance(3);
					
					// Push the second register or literal
					instruction_push(pAssm, "%s\n", to_reg(pAssm, ppeek(0)));
					
					// Advance past this second register or literal and the semicolon
					advance(2);
					
				}
				
			} break;
			
			case (UNIT_TYPE_KW_DIV) {
				
				// Division is extremely weird; the dividend is in the accumulator, and the quotient ends up in the accumulator...
				// Yeah, strange. Oh, and you have to sign extend the register with cdq or zero it out depending if it's signed or unsigned.
				
				// Advance past this keyword
				advance(1);
				
				// Depending on the type, we need to handle some special x86 quirks
				if (unit_isUnsigned(peek(0).type)) {
					
					// Advance past the type
					advance(1);
					
					// Hold the register we're pulling from to move back into later
					char* reg = to_reg(pAssm, ppeek(0));
					
					// We need to load the dividend into the accumulator register
					instruction_push(pAssm, "mov eax, %s\n", to_reg(pAssm, ppeek(0)));
					
					// Advance past the register, comma, and the type
					advance(3);
					
					// Move the divisor into the counter, since you can't divide by literals for some reason...
					instruction_push(pAssm, "mov ecx, %s\n", to_reg(pAssm, ppeek(0)));
					
					// Zero out the upper bits of edx
					instruction_push(pAssm, "xor edx, edx\n");
					
					// Divide the accumulator by the divisor
					instruction_push(pAssm, "div ecx\n");
					
					// Move this back into the register we pulled from
					instruction_push(pAssm, "mov %s, eax\n", reg);
					
					// Advance past this second register or literal and the semicolon
					advance(2);
					
				} else {
					
					// Advance past the type
					advance(1);
					
					// Hold the register we're pulling from to move back into later
					char* reg = to_reg(pAssm, ppeek(0));
					
					// We need to load the dividend into the accumulator register
					instruction_push(pAssm, "mov eax, %s\n", reg);
					
					// Advance past the register, comma, and the type
					advance(3);
					
					// Sign extend into edx
					instruction_push(pAssm, "cdq\n");
					
					// Move the divisor into the counter, since you can't divide by literals for some reason...
					instruction_push(pAssm, "mov ecx, %s\n", to_reg(pAssm, ppeek(0)));
					
					// Divide the accumulator by the register or literal
					instruction_push(pAssm, "idiv ecx\n");
					
					// Move this back into the register we pulled from
					instruction_push(pAssm, "mov %s, eax\n", reg);
					
					// Advance past this second register or literal and the semicolon
					advance(2);
					
				}
				
			}
			
			default: {
				
				advance(1);
				
			} break;
			
		}
		
	}
	
}

/*////////*/

void assm_print(assm* pAssm) {
	
	print_utf8(pAssm->buffer);
	print_utf8("\n");
	
}

bool assm_generate(assm* pAssm, assm_info* pInfo) {
	
	// Allocate a buffer for the Assembly
	if (assm_resize(pAssm, 0) == false) return false;
	
	// Add new instructions until we reach the end of the unit stream
	(pAssm->mode) = ASM_MODE_VISIBILITY;
	(pInfo->pIR->index) = 0;
	while (1) {
		if ((pAssm->size) >= pAssm->memSize)  if (!assm_resize(pAssm, 0)) return false;
		instruction_parse(pAssm, pInfo->pIR);
		if (pInfo->pIR->buffer[pInfo->pIR->index].type == UNIT_TYPE_KW_END) break;
		if (pInfo->pIR->index > pInfo->pIR->size) break;
	}
	
	// Push ExitProcess
	if (pAssm->foundMain) {
		instruction_push(pAssm, "extern ExitProcess\n");
		instruction_push(pAssm, "global _main\n");
	}
	
	// Emit the text section
	instruction_push(pAssm, "section .data\n");
	
	// Add new instructions until we reach the end of the unit stream
	(pAssm->mode) = ASM_MODE_LITERALS;
	(pInfo->pIR->index) = 0;
	while (1) {
		if ((pAssm->size) >= pAssm->memSize)  if (!assm_resize(pAssm, 0)) return false;
		instruction_parse(pAssm, pInfo->pIR);
		if (pInfo->pIR->buffer[pInfo->pIR->index].type == UNIT_TYPE_KW_END) break;
		if (pInfo->pIR->index > pInfo->pIR->size) break;
	}
	
	// Emit the text section
	instruction_push(pAssm, "section .text\n");
	
	// Add new instructions until we reach the end of the unit stream
	(pAssm->mode) = ASM_MODE_PARSE;
	(pInfo->pIR->index) = 0;
	while (1) {
		if ((pAssm->size) >= pAssm->memSize)  if (!assm_resize(pAssm, 0)) return false;
		instruction_parse(pAssm, pInfo->pIR);
		if (pInfo->pIR->buffer[pInfo->pIR->index].type == UNIT_TYPE_KW_END) break;
		if (pInfo->pIR->index > pInfo->pIR->size) break;
	}
	
	// Push ExitProcess
	if (pAssm->foundMain) {
		instruction_push(pAssm, "_main:\n");
		instruction_push(pAssm, "jmp main\n");
	}
	
	// Null terminate the string
	pAssm->buffer[pAssm->size] = '\0';
	
	// Return success
	return true;
	
}