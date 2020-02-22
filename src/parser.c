#include "parser.h"
#include "data_structures/stack.h"

/* GLOBAL VARIABLES - START */

grammar G;
table parseTable;
struct parseTree PT;
struct hashMap *nonTerminalMap;
struct hashMap *terminalMap;
struct firstAndFollow F;
extern struct stack *stack;

char nonTerminalStringRepresentations[NUM_NON_TERMINALS][32] = {
	"<program>", "<moduleDeclarations>", "<moduleDeclaration>", "<otherModules>",
	"<driverModule>", "<module>", "<ret>", "<input_plist>",
	"<sub_input_plist>", "<output_plist>", "<sub_output_plist>", "<dataType>",
	"<dynamic_range>", "<type>", "<moduleDef>", "<statements>",
	"<statement>", "<ioStmt>", "<boolConstt>", "<extended_var>",
	"<var>", "<whichId>", "<simpleStmt>", "<assignmentStmt>",
	"<whichStmt>", "<lvalueIDStmt>", "<lvalueARRStmt>", "<index>",
	"<moduleReuseStmt>", "<optional>", "<idList>", "<sub_idList>",
	"<new_expression>", "<u>", "<sub_u>", "<expression>",
	"<N7>", "<AnyTerm>", "<N8>", "<arithmeticExpr>",
	"<sub_arithmeticExpr>", "<term>", "<sub_term>", "<factor>",
	"<op1>", "<op2>", "<logicalOp>", "<relationalOp>",
	"<declareStmt>", "<conditionalStmt>", "<caseStmt>", "<nullableCaseStmt>",
	"<value>", "<default>", "<iterativeStmt>", "<range>"};


char terminalStringRepresentations[NUM_TERMINALS][16] = {
		"ID", "NUM", "RNUM", "PLUS", "MINUS",
		"MUL", "DIV", "LT", "LE", "GE",
		"GT", "EQ", "NE", "DEF", "DRIVERDEF",
		"ENDDEF", "DRIVERENDDEF", "SQBO", "SQBC", "BO",
		"BC", "RANGEOP", "COLON", "ASSIGNOP", "SEMICOL",
		"COMMA", "INTEGER", "REAL", "BOOLEAN", "OF",
		"ARRAY", "START", "END", "DECLARE", "MODULE",
		"DRIVER", "PROGRAM", "GET_VALUE", "PRINT", "USE",
		"WITH", "PARAMETERS", "TRUE", "FALSE", "TAKES",
		"INPUT", "RETURNS", "AND", "OR", "FOR",
		"IN", "SWITCH", "CASE", "BREAK", "DEFAULT",
		"WHILE", "EPSILON"
	};

	
char terminalLiteralRepresentations[NUM_TERMINALS][16] = {
		"ID", "NUM", "RNUM", "+", "-",
		"*", "/", "<", "<=", ">",
		">=", "=", "!=", "<<", "<<<",
		">>", ">>>", "[", "]", "(",
		")", "..", ":", ":=", ";",
		",", "INTEGER", "REAL", "BOOLEAN", "of",
		"array", "start", "end", "declare", "module",
		"driver", "program", "get_value", "print", "use",
		"with", "parameters", "true", "false", "takes",
		"input", "returns", "AND", "OR", "for",
		"in", "switch", "case", "break", "default",
		"while", "EPSILON"
	};

/* GLOBAL VARIABLES - END */


/* HASH MAP Helper Code - START */

struct hashMap *getTerminalMap() {
	struct hashMap *map = initialiseHashMap();
	for (int i = 0; i < NUM_TERMINALS; ++i) {
		hashMapPut(terminalStringRepresentations[i], i, map);
	}

	return map;
}

struct hashMap *getNonTerminalMap() {
	struct hashMap *map = initialiseHashMap();	
	for (int i = 0; i < NUM_NON_TERMINALS; ++i) {
		hashMapPut(nonTerminalStringRepresentations[i], i, map);
	}

	return map;
}

