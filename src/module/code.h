#pragma once

// [ DEFINING ] //

typedef enum {
	
	CODE_SUCCESS,
	
	CODE_ERROR_NULL_PTR,
	
	CODE_ERROR_MEM_ALLOC,
	
	CODE_ERROR_FILE_LOAD,
	CODE_ERROR_FILE_READ,
	
} code_error;

typedef struct {
	char* fileName;
} code_info;

typedef struct {
	size_t size;
	size_t index;
	char* buffer;
} code;

// [ FUNCTIONS ] //

void code_skipWhitespace(code* pCode);

void code_skipComments(code* pCode);

code_error code_create(code* pCode, code_info* pInfo);
void code_destroy(code* pCode);