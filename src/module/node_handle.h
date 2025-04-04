
void node_handle_identifier(node* currentNode, token* currentToken, stream* pStream, node* pParent, symbol_table* pSymbolTable) {
	
	// Level 2 highest, level 3 lowest
	// This node is a leaf
	
	currentNode->type = NODE_TYPE_IDENTIFIER;
	
	
}

void node_handle_literal(node* currentNode, token* currentToken, stream* pStream, node* pParent, symbol_table* pSymbolTable) {
	
	// Level any (under function level)
	// This node is a branch
	
	// Get the length of the first token's value
	size_t valueLen = strlen(currentNode->tokenList[0].value);
	
	char* value = currentNode->tokenList[0].value;
	
	// Check if this is a char, string, int, or float literal
	if (value[0] == '\"') {
		currentNode->type = NODE_TYPE_LITERAL_STR;
	} else if ((value[0] == '0') && (value[1] == 'x')) {
		currentNode->type = NODE_TYPE_LITERAL_INT_HEX;
	} else if (value[0] == '\'') {
		currentNode->type = NODE_TYPE_LITERAL_CHAR;
	} else if (isdigit(value[0])) {
		currentNode->type = NODE_TYPE_LITERAL_INT;
	} else if (value[valueLen - 1] == 'f') {
		currentNode->type = NODE_TYPE_LITERAL_FLOAT;
	} else {
		currentNode->type = NODE_TYPE_INVALID;
	}
	
}

/*////////*/

