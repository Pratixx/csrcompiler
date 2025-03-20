// [ INCLUDING ] //

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// [ FUNCTIONS ] //

stream_error stream_resize(stream* pStream) { 
	
	// If the buffer can hold more elements, just return
	if (pStream->size < pStream->memSize) return SYMBOL_TABLE_SUCCESS;
	
	// If there is no buffer, allocate a new buffer
	if (pStream->memSize == 0) {
		
		// Set default buffer size
		pStream->memSize = 8;
		pStream->size = 0;
		
		// Allocate a new buffer
		token* newBuffer = calloc(pStream->memSize, sizeof(token));
		if (!newBuffer) return STREAM_ERROR_MEM_ALLOC;
		
		// Assign the new buffer to the old one
		pStream->buffer = newBuffer;
		
		// Return success
		return STREAM_SUCCESS;
		
	}
	
	// Allocate a new buffer
	token* newBuffer = calloc((pStream->memSize * 2), sizeof(token));
	if (!newBuffer) return STREAM_ERROR_MEM_ALLOC;
	
	// Copy the new memory in and free the old buffer
	memcpy(newBuffer, pStream->buffer, (pStream->memSize * sizeof(token)));
	free(pStream->buffer);
	
	// Assign the new buffer to the old one
	pStream->memSize *= 2;
	pStream->buffer = newBuffer;
	
	// Return success
	return STREAM_SUCCESS;
	
}

/*////////*/

token_type str_isKeyword(const char* str) {
	
	size_t index = 0;
	
	while (token_kw_table[index].type != TOKEN_TYPE_UNDEFINED) {
		if (strncmp(str, token_kw_table[index].name, strlen(token_kw_table[index].name)) == 0) return token_kw_table[index].type;
		index++;
	}
	
	return TOKEN_TYPE_UNDEFINED;
	
}

token_type str_isLiteral(const char* str) {
	
	if (str[0] == '\0') return TOKEN_TYPE_EOF;
	
	// Number literals
	if (isdigit(str[0])) {
		while (*str != '\0') {
			if (isdigit(*str)) str++;
			else return TOKEN_TYPE_INVALID;
		}
		return TOKEN_TYPE_LITERAL;
	}
	
	return TOKEN_TYPE_UNDEFINED;
	
}

token_type str_isOperator(const char* str) {
	
	size_t index = 0;
	
	while (token_op_table[index].type != TOKEN_TYPE_UNDEFINED) {
		if (strncmp(str, token_op_table[index].name, strlen(token_op_table[index].name)) == 0) return token_op_table[index].type;
		index++;
	}
	
	return TOKEN_TYPE_UNDEFINED;
	
}

token_type str_isPunctuator(const char* str) {
	
	size_t index = 0;
	
	while (token_pt_table[index].type != TOKEN_TYPE_UNDEFINED) {
		
		if (strncmp(str, token_pt_table[index].name, strlen(token_pt_table[index].name)) == 0) return token_pt_table[index].type;
		index++;
		
	}
	
	return TOKEN_TYPE_UNDEFINED;
	
}

/*////////*/

token token_parse(code* pCode) {
	
	// This current token
	token thisToken = {};
	size_t tokenValueIndex = 0;
	
	thisToken.type = TOKEN_TYPE_UNDEFINED;
	
	// Skip whitespace and comments before appending
	code_skipWhitespace(pCode);
	
	// Append characters and check if they are a valid keyword
	while (1) {
		
		// Skip any comments
		code_skipComments(pCode);
		
		// If our code index is at the end of the file, return EOF immediately
		if (pCode->index >= pCode->size) {
			thisToken.type = TOKEN_TYPE_EOF;
			return thisToken;
		}
		
		// If we've hit the character limit for this token, invalidate it because it cannot be parsed further
		if (tokenValueIndex == MAX_VALUE_LEN) {
			thisToken.type = TOKEN_TYPE_UNDEFINED;
			break;
		}
		
		// Get the current string
		char* thisStr = &pCode->buffer[pCode->index];
		
		// If this isn't whitespace, we may append the character
		if (!char_isWhitespace(thisStr[0])) {
			thisToken.value[tokenValueIndex] = thisStr[0];
			thisToken.value[tokenValueIndex + 1] = '\0';
		}
		
		// Only check for an operator if the value is less than the maximum operator length
		if (tokenValueIndex < MAX_OPERATOR_LEN) {
			thisToken.type = str_isOperator(thisToken.value);
			if (thisToken.type != TOKEN_TYPE_UNDEFINED) break;
		}
		
		// Only check for an punctuator if the value is less than the maximum punctuator length
		if (tokenValueIndex < MAX_PUNCTUATOR_LEN) {
			thisToken.type = str_isPunctuator(thisToken.value);
			if (thisToken.type != TOKEN_TYPE_UNDEFINED) break;
		}
		
		// If this character is whitespace, an operator, or a punctuator, we should check if this is a literal or keyword
		if (char_isWhitespace(thisStr[1]) || str_isOperator(&thisStr[1]) || str_isPunctuator(&thisStr[1])) {
			
			// Check if this token is a literal
			thisToken.type = str_isLiteral(thisToken.value);
			if (thisToken.type != TOKEN_TYPE_UNDEFINED) break;
			
			// Check if this token is a keyword
			thisToken.type = str_isKeyword(thisToken.value);
			if (thisToken.type != TOKEN_TYPE_UNDEFINED) break;
			
			// If it's neither, it's probably invalid
			thisToken.type = TOKEN_TYPE_INVALID;
			break;
				
		}
		
		// If the current character is whitespace, then this token is invalid
		if (char_isWhitespace(thisStr[0])) {
			thisToken.type = TOKEN_TYPE_INVALID;
			break;
		}
		
		// Advance the code and token value index
		tokenValueIndex++;
		(pCode->index)++;
		
	}
	
	// Move the code index to the next character
	(pCode->index)++;
	
	// Return the token
	return thisToken;
	
}

