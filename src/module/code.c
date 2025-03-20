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
		if (pCode->buffer[pCode->index + 1] == '/') {
			while ((pCode->index < pCode->size) && (pCode->buffer[pCode->index] != '\n')) (pCode->index)++;
			if (pCode->index >= pCode->size) return;
			code_skipWhitespace(pCode);
			return;
		} else if (pCode->buffer[pCode->index + 1] == '*') {
			while (1) {
				if (pCode->index >= pCode->size) return;
				if ((pCode->buffer[pCode->index] == '*') && (pCode->buffer[pCode->index + 1] == '/')) {
					(pCode->index) += 2;
					break;
				}
				(pCode->index)++;
			}
			if (pCode->index >= pCode->size) return;
			code_skipWhitespace(pCode);
			return;
		}
	}
}

void code_print(code* pCode) {
	
	print_utf8("%s\n", pCode->buffer);
	
}

code_error code_create(code* pCode, code_info* pInfo) {
	
	// Check if valid pointers were passed
	if (pInfo == NULL) {
		return CODE_ERROR_NULL_PTR;
	}
	
	// Create a new code structure
	code newCode;
	
	// Open the file
	FILE* file = fopen(pInfo->fileName, "rb");
	if (file == NULL) {
		return CODE_ERROR_FILE_LOAD;
	}
	
	// Get the file size
	if (fseek(file, 0, SEEK_END) != 0) {
		fclose(file);
		return CODE_ERROR_FILE_READ;
	}
	long fileSize = ftell(file);
	if (fileSize <= 0) {
		fclose(file);
		return CODE_ERROR_FILE_READ;
	}
	rewind(file);
	
	// Allocate a buffer for the file contents
	newCode.buffer = malloc((fileSize + 1) * sizeof(char));
	if (newCode.buffer == NULL) {
		fclose(file);
		return CODE_ERROR_MEM_ALLOC;
	}
	newCode.buffer[fileSize] = '\0';
	
	// Copy the file contents to the buffer
	size_t bytesRead = fread(newCode.buffer, 1, fileSize, file);
	fclose(file);
	if (bytesRead != (size_t)fileSize) {
		free(newCode.buffer);
		return CODE_ERROR_FILE_READ;
	}
	
	newCode.size = bytesRead;
	newCode.index = 0;
	
	// Set the code to the new code
	*pCode = newCode;
	
	// Return success
	return CODE_SUCCESS;
	
}

void code_destroy(code* pCode) {
	
	// Free memory
	free(pCode->buffer);
	
}
