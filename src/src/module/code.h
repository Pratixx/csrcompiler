#pragma once

// [ DEFINING ] //

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

bool code_create(code* pCode, code_info* pInfo);
void code_destroy(code* pCode);