struct rhsNode *createRhsNode(const char *val) {
	struct rhsNode *node = (struct rhsNode *) malloc(sizeof(struct rhsNode)); // TODO: Free this memory
	if (val[0] == '<') {
		node->flag = NON_TERMINAL;
		node->symbol.non_terminal = hashMapGet(val, nonTerminalMap);
		if (node->symbol.non_terminal == -1) {
			die(val);
		}
	} else {
		node->flag = TERMINAL;
		node->symbol.terminal = hashMapGet(val, terminalMap);
		if (node->symbol.terminal == -1) {
			die(val);
		}
	}
	return node;
}

/* HASH MAP Helper Code - END */


/* GRAMMAR Helper Code - START */

int extractRules(char *line, int ruleNumber) {
	/* Extract the non-termial LHS and the rules for that LHS from a given line in the grammar.
	*  @returns          the ruleNumber to continue populating from (input ruleNumber - return
	*                    value = number of rules added). */

	if (!( line[0] == '<')) {
		// Probably just a blank line.
		return ruleNumber;
	}

    // idx is the index of the character we are reading in the line
	// we do this parsing in a single pass so idx is monotonically
	// increasing for each line (and set to 0 between lines).
	int idx = 0;
	char lhsTermBuf[64];  // Exculsively for storing the LHS of the rule (rule->non_terminal).
	char termBuf[64];  // We reuse this to obtain every symbol/term in the RHS of the rule.
	struct rule *currRule = &G[ruleNumber];
	struct rhsNode **term = &currRule->head;

	// First extract the non-terminal from this line.
	while (line[idx] != ' ') {
		lhsTermBuf[idx] = line[idx];
		idx += 1;
	}
	lhsTermBuf[idx] = '\0';
	currRule->non_terminal = hashMapGet(lhsTermBuf, nonTerminalMap);

	// Skip over the whitespace in the middle.
	while (isspace(line[idx]) || line[idx] == '-' || line[idx] == '>') {
		idx += 1;
	}

	// Now read each rule and load the terms one by one.
	// Change the rule and rule number accordingly.
	int i;
	while (true) {
		switch (line[idx]) {
			case '|':  // Next rule for this LHS/non-terminal
				ruleNumber += 1;
				currRule = &G[ruleNumber];
				currRule->non_terminal = hashMapGet(lhsTermBuf, nonTerminalMap);
				term = &currRule->head;
				idx += 1;
				break;
			case '\n':  // End all rules for this LHS/non-terminal
				ruleNumber += 1;
				return ruleNumber;
			case ' ':  // Next term/symbol - or just before the pipe symbol
				idx += 1;
				break;
			default:  // Extract the term/symbol and add it as a rhsNode to the rule.
				i = 0;
				while (!isspace(line[idx])) {
					termBuf[i] = line[idx];
					idx += 1;
					i += 1;
				}
				termBuf[i] = '\0'; // Because of this we don't need to clear the buffer.
				*term = createRhsNode(termBuf);
				term = &(*term)->next;
		}
	}

	return ruleNumber;  // Control will not actually ever reach this point; case '\n' is the base case.
}

void loadGrammar(const char *filename) {
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		die("Could not open the grammar file.");
	}

	terminalMap = getTerminalMap();
	nonTerminalMap = getNonTerminalMap();

	int i = 0;
	ssize_t rd;
	long unsigned int line_buf_size = 512;
	char *line_buf = (char *) malloc(line_buf_size);
	while (true) {
		rd = getline(&line_buf, &line_buf_size, fp);
		if (rd == -1) {
			break;
		}
		i = extractRules(line_buf, i);  // i represents rule number.
	}

	free(line_buf);
	free(terminalMap);
	free(nonTerminalMap);
	fclose(fp);
	return;
}

void printGrammar() {
	struct rule *currRule;
	struct rhsNode *currTerm;
	for (int i = 0; i < NUM_RULES; ++i) {
		currRule = &G[i];
		currTerm = currRule->head;
		printf("%s ::=", nonTerminalStringRepresentations[(int)currRule->non_terminal]);
		while (currTerm != NULL) {
			if (currTerm->flag == TERMINAL) {
				printf(" %s", terminalStringRepresentations[(int)currTerm->symbol.terminal]);
			} else {
				printf(" %s", nonTerminalStringRepresentations[(int)currTerm->symbol.non_terminal]);
			}
			currTerm = currTerm->next;
		}
		printf("\n");
	}
}

