#pragma once

/* NOTES
// At the moment, compiler uses iterative checking, not hash table. Order keywords, operators, and the such
// from longest to shortest if they share the same first character. For example, put ++ before +, and -- before -.
// 
// Also be sure to check punctuators before operators, as -> is a punctuator and - is an operator.
*/ 

// [ MACROS ] //

#define MAX_VALUE_LEN 32
#define MAX_OPERATOR_LEN 3
#define MAX_PUNCTUATOR_LEN 3

#define case(arg) case arg:

// [ INCLUDING ] //

#include "../icl/cstdef.h"
#include "../icl/cstint.h"

#include "code.h"
#include "symbol.h"
#include "stream.h"
#include "error.h"
#include "ast.h"
#include "irgen.h"
#include "asmgen.h"

// [ DEFINING ] //

struct token_table {
	char* name;
	token_type type;
};

// Token keyword table
static struct token_table token_kw_table[] = {
	
	// ADD: reflect, alignas, alignof
	
	{"return", TOKEN_TYPE_KW_RETURN},
	
	{"struct", TOKEN_TYPE_KW_STRUCT},
	{"enum", TOKEN_TYPE_KW_ENUM},
	{"union", TOKEN_TYPE_KW_UNION},
	
	{"namespace", TOKEN_TYPE_KW_NAMESPACE},
	
	{"using", TOKEN_TYPE_KW_USING},
	
	{"if",   TOKEN_TYPE_KW_IF},
	{"else", TOKEN_TYPE_KW_ELSE},
	{"switch",  TOKEN_TYPE_KW_SWITCH},
	{"case",    TOKEN_TYPE_KW_CASE},
	
	{"while",    TOKEN_TYPE_KW_WHILE},
	{"for",      TOKEN_TYPE_KW_FOR},
	{"continue", TOKEN_TYPE_KW_CONTINUE},
	{"exit",     TOKEN_TYPE_KW_EXIT},
	
	{"break", TOKEN_TYPE_KW_BREAK},
	{"jump",  TOKEN_TYPE_KW_JUMP},
	
	{"import", TOKEN_TYPE_KW_IMPORT},
	{"export", TOKEN_TYPE_KW_EXPORT},
	
	{"module", TOKEN_TYPE_KW_MODULE},
	{"header", TOKEN_TYPE_KW_HEADER},
	
	{"", TOKEN_TYPE_UNDEFINED}
	
};

// Token punctuator table
static struct token_table token_pt_table[] = {
	
	{"...", TOKEN_TYPE_PT_ELLIPSIS},
	
	{";", TOKEN_TYPE_PT_SEMICOLON},
	{":", TOKEN_TYPE_PT_COLON},
	
	{"*", TOKEN_TYPE_PT_AMPERSAND},
	{"?", TOKEN_TYPE_PT_QUESTION},
	
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
	
	{"&=",  TOKEN_TYPE_OP_ASSIGN_BIT_AND},
	{"|=",  TOKEN_TYPE_OP_ASSIGN_BIT_OR},
	{"^=",  TOKEN_TYPE_OP_ASSIGN_BIT_XOR},
	{"~=",  TOKEN_TYPE_OP_ASSIGN_BIT_NOT},
	{"<<=", TOKEN_TYPE_OP_ASSIGN_BIT_SHIFT_LEFT},
	{">>=", TOKEN_TYPE_OP_ASSIGN_BIT_SHIFT_RIGHT},
	
	{"++", TOKEN_TYPE_OP_INC},
	{"--", TOKEN_TYPE_OP_DEC},
	
	{"&&", TOKEN_TYPE_OP_CMP_AND},
	{"||", TOKEN_TYPE_OP_CMP_OR},
	{"==", TOKEN_TYPE_OP_CMP_EQUAL},
	{"<",  TOKEN_TYPE_OP_CMP_LESS},
	{"<=", TOKEN_TYPE_OP_CMP_LESS_EQUAL},
	{">",  TOKEN_TYPE_OP_CMP_GREATER},
	{">=", TOKEN_TYPE_OP_CMP_GREATER_EQUAL},
	{"!",  TOKEN_TYPE_OP_CMP_NOT},
	
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
	
	{"+", TOKEN_TYPE_OP_ADD},
	{"-", TOKEN_TYPE_OP_SUB},
	{"*", TOKEN_TYPE_OP_MUL},
	{"/", TOKEN_TYPE_OP_DIV},
	{"%", TOKEN_TYPE_OP_MOD},
	
	{"", TOKEN_TYPE_UNDEFINED}
	
};

// Token type specifier table
static struct token_table token_sp_table[] = {
	
	{"*", TOKEN_TYPE_SP_PTR},
	
	{"void", TOKEN_TYPE_SP_VOID},
	
	{"long", TOKEN_TYPE_SP_LONG},
	{"short", TOKEN_TYPE_SP_SHORT},
	
	{"signed", TOKEN_TYPE_SP_SIGNED},
	
	{"", TOKEN_TYPE_UNDEFINED}
	
};

// Token type qualifier table
static struct token_table token_qu_table[] = {
	
	{"const", TOKEN_TYPE_QU_CONST},
	{"restrict", TOKEN_TYPE_QU_RESTRICT},
	{"volatile", TOKEN_TYPE_QU_VOLATILE},
	{"atomic", TOKEN_TYPE_QU_ATOMIC},
	{"auto", TOKEN_TYPE_QU_AUTO},
	{"static", TOKEN_TYPE_QU_STATIC},
	{"thread_local", TOKEN_TYPE_QU_THREAD_LOCAL},
	
	{"", TOKEN_TYPE_UNDEFINED}
	
};

// [ FUNCTIONS ] //

bool token_isKeyword(token_type tokenType);
bool token_isIdentifier(token_type tokenType);
bool token_isLiteral(token_type tokenType);
bool token_isOperator(token_type tokenType);
bool token_isPunctuator(token_type tokenType);
bool token_isTypeSpecifier(token_type tokenType);
bool token_isTypeQualifier(token_type tokenType);

static inline bool char_isWhitespace(char character) {
	return ((character == ' ') || (character == '\n') || (character == '\t') || (character == '\r'));
}

void print_utf8(const char* msg, ...);
void print_utf16(const unsigned short* msg, ...);