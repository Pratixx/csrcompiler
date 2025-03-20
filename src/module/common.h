#pragma once

// [ MACROS ] //

#define MAX_VALUE_LEN 32
#define MAX_OPERATOR_LEN 3
#define MAX_PUNCTUATOR_LEN 2

#define case(arg) case arg:

// [ INCLUDING ] //

#include "../icl/cstdef.h"
#include "../icl/cstint.h"

#include "code.h"
#include "symbol.h"
#include "stream.h"
#include "ast.h"

// [ DEFINING ] //

typedef enum {
	
	// Syntactic errors
	ERROR_SYNTACTIC_MISSING_OPERATOR,
	ERROR_SYNTACTIC_HEADER_EXPORT,
	
	// Semantic errors
	ERROR_SEMANTIC_TYPE_MISMATCH,
	
} error;

/*////////*/

struct token_table {
	char* name;
	token_type type;
};

// Token keyword table
static struct token_table token_kw_table[] = {
	
	// ADD: typeof, alignas, alignof, sizeof
	
	{"return", TOKEN_TYPE_KW_RETURN},
	
	{"struct", TOKEN_TYPE_KW_STRUCT},
	{"enum", TOKEN_TYPE_KW_ENUM},
	{"union", TOKEN_TYPE_KW_UNION},
	
	{"namespace", TOKEN_TYPE_KW_NAMESPACE},
	
	{"auto", TOKEN_TYPE_KW_AUTO},
	
	{"static", TOKEN_TYPE_KW_STATIC},
	
	{"inline", TOKEN_TYPE_KW_INLINE},
	
	{"using", TOKEN_TYPE_KW_USING},
	
	{"if", TOKEN_TYPE_KW_IF},
	{"else", TOKEN_TYPE_KW_ELSE},
	
	{"while", TOKEN_TYPE_KW_WHILE},
	{"for", TOKEN_TYPE_KW_FOR},
	{"continue", TOKEN_TYPE_KW_CONTINUE},
	
	{"switch", TOKEN_TYPE_KW_SWITCH},
	{"case", TOKEN_TYPE_KW_CASE},
	{"default", TOKEN_TYPE_KW_DEFAULT},
	
	{"break", TOKEN_TYPE_KW_BREAK},
	{"jump", TOKEN_TYPE_KW_JUMP},
	
	{"import", TOKEN_TYPE_KW_IMPORT},
	{"export", TOKEN_TYPE_KW_EXPORT},
	
	{"module", TOKEN_TYPE_KW_MODULE},
	{"header", TOKEN_TYPE_KW_HEADER},
	
	{"", TOKEN_TYPE_UNDEFINED}
	
};

// Token punctuator table
static struct token_table token_pt_table[] = {
	
	{";", TOKEN_TYPE_PT_SEMICOLON},
	{":", TOKEN_TYPE_PT_COLON},
	
	{"{", TOKEN_TYPE_PT_OPEN_BRACE},
	{"}", TOKEN_TYPE_PT_CLOSE_BRACE},
	
	{"[", TOKEN_TYPE_PT_OPEN_BRACKET},
	{"]", TOKEN_TYPE_PT_CLOSE_BRACKET},
	
	{"(", TOKEN_TYPE_PT_OPEN_PAREN},
	{")", TOKEN_TYPE_PT_CLOSE_PAREN},
	
	{".",  TOKEN_TYPE_PT_PERIOD},
	{",",  TOKEN_TYPE_PT_COMMA},
	{"->", TOKEN_TYPE_PT_ARROW},
	
	{"", TOKEN_TYPE_UNDEFINED}
	
};

// Token operator table
static struct token_table token_op_table[] = {
	
	{"+", TOKEN_TYPE_OP_ADD},
	{"-", TOKEN_TYPE_OP_SUB},
	{"*", TOKEN_TYPE_OP_MUL},
	{"/", TOKEN_TYPE_OP_DIV},
	{"%", TOKEN_TYPE_OP_MOD},
	
	{"&",  TOKEN_TYPE_OP_BIT_AND},
	{"|",  TOKEN_TYPE_OP_BIT_OR},
	{"^",  TOKEN_TYPE_OP_BIT_XOR},
	{"~",  TOKEN_TYPE_OP_BIT_NOT},
	{"<<", TOKEN_TYPE_OP_BIT_SHIFT_LEFT},
	{">>", TOKEN_TYPE_OP_BIT_SHIFT_RIGHT},
	
	{"=",  TOKEN_TYPE_OP_ASSIGN},
	{"+=", TOKEN_TYPE_OP_ASSIGN_ADD},
	{"-=", TOKEN_TYPE_OP_ASSIGN_SUB},
	{"*=", TOKEN_TYPE_OP_ASSIGN_MUL},
	{"/=", TOKEN_TYPE_OP_ASSIGN_DIV},
	{"%=", TOKEN_TYPE_OP_ASSIGN_MOD},
	
	{"&",  TOKEN_TYPE_OP_ASSIGN_BIT_AND},
	{"|",  TOKEN_TYPE_OP_ASSIGN_BIT_OR},
	{"^",  TOKEN_TYPE_OP_ASSIGN_BIT_XOR},
	{"~",  TOKEN_TYPE_OP_ASSIGN_BIT_NOT},
	{"<<", TOKEN_TYPE_OP_ASSIGN_BIT_SHIFT_LEFT},
	{">>", TOKEN_TYPE_OP_ASSIGN_BIT_SHIFT_RIGHT},
	
	{"++", TOKEN_TYPE_OP_INC},
	{"--", TOKEN_TYPE_OP_DEC},
	
	{"?", TOKEN_TYPE_OP_TERNARY_IF},
	{":", TOKEN_TYPE_OP_TERNARY_ELSE},
	
	{"*", TOKEN_TYPE_OP_DEREF},
	
	{"&&", TOKEN_TYPE_OP_CMP_AND},
	{"||", TOKEN_TYPE_OP_CMP_OR},
	{"==", TOKEN_TYPE_OP_CMP_EQUAL},
	{"<",  TOKEN_TYPE_OP_CMP_LESS},
	{"<=", TOKEN_TYPE_OP_CMP_LESS_EQUAL},
	{">",  TOKEN_TYPE_OP_CMP_GREATER},
	{">=", TOKEN_TYPE_OP_CMP_GREATER_EQUAL},
	{"!",  TOKEN_TYPE_OP_CMP_NOT},
	
	{"", TOKEN_TYPE_UNDEFINED}
	
};

// [ FUNCTIONS ] //

static inline bool char_isWhitespace(char character) {
	return ((character == ' ') || (character == '\n') || (character == '\t') || (character == '\r'));
}

void print_utf8(const char* msg, ...);
void print_utf16(const unsigned short* msg, ...);