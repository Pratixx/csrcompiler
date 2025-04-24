// [ INCLUDING ] //

#include <windows.h>

#include "module/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

// [ DEFINING ] //

struct {
	code code;
	stream stream;
	symbol_table symbolTable;
	error_table errorTable;
	ast ast;
	ir ir;
	asm asm;
} currentFile;

// [ FUNCTIONS ] //

__attribute__((constructor)) void init() {
	
	// Set console output mode
	SetConsoleOutputCP(CP_UTF8);
	
}

/*////////*/

void print_utf16(const unsigned short* msg, ...) {
	
	// Platform dependent crap
	HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD written;
	
	// String to be appended to
	unsigned short str[256];
	
	// Appending additional arguments
	va_list args;
	va_start(args, msg);
	vsnwprintf(str, sizeof(str), msg, args);
	va_end(args);
	
	// Getting string length
	size_t len = 0;
	while (str[len] != '\0') {
		len++;
	}
	
	// Printing the string
	WriteConsoleW(stdHandle, str, (DWORD)len, &written, NULL);
	
}

void print_utf8(const char* msg, ...) {
	
	// Platform dependent crap
	HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD written;
	
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
	
	// Printing the string
	WriteConsoleA(stdHandle, str, (DWORD)len, &written, NULL);
	
}

// [ MAIN ] //

int main(int argCount, char* argList[]) {
	
	// Define file code info
	code_info currentFileCodeInfo = {};
	currentFileCodeInfo.fileName = "test.csr";
	
	// Create the code
	if (!code_create(&currentFile.code, &currentFileCodeInfo)) {
		
		// Return error
		return EXIT_FAILURE;
		
	}
	
	print_utf8("Creation of file code succeeded.\n");
	
	// Define file stream info
	stream_info currentFileStreamInfo = {};
	currentFileStreamInfo.pCode = &currentFile.code;
	currentFileStreamInfo.pSymbolTable = &currentFile.symbolTable;
	
	// Create the file stream
	if (!stream_create(&currentFile.stream, &currentFileStreamInfo)) {
		
		// Return error
		return EXIT_FAILURE;
		
	}
	
	stream_print(&currentFile.stream);
	
	print_utf8("Creation of file stream succeeded.\n");
	
	// Create the symbol table
	if (!symbol_table_create(&currentFile.symbolTable)) {
		
		// Return error
		return EXIT_FAILURE;
		
	}
	
	// Add recognized identifiers to the symbol table
	symbol_add(&currentFile.symbolTable, "byte",    SYMBOL_TYPE_TYPE, SYMBOL_SIZE_BITS_8,  SYMBOL_CLASS_TYPE);
	symbol_add(&currentFile.symbolTable, "int",     SYMBOL_TYPE_TYPE, SYMBOL_SIZE_BITS_32, SYMBOL_CLASS_TYPE);
	symbol_add(&currentFile.symbolTable, "float",   SYMBOL_TYPE_TYPE, SYMBOL_SIZE_BITS_64, SYMBOL_CLASS_TYPE);
	symbol_add(&currentFile.symbolTable, "decimal", SYMBOL_TYPE_TYPE, SYMBOL_SIZE_BITS_64, SYMBOL_CLASS_TYPE);
	symbol_add(&currentFile.symbolTable, "bool",    SYMBOL_TYPE_TYPE, SYMBOL_SIZE_BITS_8,  SYMBOL_CLASS_TYPE);
	
	symbol_add(&currentFile.symbolTable, "true",  SYMBOL_TYPE_LITERAL, SYMBOL_SIZE_BITS_0, SYMBOL_CLASS_LITERAL);
	symbol_add(&currentFile.symbolTable, "false", SYMBOL_TYPE_LITERAL, SYMBOL_SIZE_BITS_0, SYMBOL_CLASS_LITERAL);
	
	symbol_add(&currentFile.symbolTable, "null", SYMBOL_TYPE_LITERAL, SYMBOL_SIZE_BITS_0, SYMBOL_CLASS_LITERAL);
	
	print_utf8("Creation of symbol table succeeded.\n");
	
	// Create the error table
	if (!error_table_create(&currentFile.errorTable)) {
		
		// Return error
		return EXIT_FAILURE;
		
	}
	
	print_utf8("Creation of error table succeeded.\n");
	
	// Define file AST info
	ast_info currentFileASTInfo = {};
	currentFileASTInfo.pStream = &currentFile.stream;
	currentFileASTInfo.pSymbolTable = &currentFile.symbolTable;
	currentFileASTInfo.pErrorTable = &currentFile.errorTable;
	
	// Create the AST
	if (!ast_create(&currentFile.ast, &currentFileASTInfo)) {
		
		// Return error
		return EXIT_FAILURE;
		
	}
	
	print_utf8("Creation of file AST succeeded.\n");
	
	ast_print(&currentFile.ast);
	
	error_table_print(&currentFile.errorTable);
	
	symbol_table_print(&currentFile.symbolTable);
	
	// Define file IR info
	ir_info currentFileIRInfo = {};
	currentFileIRInfo.pAST = &currentFile.ast;
	currentFileIRInfo.pSymbolTable = &currentFile.symbolTable;
	
	// Generate the IR
	if (!ir_generate(&currentFile.ir, &currentFileIRInfo)) {
		
		// Return error
		return EXIT_FAILURE;
		
	}
	
	ir_print(&currentFile.ir);
	
	print_utf8("Generation of file IR succeeded.\n");
	
	// Define file IR info
	asm_info currentFileAsmInfo = {};
	currentFileAsmInfo.pIR = &currentFile.ir;
	
	// Generate the Assembly
	if (!asm_generate(&currentFile.asm, &currentFileAsmInfo)) {
		
		// Return error
		return EXIT_FAILURE;
		
	}
	
	asm_print(&currentFile.asm);
	
	print_utf8("Generation of file Assembly succeeded.\n");
	
	// Print success
	print_utf8("Creation of file compilation objects succeeded.\n");
	
	sleep(2);
	
	// Destroy everything
	stream_destroy(&currentFile.stream);
	code_destroy(&currentFile.code);
	
	// Print success
	print_utf8("Destruction of file compilation objects succeeded.\n");
	sleep(2);
	
	// Return success
	return EXIT_SUCCESS;
	
}