/* GRAMMAR Helper Code - END */


/* FIRST & FOLLOW Helper Code - START */

void computeFollow() {
	F.follow[G[0].non_terminal][DOLLAR] = '1';

	struct rhsNode *ptr;
	struct rhsNode *iter;
	
	for(int loop = 0; loop < 2; loop++) {
		for(int i = 0; i < NUM_RULES; i++)  {
			enum nonTerminal non_terminal = G[i].non_terminal;
			ptr = G[i].head;

			while(ptr != NULL) {				
				if(ptr->flag == TERMINAL) {
					ptr = ptr->next;
					continue;
				}
				
				iter = ptr;

				while (true) {
					iter = iter->next;
					
					if(iter == NULL) {
						for (int i = 0; i < NUM_TERMINALS; i++) {
							if (F.follow[non_terminal][i] == '1') {
								if(i == EPSILON)
									continue;
								F.follow[ptr->symbol.non_terminal][i] = '1';
							}
						}
						break;
					}

					else if(iter->flag == TERMINAL) {
						F.follow[ptr->symbol.non_terminal][iter->symbol.terminal] = '1';
						break;
					}

					else {
						for (int i = 0; i < NUM_TERMINALS; i++) {
							if (F.first[iter->symbol.non_terminal][i] != -1) {
								if(i == EPSILON)
									continue;
								F.follow[ptr->symbol.non_terminal][i] = '1';
							}
						}

						if(F.first[iter->symbol.non_terminal][EPSILON] == -1){
							break;
						}
	 				}
				}
				
				ptr = ptr->next;
			}
		}
	} 

	return;
}


void computeFirstAndFollowSets() {
	printf("Computing first set ...\n");
	
	int dirty_bit_non_terminals[NUM_NON_TERMINALS];
	for (int i = 0; i < NUM_NON_TERMINALS; i++) {
		dirty_bit_non_terminals[i] = 0;
	}

	int dirty_bit_rules[NUM_RULES];
	for (int i = 0; i < NUM_RULES; i++) {
		dirty_bit_rules[i] = 1;
		dirty_bit_non_terminals[G[i].non_terminal] += 1;
	}

	int rules_used_count = 0;
	struct rhsNode *ptr_node;

	enum nonTerminal non_terminal;

	while (rules_used_count != NUM_RULES) {
		
		for (int i = 0; i < NUM_RULES; i++) {
			
			non_terminal = G[i].non_terminal;
			
			if (dirty_bit_non_terminals[non_terminal] == 0 || dirty_bit_rules[i] == 0)
				continue;
			
			ptr_node = G[i].head;
			
			while (ptr_node != NULL) {
				if (ptr_node->flag == TERMINAL) {
					F.first[non_terminal][ptr_node->symbol.terminal] = i;
					dirty_bit_rules[i] = 0;
					rules_used_count += 1;
					dirty_bit_non_terminals[G[i].non_terminal] -= 1;
					break;
				}
				
				else if (dirty_bit_non_terminals[ptr_node->symbol.non_terminal] != 0) {
					break;
				}
				
				else {
					for (int j = 0; j < NUM_TERMINALS; j++) {
						if (F.first[ptr_node->symbol.non_terminal][j] != -1 && j != EPSILON) {
							F.first[non_terminal][j] = i;
						}
					}

					if (F.first[ptr_node->symbol.non_terminal][EPSILON] == -1) {
						dirty_bit_rules[i] = 0;
						rules_used_count += 1;
						dirty_bit_non_terminals[G[i].non_terminal] -= 1;
						break;
					}

					if (ptr_node->next == NULL) {
						F.first[non_terminal][EPSILON] = i;
						dirty_bit_rules[i] = 0;
						rules_used_count += 1;
						dirty_bit_non_terminals[G[i].non_terminal] -= 1;
					}
					
					ptr_node = ptr_node->next;
				}
			}
		}
	}

	printf("Computing follow set ...\n");

	computeFollow();
}

/* FIRST & FOLLOW Helper Code - END */


