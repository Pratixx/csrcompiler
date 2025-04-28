// [ INCLUDING ] //

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// [ FUNCTIONS ] //

int cststrncmp(const char *str1, const char *str2, size_t n) {
    // Compare up to n characters
    for (size_t i = 0; i < n; ++i) {
        // If both strings are null-terminated, return 0
        if (str1[i] == '\0' && str2[i] == '\0') {
            return 0;
        }
        // If one string ends prematurely, return difference
        if (str1[i] == '\0' || str2[i] == '\0') {
            return (unsigned char)str1[i] - (unsigned char)str2[i];
        }
        // Compare characters at current index
        if (str1[i] != str2[i]) {
            return (unsigned char)str1[i] - (unsigned char)str2[i];
        }
    }
    
    // After n characters, check that both strings are null-terminated
    if (str1[n] != '\0' || str2[n] != '\0') {
        return (unsigned char)str1[n] - (unsigned char)str2[n];
    }
	
    return 0;  // Strings are equal up to n characters and both end with null terminator
}

/*////////*/

bool stream_resize(stream* pStream) { 
	
	// If the buffer can hold more elements, just return
	if (pStream->size < pStream->memSize) return true;
	
	// If there is no buffer, allocate a new buffer
	if (pStream->memSize == 0) {
		
		// Set default buffer size
		pStream->memSize = 8;
		pStream->size = 0;
		
		// Allocate a new buffer
		token* newBuffer = calloc(pStream->memSize, sizeof(token));
		if (!newBuffer) return false;
		
		// Assign the new buffer to the old one
		pStream->buffer = newBuffer;
		
		// Return success
		return true;
		
	}
	
	// Allocate a new buffer
	token* newBuffer = calloc((pStream->memSize * 2), sizeof(token));
	if (!newBuffer) return false;
	
	// Copy the new memory in and free the old buffer
	memcpy(newBuffer, pStream->buffer, (pStream->memSize * sizeof(token)));
	free(pStream->buffer);
	
	// Assign the new buffer to the old one
	pStream->memSize *= 2;
	pStream->buffer = newBuffer;
	
	// Return success
	return true;
	
}

/*////////*/

token_type str_isKeyword(const char* str) {
	size_t index = 0;
    while (token_kw_table[index].type != TOKEN_TYPE_UNDEFINED) {  
		if (cststrncmp(str, token_kw_table[index].name, strlen(token_kw_table[index].name)) == 0) return token_kw_table[index].type;
		index++;
    }  
	return TOKEN_TYPE_UNDEFINED;
}

token_type str_isLiteral(const char* str) {
	
	if (str[0] == '\0') return TOKEN_TYPE_EOF;
	
	// Number literals
	if (isdigit(str[0]) || ((str[0] == '.') && isdigit(str[1]))) {
		bool dot_seen = false;
		while (*str != '\0') {
			if (isdigit(*str)) {
				str++;
			} else if ((*str == '.') && (!dot_seen)) {
				dot_seen = true;
				str++;
			} else {
				return TOKEN_TYPE_INVALID;
			}
		}
		
		return dot_seen ? TOKEN_TYPE_LITERAL_FLOAT : TOKEN_TYPE_LITERAL_INT;
		
	}
	
	// String literal
	if (str[0] == '"') {
		str++;
		while (*str != '\0' && *str != '"') {
			str++;
		}
		if (*str == '"') {
			return TOKEN_TYPE_LITERAL_STR;
		}
		return TOKEN_TYPE_INVALID;
	}
	
    // Character literal
    if (str[0] == '\'' && str[1] != '\0') {
        if (str[2] == '\'') {
            return TOKEN_TYPE_LITERAL_CHAR;
        } else if (str[1] == '\\' && str[2] != '\0' && str[3] == '\'') {
            return TOKEN_TYPE_LITERAL_CHAR;
        }
    }
	
	return TOKEN_TYPE_UNDEFINED;
	
}

