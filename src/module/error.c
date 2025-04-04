// [ INCLUDING ] //

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// [ FUNCTIONS ] //

bool error_table_resize(error_table* pErrorTable) {
	
	// If the buffer can hold more elements, just return
	if (pErrorTable->size < pErrorTable->memSize) return true;
	
	// If there is no buffer, allocate a new buffer
	if (pErrorTable->memSize == 0) {
		
		// Set default buffer size
		pErrorTable->memSize = 8;
		pErrorTable->size = 0;
		
		// Allocate a new buffer
		error* newBuffer1 = calloc(pErrorTable->memSize, sizeof(error));
		node** newBuffer2 = calloc(pErrorTable->memSize, sizeof(node*));
		if ((!newBuffer1) || (!newBuffer2)) return false;
		
		// Assign the new buffer to the old one
		pErrorTable->errorBuffer = newBuffer1;
		pErrorTable->nodeBuffer = newBuffer2;
		
		// Return success
		return true;
		
	}
	
	// Allocate a new buffer
	error* newBuffer1 = calloc(pErrorTable->memSize, sizeof(error));
	node** newBuffer2 = calloc(pErrorTable->memSize, sizeof(node*));
	if ((!newBuffer1) || (!newBuffer2)) return false;
	
	// Copy the new memory in and free the old buffer
	memcpy(newBuffer1, pErrorTable->errorBuffer, (pErrorTable->memSize * sizeof(error)));
	memcpy(newBuffer2, pErrorTable->nodeBuffer, (pErrorTable->memSize * sizeof(node*)));
	free(pErrorTable->errorBuffer);
	free(pErrorTable->nodeBuffer);
	
	// Assign the new buffer to the old one
	pErrorTable->memSize *= 2;
	pErrorTable->errorBuffer = newBuffer1;
	pErrorTable->nodeBuffer = newBuffer2;
	
	// Return success
	return true;
	
}

/*////////*/

void error_table_push(error_table* pErrorTable, error thisError, node* pNode) {
	
	// Resize if needed
	error_table_resize(pErrorTable);
	
	// Add a new error and node to the buffers
	pErrorTable->errorBuffer[pErrorTable->size] = thisError;
	pErrorTable->nodeBuffer[pErrorTable->size] = pNode;
	(pErrorTable->size)++;
	
	// Return a pointer to the new symbol
	return;
	
}

/*////////*/

void error_table_print(error_table* pErrorTable) {
	
	for (size_t i = 0; i < pErrorTable->size; i++) {
		
		print_utf8("error: %s\n",
			(pErrorTable->errorBuffer[i] == ERROR_SYNTACTIC_MISSING_SEMICOLON) ? "Missing semicolon" :
			(pErrorTable->errorBuffer[i] == ERROR_SYNTACTIC_MISSING_COLON)    ? "Missing colon" :
			(pErrorTable->errorBuffer[i] == ERROR_SYNTACTIC_MISSING_COMMA)   ? "Missing comma" :
			(pErrorTable->errorBuffer[i] == ERROR_SYNTACTIC_MISSING_BRACE)   ? "Missing brace" :
			(pErrorTable->errorBuffer[i] == ERROR_SYNTACTIC_MISSING_PAREN)   ? "Missing parenthesis" :
			(pErrorTable->errorBuffer[i] == ERROR_SYNTACTIC_MISSING_BRACKET) ? "Missing bracket" :
			(pErrorTable->errorBuffer[i] == ERROR_SYNTACTIC_MALFORMED_LITERAL) ? "Malformed literal" :
			(pErrorTable->errorBuffer[i] == ERROR_SYNTACTIC_MALFORMED_COMMENT) ? "Malformed comment" :
			(pErrorTable->errorBuffer[i] == ERROR_SEMANTIC_TYPE_MISMATCH)   ? "Type mismatch" :
			(pErrorTable->errorBuffer[i] == ERROR_SEMANTIC_REDECLARATION)   ? "Redeclaration" :
			(pErrorTable->errorBuffer[i] == ERROR_SEMANTIC_ARG_MISMATCH)    ? "Argument mismatch" :
			(pErrorTable->errorBuffer[i] == ERROR_SEMANTIC_CONST_MODIFY)    ? "Const modify" : 
			"Unknown error"
		);
		
	}
	
}

bool error_table_create(error_table* pErrorTable) {
	
	// Allocate a buffer for the stream
	if (error_table_resize(pErrorTable) == false) return false;
	
	// Return success
	return true;
	
}

void error_table_destroy(error_table* pErrorTable) {
	
	// Free memory
	free(pErrorTable->errorBuffer);
	free(pErrorTable->nodeBuffer);
	pErrorTable->memSize = 0;
	pErrorTable->size = 0;
	
}