void createParseTable() {
	for (int i = 0; i < NUM_NON_TERMINALS; i++) {
		for (int j = 0; j < NUM_TERMINALS; j++) {
			if (F.first[i][j] != -1 && j != EPSILON) {
					parseTable[i][j] = F.first[i][j];
			}
		}
		
		if (F.first[i][EPSILON] != -1) {
			for (int j = 0; j < NUM_TERMINALS; j++) {
				if (F.follow[i][j] == '1') {
					parseTable[i][j] = F.first[i][EPSILON];
				}
			}
		}
	}
}


/* PARSE SOURCE CODE Helper Code - START */

struct parseTree * initialiseParseTree() {
	struct parseTree * parse_tree = (struct parseTree *) malloc(sizeof(struct parseTree));
	return parse_tree;
}


struct treeNode * addTreeNode(struct rhsNode *rhs_node_ptr) {
	struct treeNode * tree_node_ptr = (struct treeNode *) malloc(sizeof(struct treeNode));
	
	if (rhs_node_ptr->flag == TERMINAL)
		tree_node_ptr->symbol.terminal.token = rhs_node_ptr->symbol.terminal;
	else
		tree_node_ptr->symbol.non_terminal = rhs_node_ptr->symbol.non_terminal;
	
	tree_node_ptr->flag = rhs_node_ptr->flag;
	tree_node_ptr->child = NULL;
	tree_node_ptr->next = NULL;
	
	return tree_node_ptr;
}


struct rhsNode *reverseRule(struct rhsNode *rhs_node_ptr) {
	struct rhsNode *curr_rhs_node_ptr = rhs_node_ptr;
	struct rhsNode *prev_rhs_node_ptr = NULL;
	struct rhsNode *temp_rhs_node_ptr = NULL;

	while(curr_rhs_node_ptr != NULL){
		temp_rhs_node_ptr = curr_rhs_node_ptr->next;
		curr_rhs_node_ptr->next = prev_rhs_node_ptr;
		prev_rhs_node_ptr = curr_rhs_node_ptr;
		curr_rhs_node_ptr = temp_rhs_node_ptr;
	}

	return prev_rhs_node_ptr;
}

 
struct treeNode *addRuleToStackAndTree(struct rule *grammar_rule) {
	// X -> PQR - we pop non terminal X and push in all elements of RHS of rule- in reverse order- RQP
	// X has already been popped- if we call this function
	// printf("Adding rule to stack and tree ...\n");

	struct rhsNode *curr_rhs_rule_node = grammar_rule->head;
	struct rhsNode *last_rhs_rule_node = NULL;

	struct stackNode *curr_stack_node_ptr = NULL;
	struct treeNode *curr_tree_node_ptr = NULL;
	struct treeNode *prev_tree_node_ptr = NULL;

	if (curr_rhs_rule_node->flag == TERMINAL && curr_rhs_rule_node->symbol.terminal == EPSILON)
		return addTreeNode(curr_rhs_rule_node);

	// first, temporarily reversing the order of temp order
	last_rhs_rule_node = reverseRule(curr_rhs_rule_node);
	curr_rhs_rule_node = last_rhs_rule_node;

	// now, they are in reverse order- now to insert elements as well as restore the original order
	while(curr_rhs_rule_node != NULL){
		// printf("TOKEN: %d\n", curr_rhs_rule_node->symbol);
		curr_tree_node_ptr = addTreeNode(curr_rhs_rule_node);
		curr_tree_node_ptr->next = prev_tree_node_ptr;
		prev_tree_node_ptr = curr_tree_node_ptr;
				
		curr_stack_node_ptr = addStackNode(curr_rhs_rule_node);
		curr_stack_node_ptr->tree_node_ptr = curr_tree_node_ptr;
		push(curr_stack_node_ptr);
		
		curr_rhs_rule_node = curr_rhs_rule_node->next;
	}

	// printf("Am I a seg fault?!\n");
	fflush(stdout);
	reverseRule(last_rhs_rule_node);

	// printf("Exiting addRuleToStackAndTree ...\n");
	// fflush(stdout);
	return curr_tree_node_ptr;
}

/* PARSE SOURCE CODE Helper Code - END */


