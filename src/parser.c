#include "parser.h"
#include "data_structures/stack.h"

/* GLOBAL VARIABLES - START */

grammar G;
table parseTable;
struct parseTree PT;
struct firstAndFollow F;

struct hashMap *nonTerminalMap;
struct hashMap *terminalMap;
struct hashMap *terminalLiteralMap;

extern struct stack *stack;
extern int line_no;
int PT_num_nodes = 0;
long PT_size_bytes = 0;

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
	"<value>", "<default>", "<iterativeStmt>", "<range>"
};


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
	"WHILE", "EPSILON", "DOLLAR"
};


char terminalLiteralRepresentations[NUM_TERMINALS][16] = {
	"ID", "NUM", "RNUM", "+", "-",
	"*", "/", "<", "<=", ">=",
	">", "=", "!=", "<<", "<<<",
	">>", ">>>", "[", "]", "(",
	")", "..", ":", ":=", ";",
	",", "integer", "real", "boolean", "of",
	"array", "start", "end", "declare", "module",
	"driver", "program", "get_value", "print", "use",
	"with", "parameters", "true", "false", "takes",
	"input", "returns", "AND", "OR", "for",
	"in", "switch", "case", "break", "default",
	"while", "EPSILON", "$"
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

struct hashMap *getTerminalLiteralMap() {
	struct hashMap *map = initialiseHashMap();
	for (int i = 0; i < NUM_TERMINALS; ++i) {
		hashMapPut(terminalLiteralRepresentations[i], i, map);
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

/* HASH MAP Helper Code - END */

/* Node functions - START*/

int return_PT_node_number(){
	return PT_num_nodes;
}

long return_PT_node_size(){
	return PT_size_bytes;
}

void set_PT_node_num(){
	PT_num_nodes = 0;
	PT_size_bytes = 0;
}
/* Node functions - END*/

/* GRAMMAR Helper Code - START */

struct rhsNode *newRule(
    enum nonTerminal non_terminal,
    enum terminal terminal,
    enum typeOfSymbol flag) {

    struct rhsNode *ptr = (struct rhsNode *) malloc(sizeof(struct rhsNode));
    ptr->flag = flag;

    if (flag == TERMINAL)
        ptr->symbol.terminal = terminal;
    else
        ptr->symbol.non_terminal = non_terminal;

    ptr->next = NULL;

    return ptr;
}


struct rhsNode *createRhsNode(const char *val) {
	struct rhsNode *node = (struct rhsNode *) malloc(sizeof(struct rhsNode)); // TODO: Free this memory

	if (val[0] == '<') {
		node->flag = NON_TERMINAL;
		node->symbol.non_terminal = hashMapGet(val, nonTerminalMap);
		if (node->symbol.non_terminal == -1) {
			die(val);
		}
	}

	else {
		node->flag = TERMINAL;
		node->symbol.terminal = hashMapGet(val, terminalMap);
		if (node->symbol.terminal == -1) {
			die(val);
		}
	}

	return node;
}


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

	return;
}

/* GRAMMAR Helper Code - END */


/* FIRST & FOLLOW Helper Code - START */

void initializeFirstAndFollow() {
    for (int i = 0; i < NUM_NON_TERMINALS; i++) {
        for (int j = 0; j < NUM_TERMINALS; j++) {
            F.first[i][j] = -1;
            F.follow[i][j] = '0';
        }
    }

    return;
}


void computeFollow() {
	F.follow[G[0].non_terminal][DOLLAR] = '1';

	struct rhsNode *ptr;
	struct rhsNode *iter;

	enum nonTerminal non_terminal;

	for(int loop = 0; loop < 2; loop++) {
		for(int i = 0; i < NUM_RULES; i++)  {

			non_terminal = G[i].non_terminal;
			ptr = G[i].head;

			while(ptr != NULL) {

				if(ptr->flag == TERMINAL) {
					ptr = ptr->next;
					continue;
				}

				// iter iterates over all symbols to the right of current RHS non terminal
				// pointed to by ptr
				iter = ptr;
				while (true) {
					iter = iter->next;

					// CASE 1 - iter reaches end of rule
					if(iter == NULL) {
						// add follow of current LHS non terminal to
						// follow of current RHS non terminal
						for (int i = 0; i < NUM_TERMINALS; i++) {
							if (F.follow[non_terminal][i] == '1' && i != EPSILON)
								F.follow[ptr->symbol.non_terminal][i] = '1';
						}
						break;
					}

					// CASE 2 - iter reaches a terminal
					else if(iter->flag == TERMINAL) {
						// add terminal to the follow of current RHS non terminal and break
						F.follow[ptr->symbol.non_terminal][iter->symbol.terminal] = '1';
						break;
					}

					// CASE 3 - iter reaches a non terminal
					else {
						// add all terminals in the follow of iter non terminal to ptr non terminal
						for (int i = 0; i < NUM_TERMINALS; i++) {
							if (F.first[iter->symbol.non_terminal][i] != -1 && i != EPSILON)
								F.follow[ptr->symbol.non_terminal][i] = '1';
						}

						// if iter non terminal does NOT derive EPSILON then break
						if(F.first[iter->symbol.non_terminal][EPSILON] == -1)
							break;
	 				}
				}

				ptr = ptr->next;
			}
		}
	}

	return;
}


void computeFirstAndFollowSets() {
	// ditry bit for non terminals -
	// 0 if first for non terminal hasn't been computed
	// initialized to total number of rules for that non terminal

	// ditry bit for grammar rules -
	// 0 if grammar rule hasn't been used in first set computation
	// initialized to 1

	int dirty_bit_non_terminals[NUM_NON_TERMINALS];
	int dirty_bit_rules[NUM_RULES];

	for (int i = 0; i < NUM_NON_TERMINALS; i++) {
		dirty_bit_non_terminals[i] = 0;
	}

	for (int i = 0; i < NUM_RULES; i++) {
		dirty_bit_rules[i] = 1;
		dirty_bit_non_terminals[G[i].non_terminal] += 1;
	}

	struct rhsNode *ptr_node;
	enum nonTerminal non_terminal;

	// loop till all rules have used in first set computation
	int rules_used_count = 0;
	while (rules_used_count != NUM_RULES) {

		for (int i = 0; i < NUM_RULES; i++) {

			non_terminal = G[i].non_terminal;

			// first of non terminal has ALREADY been computed, OR
			// current rule has ALREADY been used
			if (dirty_bit_non_terminals[non_terminal] == 0 || dirty_bit_rules[i] == 0)
				continue;

			// iterate over symbols in the RHS of the current rule
			ptr_node = G[i].head;
			while (ptr_node != NULL) {

				// CASE 1 - symbol is a terminal
				if (ptr_node->flag == TERMINAL) {
					// add terminal to first of LHS non terminal
					F.first[non_terminal][ptr_node->symbol.terminal] = i;
					dirty_bit_rules[i] = 0;
					rules_used_count += 1;
					dirty_bit_non_terminals[non_terminal] -= 1;
					break;
				}

				// CASE 2 - symbol is a non terminal & it's first has NOT been computed
				else if (dirty_bit_non_terminals[ptr_node->symbol.non_terminal] != 0) {
					break;
				}

				// CASE 3
				else {
					// add all terminals in the first of RHS non terminal to
					// first of LHS non terminal
					for (int j = 0; j < NUM_TERMINALS; j++) {
						if (F.first[ptr_node->symbol.non_terminal][j] != -1 && j != EPSILON) {
							F.first[non_terminal][j] = i;
						}
					}

					// end the traversal through rhs if current LHS non terminal
					// does NOT derive EPSILON
					if (F.first[ptr_node->symbol.non_terminal][EPSILON] == -1) {
						dirty_bit_rules[i] = 0;
						rules_used_count += 1;
						dirty_bit_non_terminals[G[i].non_terminal] -= 1;
						break;
					}

					// if current LHS non terminal derives EPSILON and
					// is the last symbol in the rule THEN add EPSILON to first set
					if (ptr_node->next == NULL) {
						F.first[non_terminal][EPSILON] = i;
						dirty_bit_rules[i] = 0;
						rules_used_count += 1;
						dirty_bit_non_terminals[G[i].non_terminal] -= 1;
					}

					// continue traversal to the next symbol in RHS
					ptr_node = ptr_node->next;
				}
			}
		}
	}

	computeFollow();

	return;
}

/* FIRST & FOLLOW Helper Code - END */


void intializeParseTable() {
    for (int i = 0; i < NUM_NON_TERMINALS; i++) {
        for (int j = 0; j < NUM_TERMINALS; j++) {
            parseTable[i][j] = -1;
        }
    }

    return;
}

void createParseTable() {
	for (int i = 0; i < NUM_NON_TERMINALS; i++) {
		for (int j = 0; j < NUM_TERMINALS; j++) {

			if (F.first[i][j] != -1 && j != EPSILON) {
				parseTable[i][j] = F.first[i][j];
			}

			else if (parseTable[i][j] != -1) {
				// LL(1) Compatibility check.
				printGrammar();
				char *stri, *strj;
				stri = nonTerminalStringRepresentations[i];
				strj = terminalStringRepresentations[j];
				printf("Rule #%d and #%d clash - (%s, %s) violation of LL(1) compatibility.\n",
					   parseTable[i][j], F.first[i][j], stri, strj);
				die("LL(1) Compatibility violation");
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

	return;
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

	PT_num_nodes += 1;
	PT_size_bytes += sizeof(struct treeNode *);

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

struct treeNode *addRuleToStackAndTree(int rule_no) {
	// X -> PQR - we pop non terminal X and push in all elements of RHS of rule- in reverse order- RQP
	// X has already been popped- if we call this function

    struct rule *grammar_rule = &G[rule_no];

	struct rhsNode *curr_rhs_rule_node = grammar_rule->head;
	struct rhsNode *last_rhs_rule_node = NULL;

	struct stackNode *curr_stack_node_ptr = NULL;
	struct treeNode *curr_tree_node_ptr = NULL;
	struct treeNode *prev_tree_node_ptr = NULL;

	if (curr_rhs_rule_node->flag == TERMINAL && curr_rhs_rule_node->symbol.terminal == EPSILON)
		return addTreeNode(curr_rhs_rule_node);

	// first, temporarily reverse the order of temp order.
	last_rhs_rule_node = reverseRule(curr_rhs_rule_node);
	curr_rhs_rule_node = last_rhs_rule_node;

	// now, they are in reverse order - insert elements into the stack and tree.
	while(curr_rhs_rule_node != NULL){
		curr_tree_node_ptr = addTreeNode(curr_rhs_rule_node);
		curr_tree_node_ptr->next = prev_tree_node_ptr;
		prev_tree_node_ptr = curr_tree_node_ptr;

		curr_stack_node_ptr = addStackNode(curr_rhs_rule_node);
		curr_stack_node_ptr->tree_node_ptr = curr_tree_node_ptr;
		curr_stack_node_ptr->tree_node_parent_enum = grammar_rule->non_terminal;
		push(curr_stack_node_ptr);

		curr_rhs_rule_node = curr_rhs_rule_node->next;
	}

	// Now restore the original order of the rule.
	reverseRule(last_rhs_rule_node);

	return curr_tree_node_ptr;
}

void printSyntaxError(struct symbol symbol) {
	if (symbol.token == IDENTIFIER)
		printf("\nSYNTAX ERROR: %9d %30s %30s\n",
			symbol.line_no, symbol.lexeme.str, "----");
	else if (symbol.token == NUM)
		printf("\nSYNTAX ERROR: %9d %30d %30s\n",
			symbol.line_no, symbol.lexeme.num, "----");
	else if (symbol.token == RNUM)
		printf("\nSYNTAX ERROR: %9d %30f %30s\n",
			symbol.line_no, symbol.lexeme.rnum, "----");
	else
		printf("\nSYNTAX ERROR: %9d %30s %30s\n",
			symbol.line_no, terminalStringRepresentations[symbol.token], "----");
}

void syntaxErrorRecovery(FILE *fp, struct symbol *symbol, int which_recovery) {
    enum nonTerminal stack_top_non_terminal;
    enum terminal input_stream_terminal;

	if (which_recovery == 1) {
		while(stack->head != NULL && stack->head->flag == TERMINAL)
			pop();

		if (stack->head == NULL)
			return;
	}

	// discard terminals from input stream till terminal is in
	// the follow of stack top non terminal, and then discard
	// the stack top non terminal
	stack_top_non_terminal = stack->head->symbol.non_terminal;
	input_stream_terminal = symbol->token;
	while(true) {

		// input terminal is in follow of stack top non terminal, discard the non terminal
		if(F.follow[stack_top_non_terminal][input_stream_terminal] == '1') {
			pop();
			break;
		}

		// SPECIAL CASE - if input terminal is dollar, then discard symbols
		// from stack till dollar is in the follow of stack top non terminal
		else if(input_stream_terminal == DOLLAR) {

			while (stack->head != NULL) {
				if (stack->head->flag == NON_TERMINAL) {
					stack_top_non_terminal = stack->head->symbol.non_terminal;
					if (F.follow[stack_top_non_terminal][input_stream_terminal] == '1')
						break;
				}

				pop();
			}

			break;
		}

		// input terminal is not dollar and not in follow of stack top non terminal
		// discard current input symbol and get next
		else{
			getNextToken(fp, symbol);
			input_stream_terminal = symbol->token;
		}
	}

	return;
}

void populateTreeNodeWithSymbol(struct treeNode *tree_node_ptr, struct symbol symbol) {
	tree_node_ptr->symbol.terminal.line_no = symbol.line_no;

	if (symbol.token == IDENTIFIER) {
		strcpy(tree_node_ptr->symbol.terminal.lexeme.str, symbol.lexeme.str);
	}

	else if (symbol.token == NUM) {
		tree_node_ptr->symbol.terminal.lexeme.num = symbol.lexeme.num;
	}

	else if (symbol.token == RNUM) {
		tree_node_ptr->symbol.terminal.lexeme.rnum = symbol.lexeme.rnum;
	}

	return;
}

/* PARSE SOURCE CODE Helper Code - END */


void parseInputSourceCode(char *testcaseFile,int print_check) {
	FILE *fp = fopen(testcaseFile, "r");

	if (fp == NULL) {
		printf("Error in opening the file!!\n");
		exit(0);
	}

	int no_errors = 1;
	// Add <program> to root of the parsetree
	struct treeNode *tree_node_ptr = (struct treeNode *) malloc(sizeof(struct treeNode));
	tree_node_ptr->symbol.non_terminal = _PROGRAM;
	tree_node_ptr->flag = NON_TERMINAL;
	tree_node_ptr->child = NULL;
	tree_node_ptr->next = NULL;

	PT.head = tree_node_ptr;
	PT_num_nodes += 1;
	PT_size_bytes += sizeof(struct treeNode);

    initialiseStack();

    // Add <program> to stack
    struct stackNode *stack_node = (struct stackNode *) malloc(sizeof(struct stackNode));
    stack_node->symbol.non_terminal = _PROGRAM;
    stack_node->flag = NON_TERMINAL;
    stack_node->next = NULL;
    stack_node->tree_node_ptr = tree_node_ptr;
    push(stack_node);

    stack_node = NULL;

    // variables for later use
    struct symbol symbol;
    enum nonTerminal stack_top_non_terminal;
    enum terminal symbol_terminal;
    int rule_no;

	// reinitialize the global line no in lexer
	line_no = 1;

	getNextToken(fp, &symbol);
	symbol_terminal = symbol.token;


    while(true) {
    	if(stack->head == NULL){
    		break;
    	}

    	if (stack->head->flag == NON_TERMINAL) {
    		stack_top_non_terminal = stack->head->symbol.non_terminal;

    		// find next applicable rule
    		rule_no = parseTable[stack_top_non_terminal][symbol_terminal];
    		if (rule_no != -1) {
       			stack_node = pop();
				stack_node->tree_node_ptr->rule_number = rule_no;
				stack_node->tree_node_ptr->child = addRuleToStackAndTree(rule_no);
    		}

    		// SYNTAX ERROR - input TERMINAL not in first of stack top NON TERMINAL
    		else {
    			no_errors = 0;
    			printSyntaxError(symbol);
    			syntaxErrorRecovery(fp, &symbol, 0);
    			symbol_terminal = symbol.token;
    		}
    	}

    	// CASE 2 - stack top is a TERMINAL which matches input TERMINAL
    	else if (stack->head->symbol.terminal == symbol_terminal) {
    		// pop the terminal and update information at tree node
    		stack_node = pop();
    		populateTreeNodeWithSymbol(stack_node->tree_node_ptr, symbol);

			// get the next token
			getNextToken(fp, &symbol);
			symbol_terminal = symbol.token;
    	}

    	// CASE 3
		// SYNTAX ERROR - input TERMINAL not matching stack top TERMINAL
    	else {
    		no_errors = 0;
    		printSyntaxError(symbol);
			syntaxErrorRecovery(fp, &symbol, 1);
			symbol_terminal = symbol.token;
    	}
    }

    fclose(fp);
    if(no_errors == 1){
		if(print_check)
    		printf("Syntax Analysis completed successfully! No errors found.\n");
    }
	else{
		exit(EXIT_FAILURE);
	}
    return;
}


/* PRINT PARSE TREE Helper Code - START */

void writeNode(struct treeNode *ptr, struct treeNode *p_ptr, FILE *fp) {
	/*
		ptr - pointer to current node
		p_ptr - pointer to parent node
	*/

	char is_leaf_node[4];
	char parent_node_symbol[MAX_LEXEME_LEN * 3]; // the x3 part is just a temporary fix.
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
			fprintf(fp, "%-25s %-15.4d %-15s %-15d %-35s %-15s %s\n",
				"----", line_no, token_name, value_if_number,
				parent_node_symbol, is_leaf_node, "----");
		}

		else if (ptr->symbol.terminal.token == RNUM) {
			float value_if_number = ptr->symbol.terminal.lexeme.rnum;
			fprintf(fp, "%-25s %-15.4d %-15s %-15f %-35s %-15s %s\n",
				"----", line_no, token_name, value_if_number,
				parent_node_symbol, is_leaf_node, "----");
		}

		else {
			char lexeme[MAX_LEXEME_LEN];

			if (ptr->symbol.terminal.token == IDENTIFIER) {
				strcpy(lexeme, ptr->symbol.terminal.lexeme.str);
			}

			else {
				strcpy(lexeme, terminalLiteralRepresentations[ptr->symbol.terminal.token]);
			}

			fprintf(fp, "%-25s %-15.4d %-15s %-15s %-35s %-15s %-25s\n",
				lexeme, line_no, token_name, "----",
				parent_node_symbol, is_leaf_node, "----");
		}
	}

	else {
		strcpy(is_leaf_node, "NO");
		char node_symbol[30];
		strcpy(node_symbol, nonTerminalStringRepresentations[ptr->symbol.non_terminal]);
		fprintf(fp, "%-25s %-15s %-15s %-15s %-35s %-15s %s\n",
				"----", "----", "----", "----",
				parent_node_symbol, is_leaf_node, node_symbol);
	}
}

struct treeNode *recursiveInOrderPrint(struct treeNode *ptr, struct treeNode *p_ptr, FILE *fp) {
	if (ptr == NULL)
		return NULL;

	struct treeNode *next_child = recursiveInOrderPrint(ptr->child, ptr, fp);

	writeNode(ptr, p_ptr, fp);

	while (next_child != NULL) {
		recursiveInOrderPrint(next_child, ptr, fp);
		next_child = next_child->next;
	}

	return ptr->next;
}

/* PRINT PARSE TREE Helper Code - END */


void printParseTree() {

	struct treeNode *ptr = PT.head;

	fprintf(stdout, "%-25s %-15s %-15s %-15s %-35s %-15s %s\n\n",
		   "LEXEME", "LINE #", "TOKEN", "NUM VALUE",
		   "PARENT NODE", "IS LEAF?", "NODE SYMBOL");

	recursiveInOrderPrint(ptr, NULL, stdout);

	// printf("\nPrinted the PARSE TREE to file: %s.\n\n", outfile);
}
