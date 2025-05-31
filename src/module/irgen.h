#pragma once

// [ DEFINING ] //

typedef enum {
	
	// General units
	UNIT_TYPE_UNDEFINED,
	
	UNIT_TYPE_IDENTIFIER,
	UNIT_TYPE_LITERAL,
	
	// Punctuator units
	UNIT_TYPE_PT_SEMICOLON,
	UNIT_TYPE_PT_COMMA,
	UNIT_TYPE_PT_COLON,
	UNIT_TYPE_PT_AMPERSAND,
	UNIT_TYPE_PT_ASTERISK,
	
	// Keyword units
	UNIT_TYPE_KW_FUNC,
	UNIT_TYPE_KW_RETURN,
	
	UNIT_TYPE_KW_CALL,
	UNIT_TYPE_KW_JUMP,
	
	UNIT_TYPE_KW_EXPORT,
	UNIT_TYPE_KW_IMPORT,
	
	UNIT_TYPE_KW_STATIC,
	UNIT_TYPE_KW_LOCAL,
	
	UNIT_TYPE_KW_MOVE,
	
	UNIT_TYPE_KW_ALLOC,
	UNIT_TYPE_KW_FREE,
	
	UNIT_TYPE_KW_ADD,
	UNIT_TYPE_KW_SUB,
	UNIT_TYPE_KW_MUL,
	UNIT_TYPE_KW_DIV,
	
	UNIT_TYPE_KW_ARG_PUSH,
	UNIT_TYPE_KW_ARG_POP,
	
	UNIT_TYPE_KW_END,
	
	// Type units
	UNIT_TYPE_TP_UK,
	
	UNIT_TYPE_TP_VOID,
	
	UNIT_TYPE_TP_S8,
	UNIT_TYPE_TP_S16,
	UNIT_TYPE_TP_S32,
	UNIT_TYPE_TP_S64,
	
	UNIT_TYPE_TP_U8,
	UNIT_TYPE_TP_U16,
	UNIT_TYPE_TP_U32,
	UNIT_TYPE_TP_U64,
	
	UNIT_TYPE_TP_F32,
	UNIT_TYPE_TP_F64,
	UNIT_TYPE_TP_F128,
	
	// Register units
	UNIT_TYPE_RG_UK,
	
	UNIT_TYPE_RG_RETVAL,
	
	UNIT_TYPE_RG_AR1,
	UNIT_TYPE_RG_AR2,
	
	UNIT_TYPE_RG_RG1,
	UNIT_TYPE_RG_RG2,
	
	UNIT_TYPE_RG_ARG1,
	UNIT_TYPE_RG_ARG2,
	UNIT_TYPE_RG_ARG3,
	UNIT_TYPE_RG_ARG4,
	
} unit_type;

typedef struct {
	unit_type type;
	char value[MAX_VALUE_LEN];
} unit;

/*////////*/

typedef struct {
	ast* pAST;
	symbol_table* pSymbolTable;
} ir_info;

typedef struct {
	size_t index;
	size_t memSize;
	size_t size;
	unit_type lastRegister;
	unit_type lastRegisterType;
	unit* buffer;
} ir;

// [ FUNCTIONS ] //

bool ir_generate(ir* pIR, ir_info* pInfo);
void ir_print(ir* pIR);