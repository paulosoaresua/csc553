/*
 * symbol-table.h
 *
 * Author: Saumya Debray
 */

#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include "set.h"

#define Global 0
#define Local  1

typedef struct VarListNode {
  struct stblnode* var;
  struct VarListNode* next;
} var_list_node;

typedef struct stblnode {
  char *name;
  int scope;
  bool formal;             /* true if formal, false o/2 */
  int type;                /* the type of the symbol */
  int elt_type;            /* the type of array elements */
  int num_elts;            /* no. of array elements */
  int ret_type;            /* the return type of a function */
  struct stblnode *formals;  /* the list of formals for a function */
  int fn_proto_state;      /* status of prototype definitions for a function */
  bool is_extern;          /* whether or not an ID was declared as an extern */
  int byte_size;           // Byte size of the local variables and temporaries
  int fp_offset;           // Memory location as an offset of the frame pointer
  int num_formals;         // Number of formal parameters
  char* const_str;         // Constant string
  int const_val;           // Variable with a constant value
  bool is_temporary;
  bool is_constant;
  struct stblnode *next_free; // List of free temporaries
  struct stblnode *next;

  // For code optimization
  unsigned long long id; // Unique global ID
  set definitions; // Set of other instructions withing a block where the
  // variable defined by an assignment instruction is redefined (it's only
  // used if the instruction is an assignment like one).
  struct stblnode* copied_from; // Stored during copy propagation
  var_list_node* copied_to; // List of variables
} symtabnode;

/*********************************************************************
 *                                                                   *
 *                             Prototypes                            *
 *                                                                   *
 *********************************************************************/

void SymTabInit(int sc); // initialize the symbol table at scope sc to empty
symtabnode *SymTabLookup(char *str, int sc); // lookup scope sc
symtabnode *SymTabLookupAll(char *str); // lookup local first, then global
symtabnode *SymTabInsert(char *str, int sc);  // add ident to symbol table
symtabnode *SymTabRecordFunInfo(bool isProto);
void CleanupFnInfo(void);
/*
 * Debugging functions
 */
void printSTNode(symtabnode *stptr);
void DumpSymTabLocal();
void DumpSymTabGlobal();
void DumpSymTab();

/*********************************************************************
 *                                                                   *
 *          Functions: states for prototypes and definitions         *
 *                                                                   *
 *********************************************************************/

#define FN_PROTO     0  // prototype seen
#define FN_DEFINED   1  // definition seen

/*********************************************************************
 *                                                                   *
 *          Functions: codegen                                       *
 *                                                                   *
 *********************************************************************/

static int tmp_counter = 0;

/**
 * Creates a local entry for a temporary variable in the symbol table.
 *
 * @param type: type of the temporary
 *
 * @return pointer to the newly created entry
 */
symtabnode *create_temporary(int type);

/**
 * Places the temporary in the list of available temporaries (for reuse) of
 * its type.
 */
void free_temporary(symtabnode* tmp);

/**
 * Creates a symbol table node to contain a string constant. This node is not
 * added to the symbol table, but collected separately.
 *
 * @param str: content of the string
 *
 * @return pointer to the newly created entry
 */
symtabnode *create_constant_string(char* str);

/**
 * Creates a symbol table node to contain a constant value. This node is not
 * added to the symbol table, it can be used as a RHS variable in some
 * instructions.
 *
 * @param type: type of the variable
 * @param value: value of the constant
 *
 * @return pointer to the newly created entry
 */
symtabnode *create_constant_variable(int type, int value);

/**
 * Returns the head of the list of strings created.
 *
 * @return head of the string list
 */
symtabnode *get_string_list_head();

/**
 * Traverses the local symbol table and fills memory address for each local
 * variable as offsets relative to the frame pointer.
 */
int fill_local_allocations();

/**
 * Returns the symbol table entries for global variables.
 *
 * @return symbol table entries
 */
symtabnode** get_globals();

/**
 * Set a unique numeric id to each local variable. This will help in the
 * program analysis stage.
 *
 * @param node: symbol table entry for the variable
 */
void fill_id(symtabnode* node);

/**
 * Gets the total number of local variables created in a function
 *
 * @return
 */
int get_total_local_variables();

/**
 * Adds a variable to a list of variables.
 *
 * @param var: symbol table entry
 * @param list_head: head of a list of variables
 *
 * @return head of the new list.
 */
var_list_node*add_to_list_of_variables(symtabnode* var, var_list_node* list_head);

/**
 * Removes a variable from a list of variables.
 *
 * @param var: symbol table entry
 * @param list_head: head of a list of variables
 *
 * @return head of the new list.
 */
var_list_node*remove_from_list_of_variables(symtabnode* var, var_list_node* list_head);

/**
 * Frees pointers in a list of variables.
 *
 * @param list_head: head of a list of variables
 */
void clear_list_of_variables(var_list_node* list_head);

#endif /* _SYMBOL_TABLE_H_ */
