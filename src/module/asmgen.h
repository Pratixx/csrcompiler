#pragma once

// [ DEFINING ] //

typedef struct {
	ir* pIR;
} asm_info;

typedef struct {
	size_t memSize;
	size_t size;
	size_t index;
	char* buffer;
} asm;

// [ FUNCTIONS ] //

bool asm_generate(asm* pAsm, asm_info* pInfo);
void asm_print(asm* pAsm);