#ifndef ASTDEF_H

#define ASTDEF_H

#include "grammar.h"
#include "lexerDef.h"
#include "intCodeDef.h"

#define IDENTIFIER_NAME_MAX_LEN 20

/* ----- BEGIN UNIONS ----- */
union ASTNodesUnion {
  struct ProgramNode *pro;
  struct ModuleDeclarationNode *mod_dec;
  struct OtherModuleNode *oth_mod;
  struct ModuleNode *mod;
  struct InputPlistNode *inp_pli;
  struct OutputPlistNode *out_pli;
  struct LeafNode *lea;
  struct ArrayTypeNode *arr_typ;
  struct DynamicRangeNode *dyn_ran;
  struct StatementNode *stm;
  struct InputNode *inp;
  struct PrintNode *pri;
  struct ArrayNode *arr;
  struct AssignStmtNode *agn_stm;
  struct LvalueIDNode *lva_id;
  struct LvalueARRNode *lva_arr;
  struct ModuleReuseStmtNode *mod_reu_stm;
  struct IdListNode *id_lis;
  struct UNode *u;
  struct N7Node *n7;
  struct N8Node *n8;
  struct ArithmeticExprNode *ari_exp;
  struct TermNode *ter;
  struct DeclareStmtNode *dec_stm;
  struct ConditionalStmtNode *con_stm;
  struct CaseStmtNode *cas_stm;
  struct ForIterativeStmtNode *for_ite_stm;
  struct WhileIterativeStmtNode *whi_ite_stm;
  struct RangeNode *ran;
};

union LeafNodeUnion {
  int num;
  float rnum;
  bool boolean;
  char entry[IDENTIFIER_NAME_MAX_LEN];
};
/* ----- END UNIONS ----- */

/* ----- BEGIN ENUMS ----- */
enum ASTNodesEnum {
  /* 00 */ PROGRAM_NODE,
  /* 01 */ MODULE_DECLARATION_NODE,
  /* 02 */ OTHER_MODULE_NODE,
  /* 03 */ MODULE_NODE,
  /* 04 */ INPUT_PLIST_NODE,
  /* 05 */ OUTPUT_PLIST_NODE,
  /* 06 */ LEAF_NODE,
  /* 07 */ ARRAY_TYPE_NODE,
  /* 08 */ DYNAMIC_RANGE_NODE,
  /* 09 */ STATEMENT_NODE,
  /* 10 */ INPUT_NODE,
  /* 11 */ PRINT_NODE,
  /* 12 */ ARRAY_NODE,
  /* 13 */ ASSIGN_STMT_NODE,
  /* 14 */ LVALUE_ID_NODE,
  /* 15 */ LVALUE_ARR_NODE,
  /* 16 */ MODULE_REUSE_STMT_NODE,
  /* 17 */ ID_LIST_NODE,
  /* 18 */ U_NODE,
  /* 19 */ N7_NODE,
  /* 20 */ N8_NODE,
  /* 21 */ ARITHMETIC_EXPR_NODE,
  /* 22 */ TERM_NODE,
  /* 23 */ DECLARE_STMT_NODE,
  /* 24 */ CONDITIONAL_STMT_NODE,
  /* 25 */ CASE_STMT_NODE,
  /* 26 */ FOR_ITERATIVE_STMT_NODE,
  /* 27 */ WHILE_ITERATIVE_STMT_NODE,
  /* 28 */ RANGE_NODE,
  /* 29 */ NULL_NODE                  
};
/* ----- END ENUMS ----- */

/* ----- BEGIN STRUCTS ----- */
struct ProgramNode {
  struct ModuleDeclarationNode *ptr1;
  struct OtherModuleNode *ptr2;
  struct StatementNode *ptr3;
  struct OtherModuleNode *ptr4;
  int driver_starting_line_number;
  int driver_ending_line_number;
  int starting_line_number;
  int ending_line_number;
};

struct ModuleDeclarationNode {
  struct LeafNode *ptr1;  /* WARNING: The scope won't be set for this. */
  struct ModuleDeclarationNode *ptr2;
};

struct OtherModuleNode {
  struct ModuleNode *ptr1;
  struct OtherModuleNode *ptr2;
};

struct ModuleNode {
  struct LeafNode *ptr1;
  struct InputPlistNode *ptr2;
  struct OutputPlistNode *ptr3;
  struct StatementNode *ptr4;
  int starting_line_number;
  int ending_line_number;
};

struct InputPlistNode {
  struct LeafNode *ptr1;
  struct Attribute *ptr2; 
  struct InputPlistNode *ptr3;
};

