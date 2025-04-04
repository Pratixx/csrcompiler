// [ INCLUDING ] //

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// [ FUNCTIONS ] //

void code_skipWhitespace(code* pCode) {
	while ((pCode->index < pCode->size) && char_isWhitespace(pCode->buffer[pCode->index])) (pCode->index)++;
}

void code_skipComments(code* pCode) {
	if (((pCode->index + 1) < pCode->size) && (pCode->buffer[pCode->index] == '/')) {
		
		// Handle single and multi-line comments appropriately
		if (pCode->buffer[pCode->index + 1] == '/') {
			
			// Skip to the end of the comment
			while ((pCode->index < pCode->size) && (pCode->buffer[pCode->index] != '\n')) (pCode->index)++;
			
			// Skip to the character after the newline character
			if (pCode->index < pCode->size) (pCode->index)++;
			
			// Return
			return;
			
		} else if (pCode->buffer[pCode->index + 1] == '*') {
			
			// Skip to the end of the comment
			while (1) {
				
				// Obviously return if we've hit the end of file
				if ((pCode->index + 1) >= pCode->size) return;
				
				// If we find the end of the comment, break the loop
				if ((pCode->buffer[pCode->index] == '*') && (pCode->buffer[pCode->index + 1] == '/')) {
					(pCode->index) += 2;
					break;
				}
				
				// Otherwise, advance the code index
				(pCode->index)++;
				
			}
			
			// Return
			return;
			
		}
	}
}

void code_print(code* pCode) {
	
	print_utf8("%s\n", pCode->buffer);
	
}

/*////////*/

bool code_create(code* pCode, code_info* pInfo) {
	
	// Check if valid pointers were passed
	if (pInfo == NULL) {
		return false;
	}
	
	// Create a new code structure
	code newCode;
	
	// Open the file
	FILE* file = fopen(pInfo->fileName, "rb");
	if (file == NULL) {
		return false;
	}
	
	// Get the file size
	if (fseek(file, 0, SEEK_END) != 0) {
		fclose(file);
		return false;
	}
	long fileSize = ftell(file);
	if (fileSize <= 0) {
		fclose(file);
		return false;
	}
	rewind(file);
	
	// Allocate a buffer for the file contents
	newCode.buffer = malloc(fileSize * sizeof(char));
	if (newCode.buffer == NULL) {
		fclose(file);
		return false;
	}
	
	// Copy the file contents to the buffer
	size_t bytesRead = fread(newCode.buffer, 1, fileSize, file);
	fclose(file);
	if (bytesRead != (size_t)fileSize) {
		free(newCode.buffer);
		return false;
	}
	
	newCode.size = bytesRead;
	newCode.index = 0;
	
	// Set the code to the new code
	*pCode = newCode;
	
	// Return success
	return true;
	
}

void code_destroy(code* pCode) {
	
	// Free memory
	free(pCode->buffer);
	pCode->index = 0;
	pCode->size = 0;
	
}