token_type str_isOperator(const char* str, size_t* out) {
	size_t index = 0;
	while (token_op_table[index].type != TOKEN_TYPE_UNDEFINED) {
		*out = strlen(token_op_table[index].name);
		if (strncmp(str, token_op_table[index].name, *out) == 0) return token_op_table[index].type;
		index++;
	}
	return TOKEN_TYPE_UNDEFINED;
}

token_type str_isPunctuator(const char* str, size_t* out) {
	size_t index = 0;
	while (token_pt_table[index].type != TOKEN_TYPE_UNDEFINED) {
		*out = strlen(token_pt_table[index].name);
		if (strncmp(str, token_pt_table[index].name, *out) == 0) return token_pt_table[index].type;
		index++;
		
	}
	return TOKEN_TYPE_UNDEFINED;
}

token_type str_isTypeSpecifier(const char* str) {
	size_t index = 0;
	while (token_sp_table[index].type != TOKEN_TYPE_UNDEFINED) {
		
		if (cststrncmp(str, token_sp_table[index].name, strlen(token_sp_table[index].name)) == 0) return token_sp_table[index].type;
		index++;
		
	}
	return TOKEN_TYPE_UNDEFINED;
}

token_type str_isTypeQualifier(const char* str) {
	size_t index = 0;
	while (token_qu_table[index].type != TOKEN_TYPE_UNDEFINED) {
		
		if (cststrncmp(str, token_qu_table[index].name, strlen(token_qu_table[index].name)) == 0) return token_qu_table[index].type;
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
	
	// The total length of the chosen token's string equivalent
	size_t out;
	
	// Append characters and check if they are a valid keyword
	while (1) {
		
		// Skip whitespace and comments before appending
		code_skipWhitespace(pCode);
		code_skipComments(pCode);
		code_skipWhitespace(pCode);
		
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
		
		// Only check for an punctuator if the value is less than the maximum punctuator length
		if (tokenValueIndex < MAX_PUNCTUATOR_LEN) {
			thisToken.type = str_isPunctuator(thisStr, &out);
			if (thisToken.type != TOKEN_TYPE_UNDEFINED) {
				
				// Append the remaining characters to the token value
				for (size_t i = tokenValueIndex; i < (out + tokenValueIndex); i++) {
					thisToken.value[i] = thisStr[i];
				}
				thisToken.value[out + tokenValueIndex] = '\0';
				
				// Increment the code index by the length of the string, minus one to account for the fact we're already on the first character
				(pCode->index) += (out - 1);
				
				break;
				
			}
		}
		
		// Only check for an operator if the value is less than the maximum operator length
		if (tokenValueIndex < MAX_OPERATOR_LEN) {
			thisToken.type = str_isOperator(thisStr, &out);
			if (thisToken.type != TOKEN_TYPE_UNDEFINED) {
				
				// Append the remaining characters to the token value
				for (size_t i = tokenValueIndex; i < (out + tokenValueIndex); i++) {
					thisToken.value[i] = thisStr[i];
				}
				thisToken.value[out + tokenValueIndex] = '\0';
				
				// Increment the code index by the length of the string, minus one to account for the fact we're already on the first character
				(pCode->index) += (out - 1);
				
				break;
				
			}
		}
		
		// If this character is whitespace, an operator, or a punctuator, we should check if this is a literal or keyword
		if (char_isWhitespace(thisStr[1]) || str_isPunctuator(&thisStr[1], &out) || str_isOperator(&thisStr[1], &out)) {
			
			// Check if this token is a literal
			thisToken.type = str_isLiteral(thisToken.value);
			if (thisToken.type != TOKEN_TYPE_UNDEFINED) break;
			
			// Check if this token is a keyword
			thisToken.type = str_isKeyword(thisToken.value);
			if (thisToken.type != TOKEN_TYPE_UNDEFINED) break;
			
			// Check if this token is a type specifier
			thisToken.type = str_isTypeSpecifier(thisToken.value);
			if (thisToken.type != TOKEN_TYPE_UNDEFINED) break;
			
			// Check if this token is a type qualifier
			thisToken.type = str_isTypeQualifier(thisToken.value);
			if (thisToken.type != TOKEN_TYPE_UNDEFINED) break;
			
			// If it's none of them, it's probably invalid
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
		char value[MAX_VALUE_LEN * 2] = {};
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
			
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_LITERAL_CHAR) ? strcat(strcat(strcpy(value, "LITERAL_CHAR ["), pStream->buffer[tokenSetIndex].value), "]") :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_LITERAL_STR) ? strcat(strcat(strcpy(value, "LITERAL_STR ["), pStream->buffer[tokenSetIndex].value), "]") :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_LITERAL_INT) ? strcat(strcat(strcpy(value, "LITERAL_INT ["), pStream->buffer[tokenSetIndex].value), "]") :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_LITERAL_INT_HEX) ? strcat(strcat(strcpy(value, "LITERAL_INT_HEX ["), pStream->buffer[tokenSetIndex].value), "]") :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_LITERAL_FLOAT) ? strcat(strcat(strcpy(value, "LITERAL_FLOAT ["), pStream->buffer[tokenSetIndex].value), "]") :
			
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_IDENTIFIER) ? strcat(strcat(strcpy(value, "IDENTIFIER ["), pStream->buffer[tokenSetIndex].value), "]") :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_INVALID)   ? strcat(strcat(strcpy(value, "INVALID ["), pStream->buffer[tokenSetIndex].value), "]")   :
			
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_UNDEFINED) ? "UNDEFINED" :
			
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_ASSIGN) ? "OP_ASSIGN" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_ADD) ? "OP_ADD" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_SUB) ? "OP_SUB" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_MUL) ? "OP_MUL" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_DIV) ? "OP_DIV" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_MOD) ? "OP_MOD" :
			
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_CMP_NOT) ? "OP_CMP_NOT" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_CMP_OR) ? "OP_CMP_OR" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_CMP_AND) ? "OP_CMP_AND" :
			
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_INC) ? "OP_INC" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_OP_DEC) ? "OP_DEC" :
			
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_PT_SEMICOLON) ? "PN_SEMICOLON" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_PT_COMMA) ? "PN_COMMA" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_PT_AMPERSAND) ? "PT_AMPERSAND" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_PT_OPEN_PAREN)  ? "PT_OPEN_PAREN"  :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_PT_CLOSE_PAREN) ? "PT_CLOSE_PAREN" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_PT_OPEN_BRACE)  ? "PT_OPEN_BRACE"  :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_PT_CLOSE_BRACE) ? "PT_CLOSE_BRACE" :
			
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_SP_PTR) ? "SP_PTR" :
			
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_QU_CONST) ? "QU_CONST" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_QU_RESTRICT) ? "QU_RESTRICT" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_QU_VOLATILE) ? "QU_VOLATILE" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_QU_ATOMIC) ? "QU_ATOMIC" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_QU_STATIC) ? "QU_STATIC" :
			(pStream->buffer[tokenSetIndex].type == TOKEN_TYPE_QU_THREAD_LOCAL) ? "QU_THREAD_LOCAL" :
			
			"EOF"
			
		);
		tokenSetIndex++;
	}
	
	print_utf8("\n");
	
}

bool stream_create(stream* pStream, stream_info* pInfo) {
	
	// Allocate a buffer for the stream
	if (stream_resize(pStream) == false) return false;
	
	// Add new tokens until we reach EOF
	while (1) {
		if ((pStream->size) == pStream->memSize)  if (!stream_resize(pStream)) return false;
		pStream->buffer[pStream->size] = token_parse(pInfo->pCode);
		if (pStream->buffer[pStream->size].type == TOKEN_TYPE_EOF) break;
		(pStream->size)++;
	}
	
	// Return success
	return true;
	
}

void stream_destroy(stream* pStream) {
	
	// Free memory
	free(pStream->buffer);
	pStream->memSize = 0;
	pStream->size = 0;
	pStream->index = 0;
	
}