struct OutputPlistNode {
  struct LeafNode *ptr1;
  struct Attribute *ptr2;
  struct OutputPlistNode *ptr3;
};

struct LeafNode {
  bool nullData;
  enum terminal type; /* An enum TERMINAL value. One of BOOL, ID, NUM, RNUM. */
  union LeafNodeUnion value;
  int line_number;
  struct SymbolTable *scope;
  ICAddr addr;
};

struct ArrayTypeNode {
  struct LeafNode *ptr1;
  struct DynamicRangeNode *ptr2;
};

struct DynamicRangeNode {
  struct LeafNode *ptr1;  /* Lower bound */
  struct LeafNode *ptr2; /* Upper bound */
};

struct StatementNode {
  struct Attribute *ptr1;
  struct StatementNode *ptr2;
};

struct InputNode {
  struct LeafNode *ptr1;
};

struct PrintNode {
  struct Attribute *ptr1;
};

struct ArrayNode {
  struct LeafNode *ptr1;
  struct LeafNode *ptr2;
  ICAddr addr;
};

struct AssignStmtNode {
  struct LeafNode *ptr1;
  struct Attribute *ptr2;
  ICAddr addr;
};


struct LvalueIDNode {
  struct Attribute *ptr1;
  ICAddr addr;
};

struct LvalueARRNode {
  struct LeafNode *ptr1; //index
  struct Attribute *ptr2;
  ICAddr addr;
};

struct ModuleReuseStmtNode {
  struct IdListNode *ptr1;
  struct LeafNode *ptr2;
  struct IdListNode *ptr3;
  ICAddr addr;
};

struct IdListNode {
  struct LeafNode *ptr1;
  struct IdListNode *ptr2;
  ICAddr addr;
};

struct UNode {
  enum terminal op;
  struct Attribute *ptr1;
  enum terminal data_type;
  int line_number;
  ICAddr addr;
};

struct N7Node {
  struct Attribute *ptr1;
  enum terminal logicalOp;
  struct Attribute *ptr2;
  int line_number;
  ICAddr addr;
};

struct N8Node {
  struct Attribute *ptr1; /* The inherited attribute. */
  enum terminal relationalOp;
  struct Attribute *ptr2;
  int line_number;
  ICAddr addr;
};

struct ArithmeticExprNode {
  struct Attribute *ptr1; /* Sub Arithmetic Expression */
  bool is_first;
  enum terminal op;
  struct Attribute *ptr2; /* Term */
  struct Attribute *ptr3;
  enum terminal data_type;
  int line_number;
  ICAddr addr;
};

struct TermNode {
  struct Attribute *ptr1; /* Subterm */
  bool is_first;
  enum terminal op;
  struct Attribute *ptr2; /* Factor - If TermNode* == NULL then we use this.*/
  struct Attribute *ptr3;
  enum terminal data_type;
  int line_number;
  ICAddr addr;
};

struct DeclareStmtNode {
  struct IdListNode *ptr1;
  struct Attribute *ptr2;
};

struct ConditionalStmtNode {
  struct LeafNode *ptr1;
  struct CaseStmtNode *ptr2;
  struct StatementNode *ptr3;
  int starting_line_number;
  int ending_line_number;
};

struct CaseStmtNode {
  struct LeafNode *ptr1;
  struct StatementNode *ptr2;
  struct CaseStmtNode *ptr3;
};

struct ForIterativeStmtNode {
  struct LeafNode *ptr1;
  struct RangeNode *ptr2;
  struct StatementNode *ptr3;
  int starting_line_number;
  int ending_line_number;
};

struct WhileIterativeStmtNode {
  struct Attribute *ptr1;
  struct StatementNode *ptr2;
  int starting_line_number;
  int ending_line_number;
};

struct RangeNode {
  struct LeafNode *ptr1;  /* Lower bound */
  struct LeafNode *ptr2; /* Upper bound */
};

struct Attribute {
  union ASTNodesUnion node;
  enum ASTNodesEnum type;
  ICAddr addr;
};

/* ----- END STRUCTS ----- */

/* ----- BEGIN PARSER STRUCTURES ----- */
enum typeOfSymbol {
	TERMINAL,
	NON_TERMINAL
};

union nodeValue {
	struct symbol terminal;
	enum nonTerminal non_terminal;
};

struct treeNode {
	int rule_number;  /* Use this to index into grammar[]. */
	union nodeValue symbol;
	enum typeOfSymbol flag;
	struct treeNode *child;
	struct treeNode *next;
  struct Attribute syn;
  struct Attribute inh;
  enum terminal val;
};

struct parseTree {
	struct treeNode *head;
};
/* ----- END PARSER STRUCTURES ----- */

#endif
