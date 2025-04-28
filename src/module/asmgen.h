#pragma once

// [ DEFINING ] //

typedef struct {
	ir* pIR;
} asm_info;

typedef enum {
	
	ASM_MODE_SCAN,
	ASM_MODE_PARSE,
	
} asm_mode;

typedef struct {
	size_t memSize;
	size_t size;
	size_t index;
	bool foundMain;
	asm_mode mode;
	char* buffer;
} asm;

// [ FUNCTIONS ] //

bool asm_generate(asm* pAsm, asm_info* pInfo);
void asm_print(asm* pAsm);