void parseInputSourceCode(char *testcaseFile) {
	FILE *fp = fopen(testcaseFile, "r");

	if (fp == NULL) {
		printf("Error in opening the file!!\n");
		exit(0);
	}

	printf("Creating <program> parse tree node ...\n");
	struct treeNode *tree_node_ptr = (struct treeNode *) malloc(sizeof(struct treeNode));
	tree_node_ptr->symbol.non_terminal = _PROGRAM;
	tree_node_ptr->flag = NON_TERMINAL;
	tree_node_ptr->child = NULL;
	tree_node_ptr->next = NULL;

	printf("Initialising parse tree ...\n");
	PT.head = tree_node_ptr;

	printf("Initialising stack ...\n");
    initialiseStack();

    printf("Adding <program> to stack ...\n");
    struct stackNode *stack_node = (struct stackNode *) malloc(sizeof(struct stackNode));
    stack_node->symbol.non_terminal = _PROGRAM;
    stack_node->flag = NON_TERMINAL;
    stack_node->next = NULL;
    stack_node->tree_node_ptr = tree_node_ptr;
    push(stack_node);
    
    struct symbol symbol;
    stack_node = NULL;
    enum nonTerminal stack_top_non_terminal;
    enum terminal symbol_terminal;
    int rule_no;

	printf("\n");
	symbol = getNextToken(fp);
	printf("Token: %s\n", terminalStringRepresentations[symbol.token]);
	fflush(stdout);
	symbol_terminal = symbol.token;

    while(true) {

    	if(stack->head == NULL){
    		break;
    	}

    	printf("\n");
    	printf("Printing stack ...\n");
    	printStack();
    	printf("\n");

    	if (stack->head->flag == NON_TERMINAL) {
    		stack_top_non_terminal = stack->head->symbol.non_terminal;
    		
    		// find next applicable rule
    		if (parseTable[stack_top_non_terminal][symbol_terminal] != -1) {
    			rule_no = parseTable[stack_top_non_terminal][symbol_terminal];
       			stack_node = pop();
				stack_node->tree_node_ptr->child = addRuleToStackAndTree(&G[rule_no]);
		    	// printf("Printing stack ...\n");
		    	// printStack();
		    	// fflush(stdout);
    		}

    		// if no rule, then ERROR state
    		else {
	    		printf("ERROR!!\n");
	    		fflush(stdout);
	    		break;
    		}
    	}

    	else if (stack->head->symbol.terminal == symbol_terminal) {
    		// pop the terminal
    		stack_node = pop();

    		// update information at tree node
    		stack_node->tree_node_ptr->symbol.terminal.line_no = symbol.line_no;

    		if (symbol.token == IDENTIFIER) {
    			strcpy(stack_node->tree_node_ptr->symbol.terminal.lexeme.str, symbol.lexeme.str);
    		}

    		else if (symbol.token == NUM) {
    			stack_node->tree_node_ptr->symbol.terminal.lexeme.num = symbol.lexeme.num;
    		}

    		else if (symbol.token == RNUM) {
    			stack_node->tree_node_ptr->symbol.terminal.lexeme.rnum = symbol.lexeme.rnum;
    		}
			
			// get the next token
			symbol = getNextToken(fp);
			symbol_terminal = symbol.token;
			printf("Token: %s\n", terminalStringRepresentations[symbol.token]);
			fflush(stdout);
    	}

    	else {
    		// ERROR state
    		printf("ERROR!!\n");
    		fflush(stdout);
    		break;
    	}
    	
    }

}


/* PRINT PARSE TREE Helper Code - START */

