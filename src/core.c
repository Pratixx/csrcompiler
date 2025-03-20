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
	symbol_table symbolTable;
	stream stream;
	ast ast;
	// ir ir;
	// obj obj;
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

/*////////*/

// void node_print(node* pNode, unsigned int depth) {
	
// 	if (depth > 0) {
// 		node* currentNode = pNode;
// 		unsigned int currentDepth = (depth - 1);
// 		unsigned short treeGraph[depth + 1];
// 		treeGraph[depth] = '\0';
// 		while (currentNode->parent) {
// 			if (currentDepth == (depth - 1)) {
// 				// if (currentNode->parent->firstChild == currentNode) {}
// 				if (currentNode->next) {
// 					treeGraph[currentDepth] = L'├';
// 				} else {
// 					treeGraph[currentDepth] = L'└';
// 				}
// 			} else {
// 				if (currentNode->parent->firstChild) {
// 					treeGraph[currentDepth] = L'│';
// 				} else {
// 					treeGraph[currentDepth] = L' ';
// 				}
// 			}
// 			currentNode = currentNode->parent;
// 			currentDepth--;
// 		}
// 		print_utf16(treeGraph);
// 	}
	
// 	print_utf8("%s ",
// 		(pNode->type == NODE_TYPE_RETURN) ? "RETURN" :
// 		(pNode->type == NODE_TYPE_LITERAL) ? "LITERAL" :
// 		(pNode->type == NODE_TYPE_FILE) ? "FILE" :
// 		(pNode->type == NODE_TYPE_INVALID) ? "INVALID" :
// 		(pNode->type == NODE_TYPE_UNDEFINED) ? "UNDEFINED" :
// 		"EOF"
// 	);
	
// 	if (pNode->tokenCount > 0) {
// 		print_utf8("[");
// 		for (size_t i = 0; i < pNode->tokenCount; i++) {
// 			print_utf8("%s", pNode->tokenList[i].value);
// 			if (i < pNode->tokenCount - 1) print_utf8(", ");
// 		}
// 		print_utf8("]");
// 	}
	
// 	print_utf8("\n");
	
// 	if (pNode->firstChild) {
// 		node_print(pNode->firstChild, depth + 1);
// 	}
	
// 	if (pNode->next) {
// 		node_print(pNode->next, depth);
// 	}
	
// }

// [ MAIN ] //

int main(int argCount, char* argList[]) {
	
	// Define file code info
	code_info currentFileCodeInfo = {};
	currentFileCodeInfo.fileName = "test.l";
	
	// Create the code
	if (code_create(&currentFile.code, &currentFileCodeInfo) != CODE_SUCCESS) {
		
		// Return error
		return EXIT_FAILURE;
		
	}
	
	print_utf8("Creation of file code succeeded.\n");
	
	// Create the symbol table
	if (symbol_table_create(&currentFile.symbolTable) != SYMBOL_TABLE_SUCCESS) {
		
		// Return error
		return EXIT_FAILURE;
		
	}
	
	// Add recognized identifiers to the symbol table
	symbol* thisSymbol = NULL;
	
	thisSymbol = symbol_add(&currentFile.symbolTable, "void", SYMBOL_TYPE_TYPE, 0);
	
	thisSymbol = symbol_add(&currentFile.symbolTable, "byte", SYMBOL_TYPE_TYPE, 0); thisSymbol->size = SYMBOL_SIZE_BITS_8;
	thisSymbol = symbol_add(&currentFile.symbolTable, "int", SYMBOL_TYPE_TYPE, 0); thisSymbol->size = SYMBOL_SIZE_BITS_32;
	thisSymbol = symbol_add(&currentFile.symbolTable, "float", SYMBOL_TYPE_TYPE, 0); thisSymbol->size = SYMBOL_SIZE_BITS_64;
	thisSymbol = symbol_add(&currentFile.symbolTable, "decimal", SYMBOL_TYPE_TYPE, 0); thisSymbol->size = SYMBOL_SIZE_BITS_64;
	thisSymbol = symbol_add(&currentFile.symbolTable, "bool", SYMBOL_TYPE_TYPE, 0); thisSymbol->size = SYMBOL_SIZE_BITS_8;
	
	symbol_add(&currentFile.symbolTable, "true", SYMBOL_TYPE_TYPE, 0);
	symbol_add(&currentFile.symbolTable, "false", SYMBOL_TYPE_TYPE, 0);
	
	symbol_add(&currentFile.symbolTable, "null", SYMBOL_TYPE_TYPE, 0);
	
	symbol_add(&currentFile.symbolTable, "signed", SYMBOL_TYPE_TYPE_QUALIFIER, 0);
	
	symbol_add(&currentFile.symbolTable, "long", SYMBOL_TYPE_TYPE_QUALIFIER, 0);
	symbol_add(&currentFile.symbolTable, "short", SYMBOL_TYPE_TYPE_QUALIFIER, 0);
	
	symbol_add(&currentFile.symbolTable, "restrict", SYMBOL_TYPE_TYPE_QUALIFIER, 0);
	symbol_add(&currentFile.symbolTable, "volatile", SYMBOL_TYPE_TYPE_QUALIFIER, 0);
	symbol_add(&currentFile.symbolTable, "atomic", SYMBOL_TYPE_TYPE_QUALIFIER, 0);
	symbol_add(&currentFile.symbolTable, "inline", SYMBOL_TYPE_TYPE_QUALIFIER, 0);
	
	symbol_add(&currentFile.symbolTable, "static", SYMBOL_TYPE_STORAGE_SPECIFIER, 0);
	symbol_add(&currentFile.symbolTable, "const", SYMBOL_TYPE_STORAGE_SPECIFIER, 0);
	symbol_add(&currentFile.symbolTable, "thread_local", SYMBOL_TYPE_STORAGE_SPECIFIER, 0);
	
	print_utf8("Creation of symbol table succeeded.\n");
	
	// Define file stream info
	stream_info currentFileStreamInfo = {};
	currentFileStreamInfo.pCode = &currentFile.code;
	
	// Create the file stream
	if (stream_create(&currentFile.stream, &currentFileStreamInfo) != STREAM_SUCCESS) {
		
		// Return error
		return EXIT_FAILURE;
		
	}
	
	stream_print(&currentFile.stream);
	
	print_utf8("Creation of file stream succeeded.\n");
	
	symbol_table_print(&currentFile.symbolTable);
	
	// Define file AST info
	ast_info currentFileASTInfo = {};
	currentFileASTInfo.pStream = &currentFile.stream;
	
	// Create the AST
	if (ast_create(&currentFile.ast, &currentFileASTInfo) != AST_SUCCESS) {
		
		// Return error
		return EXIT_FAILURE;
		
	}
	
	ast_print(&currentFile.ast);
	
	print_utf8("Creation of file AST succeeded.\n");
	
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
