// [ INCLUDING ] //

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

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

void instruction_push(asm* pAsm, char* msg, ...) {
	
	// String to be appended to
	char str[256];
	
	// Appending additional arguments
	va_list args;
	va_start(args, msg);
	vsnprintf(str, sizeof(str), msg, args);
	va_end(args);
	
	// Getting string length
	size_t len = 0;
	while (str[len] != '\0') {
		len++;
	}
	
	// Resize if needed
	asm_resize(pAsm, len);
	
	// Copy the string into the Assembly
	strncpy(&pAsm->buffer[pAsm->size], str, len);

	// Add len to the size
	pAsm->size += len;
	
	// Add a newline
	pAsm->buffer[pAsm->size] = '\n';
	
	// Add the newline
	pAsm->size++;
	
}

void instruction_parse(asm* pAsm, ir* pIR) {
	
	// Check the current unit and emit the appropriate instruction
	unit* currentUnit = &pIR->buffer[pIR->index];
	switch (currentUnit->type) {
		
		// We need to skip FUNC and $ to get the identifier
		case (UNIT_TYPE_KW_EXPORT) {
			instruction_push(pAsm, ".global ", currentUnit[3].value);
		} break;
		
		// Functions are handled in bulk
		case (UNIT_TYPE_KW_FUNC) {
			
			// Skip the function keyword and dollar
			(pIR->index) += 2;
			
			// Push the identifier
			instruction_push(pAsm, currentUnit->value);
			
		} break;
		
	}
	
	// Increment the IR index
	(pIR->index)++;
	
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
	while (1) {
		if ((pAsm->size) == pAsm->memSize)  if (!asm_resize(pAsm, 0)) return false;
		instruction_parse(pAsm, pInfo->pIR);
		if (pInfo->pIR->buffer[pInfo->pIR->index].type == UNIT_TYPE_KW_END) break;
		if (pInfo->pIR->index > pInfo->pIR->size) break;
		(pAsm->size)++;
	}
	
	// Null terminate the string
	pAsm->buffer[pAsm->size] = '\0';
	
	// Return success
	return true;
	
}