void writeNode(struct treeNode *ptr, struct treeNode *p_ptr, FILE *fp) {
	/*
		ptr - pointer to current node
		p_ptr - pointer to parent node
	*/
	// printf("Printing node ...\n");
	// fflush(stdout);
	
	// if (ptr->flag == TERMINAL)
	// 	printf("T: %d\n", ptr->symbol.terminal.token);
	// else
	// 	printf("NT: %d\n", ptr->symbol.non_terminal);

	char is_leaf_node[4];
	char parent_node_symbol[MAX_LEXEME_LEN];
	if (p_ptr != NULL) {
		strcpy(parent_node_symbol, nonTerminalStringRepresentations[p_ptr->symbol.non_terminal]);
	}
	else {
		strcpy(parent_node_symbol, "NULL");
	}

	if (ptr->flag == TERMINAL) {
		int line_no = ptr->symbol.terminal.line_no;
		char token_name[30];
		strcpy(token_name, terminalStringRepresentations[ptr->symbol.terminal.token]);
		strcpy(is_leaf_node, "YES");

		if (ptr->symbol.terminal.token == NUM) {
			int value_if_number = ptr->symbol.terminal.lexeme.num;
			// fprintf(fp, "---- \t %d \t %s \t %d \t %s \t %s \n",
			printf("%-25s %-15.4d %-15s %-25d %-25s %-25s\n",
				"", line_no, token_name, value_if_number,
				parent_node_symbol, is_leaf_node, "");
			fflush(stdout);
		}

		else if (ptr->symbol.terminal.token == RNUM) {
			float value_if_number = ptr->symbol.terminal.lexeme.rnum;
			// fprintf(fp, "---- \t %d \t %s \t %f \t %s \t %s \n",
			printf("%-25s %-15.4d %-15s %-25f %-25s %-25s\n",
				"", line_no, token_name, value_if_number,
				parent_node_symbol, is_leaf_node, "");
			fflush(stdout);
		}

		else {
			char lexeme[MAX_LEXEME_LEN];

			if (ptr->symbol.terminal.token == EPSILON) {
				return;
			}
			
			if (ptr->symbol.terminal.token == IDENTIFIER) {
				strcpy(lexeme, ptr->symbol.terminal.lexeme.str);
			}

			else {
				strcpy(lexeme, terminalLiteralRepresentations[ptr->symbol.terminal.token]);
			}

			// fprintf(fp, "%s \t %d \t %s \t  \t %s \t %s \n",
			printf("%-25s %-15.4d %-15s %-25s %-25s %-25s %-25s\n",
				lexeme, line_no, token_name, "", parent_node_symbol, is_leaf_node, "");
			fflush(stdout);
		}
	}

	else {
		strcpy(is_leaf_node, "NO");
		char node_symbol[30];
		strcpy(node_symbol, nonTerminalStringRepresentations[ptr->symbol.non_terminal]);
		// fprintf(fp, "%-25s %-25.4d %-25s %-25d %-25s %-25s\n",
		// 	parent_node_symbol, is_leaf_node, node_symbol);
		printf("%-25s %-15s %-15s %-25s %-25s %-15s %s\n",
				"", "", "", "", parent_node_symbol, is_leaf_node, node_symbol);
	}
}


struct treeNode *recursiveInOrderPrint(struct treeNode *ptr, struct treeNode *p_ptr, FILE *fp) {
	if (ptr == NULL)
		return NULL;

	// printf("Running recursiveInOrderPrint ...\n");
	// if (ptr->flag == TERMINAL)
	// 	printf("T: %d\n", ptr->symbol.terminal.token);
	// else
	// 	printf("NT: %d\n", ptr->symbol.non_terminal);

	fflush(stdout);
	
	
	struct treeNode *last_child = recursiveInOrderPrint(ptr->child, ptr, fp);
	
	writeNode(ptr, p_ptr, fp);
	
	if (last_child != NULL) {
		recursiveInOrderPrint(last_child, ptr, fp);
	}
	
	if (ptr->next != NULL && ptr->next->next == NULL) {
		return ptr->next;
	}
	
	return recursiveInOrderPrint(ptr->next, ptr, fp);
}

/* PRINT PARSE TREE Helper Code - END */


void printParseTree(char *outfile) {
    printf("Printing parse tree ...\n");
    fflush(stdout);

    FILE * fp = fopen(outfile, "w");

    if(fp == NULL) {
        printf("Error in opening the file!\n");
        return;
    }

	struct treeNode *ptr = PT.head;

	printf("%-25s %-15s %-15s %-25s %-25s %-15s %s\n",
		   "lexeme", "lineno", "token name", "num value",
		   "parent node symbol", "leaf node", "node symbol");

	recursiveInOrderPrint(ptr, NULL, fp);
}