/*////////*/

void stream_print(stream* pStream) {
	
	size_t tokenSetIndex = 0;
	while (pStream->buffer[tokenSetIndex].type != TOKEN_TYPE_EOF) {
		
		// This is probably the most disgusting thing I've ever written
		char value[(MAX_VALUE_LEN * 2)] = {};
		print_utf8("%s ",
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_KW_RETURN) ? "KW_RETURN" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_KW_IF) ? "KW_IF" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_KW_ELSE) ? "KW_ELSE" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_KW_WHILE) ? "KW_WHILE" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_KW_FOR) ? "KW_FOR" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_KW_BREAK) ? "KW_BREAK" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_KW_CONTINUE) ? "KW_CONTINUE" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_KW_IMPORT) ? "KW_IMPORT" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_KW_EXPORT) ? "KW_EXPORT" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_KW_MODULE) ? "KW_MODULE" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_KW_HEADER) ? "KW_HEADER" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_LITERAL) ? strcat(strcat(strcpy(value, "LITERAL ["), pStream->buffer[tokenSetIndex].value), "]") :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_TYPE) ? strcat(strcat(strcpy(value, "TYPE ["), pStream->buffer[tokenSetIndex].value), "]") :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_IDENTIFIER) ? strcat(strcat(strcpy(value, "IDENTIFIER ["), pStream->buffer[tokenSetIndex].value), "]") :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_PT_SEMICOLON) ? "PN_SEMICOLON" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_INVALID)   ? strcat(strcat(strcpy(value, "INVALID ["), pStream->buffer[tokenSetIndex].value), "]")   :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_UNDEFINED) ? "UNDEFINED" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_ASSIGN) ? "OP_ASSIGN" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_ADD) ? "OP_ADD" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_SUB) ? "OP_SUB" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_MUL) ? "OP_MUL" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_DIV) ? "OP_DIV" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_PT_OPEN_PAREN)  ? "PN_OPEN_PAREN"  :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_PT_CLOSE_PAREN) ? "PN_CLOSE_PAREN" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_PT_OPEN_BRACE)  ? "PN_OPEN_BRACE"  :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_PT_CLOSE_BRACE) ? "PN_CLOSE_BRACE" :
			"EOF"
		);
		tokenSetIndex++;
		
	}
	
	print_utf8("\n");
	
}

stream_error stream_create(stream* pStream, stream_info* pInfo) {
	
	// Allocate a buffer for the stream
	if (stream_resize(pStream) == STREAM_ERROR_MEM_ALLOC) return STREAM_ERROR_MEM_ALLOC;
	
	// Add new tokens until we reach EOF
	while (1) {
		if ((pStream->size) == pStream->memSize)  if (stream_resize(pStream) == STREAM_ERROR_MEM_ALLOC) return STREAM_ERROR_MEM_ALLOC;
		pStream->buffer[pStream->size] = token_parse(pInfo->pCode);
		if (pStream->buffer[pStream->size].type == TOKEN_TYPE_EOF) break;
		(pStream->size)++;
	}
	
	// Perform resolving and syntactic analysis
	for (pStream->index = 0; pStream->index < pStream->size; (pStream->index)++) {
		
		// Get the current stream
		token* thisStream = &pStream->buffer[pStream->index];
		
		// Check if this is a token that needs resolving
		if ((thisStream[0].type == TOKEN_TYPE_INVALID) && (pStream->index >= 1)) {
			
			if (thisStream[-1].type == TOKEN_TYPE_IDENTIFIER) {
				
				
				
				// Continue to the next token
				continue;
				
			}
			
			if (thisStream[-1].type == TOKEN_TYPE_KW_HEADER) {
				
				
				
			}
			
		}
		
	}
	
	// Return success
	return STREAM_SUCCESS;
	
}

void stream_destroy(stream* pStream) {
	
	// Free memory
	free(pStream->buffer);
	pStream->memSize = 0;
	pStream->size = 0;
	pStream->index = 0;
	
}
