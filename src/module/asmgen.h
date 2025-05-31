#pragma once

// [ DEFINING ] //

typedef struct {
	ir* pIR;
} assm_info;

typedef enum {
	
	ASM_MODE_VISIBILITY,
	ASM_MODE_LITERALS,
	ASM_MODE_PARSE,
	
} assm_mode;

typedef struct {
	size_t memSize;
	size_t size;
	size_t index;
	bool foundMain;
	char* currentFunc;
	char* stackSize;
	assm_mode mode;
	char* buffer;
	symbol_table offsetTable;
	size_t offset;
} assm;

// [ FUNCTIONS ] //

bool assm_generate(assm* pAsm, assm_info* pInfo);
void assm_print(assm* pAsm);