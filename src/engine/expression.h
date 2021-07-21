/*
 * bool_expression.h
 *
 *  Created on: 21 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 *  Boolean expression of an event
 */

#ifndef SRC_LIB_EXPRESSION_H_
#define SRC_LIB_EXPRESSION_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../lib/mmt_lib.h"
#include "message_t.h"

#define UNKNOWN_REF_INDEX ((uint16_t)UNKNOWN)

/**
 * Constant
 */
typedef struct{
	enum data_type data_type;
	/**
	 * size of the pointer *data
	 */
	size_t data_size;
	void *data;
} constant_t;

/**
 * Convert from data types from MMT_DPI to #data_type that is
 * either a MMT_SEC_MSG_DATA_TYPE_NUMERIC or a MMT_SEC_MSG_DATA_TYPE_STRING
 */
int convert_data_type( int mmt_dpi_data_type );

/**
 * Variable
 */
typedef struct{
	int  data_type;
	int dpi_type;
	//a variable: TCP.SRC or TCP.SRC.1
	char *proto, *att;
	uint32_t proto_id, att_id;//those are generated from their name: tcp => 354
	uint16_t ref_index;
} variable_t;


enum operator{
	//boolean operator: or, and
  OR, AND,
  NOT,
  //comparison operators: not equal, equal, ...
  NEQ, EQ, GT, GTE, LT, LTE,
  //numeric operators: + - * /
  ADD, SUB, MUL, DIV,
  //a embedded function
  FUNCTION
};


/**
 * Expression that is x-ary expression:
 * that can be a function: name( param_1, param_2, ...)
 * or a boolean expression: or( param_1, param_2, ...)
 * or a calculation expression: *( param_1, param_2, ... )
 */
typedef struct operation_struct{
	//type id of return data
	enum data_type data_type;
	enum operator operator;

	/**
	 * Representation of operator in plain text
	 * that is either a function name or an operator
	 */
	char *name;
	/**
	 * Number of parameters
	 */
	size_t params_size;
	/**
	 * List of parameters, it data has type expression_t
	 */
	link_node_t *params_list;
}operation_t;

/**
 * An expression is either a variable, or a constant, or an operation
 */
typedef struct expression_struct{
	enum expression { VARIABLE, CONSTANT, OPERATION} type;

	union{
		constant_t *constant;
		variable_t *variable;
		operation_t *operation;
	};
	struct expression_struct *father;
}expression_t;



/**
 * Get a set of variables (distinguished by "proto" and "att") of an expression.
 * - Input:
 * 	+ expr
 * - Output:
 * 	+ create and assign a map containing unique variables
 * - Return:
 * 	+ number of unique variables
 */
size_t get_unique_variables_of_expression( const expression_t *expr, mmt_map_t **variables_map, bool has_ref);

/**
 * Parse a string to get expression
 * - Input
 * 	+ string:
 * 	+ size  : size of the string
 * - Output
 * 	+ expr  : that is a pointer points to the result
 * - Return
 * 	+ O if success
 * 	+ error code if fail
 * - Note:
 * 	use mmt_free to free the expr when one does not need it anymore
 */
int parse_expression( expression_t **expr, const char *string, size_t size );

/**
 * Convert an expression to a string
 * - Input
 *    + expr: the expression to be stringified
 * - Output
 *    + string that is a pointer points to the result.
 * - Return
 * 	+ the size of the result string
 * - Note:
 * 	use mmt_free to free the string when one does not need it anymore
 */
size_t expr_stringify_constant( char **string, const constant_t *expr);
size_t expr_stringify_variable( char **string, const variable_t *var);
size_t expr_stringify_operation( char **string, const operation_t *opt );
size_t expr_stringify_expression( char **string, const expression_t *expr);


constant_t *expr_create_a_constant( enum data_type type, size_t data_size, void *data );
variable_t *expr_create_a_variable( char *proto, char *attr, uint16_t ref_index );
operation_t *expr_create_an_operation( char *name, enum operator operator );
expression_t *expr_create_an_expression( enum expression type, void *data );

void expr_update_data_type( expression_t *expr );
/**
 * Free a constant
 */
void expr_free_a_constant( constant_t *, bool free_data);
void expr_free_a_variable( variable_t *, bool free_data);
void expr_free_an_operation( operation_t *, bool free_data);
void expr_free_an_expression( expression_t *, bool free_data);

constant_t *evaluate_expression( const expression_t *expr, const constant_t **constants, size_t const_size );


/**
 * Compare 2 variables by its "proto" and "att"
 */
static inline int compare_variable_name( const void *v1, const void *v2){
	variable_t *x = (variable_t *)v1, *y = (variable_t *)v2;
	int d1, d2;
	ASSERT( v1 != NULL && v2 != NULL, "Error: Variables are NULL" );
	d1 = strcmp( x->proto, y->proto );
	d2 = strcmp( x->att,   y->att );
	if( d1 == 0 && d2 == 0 )
		return 0;
	else if( d1 != 0 )
		return d1;
	else
		return d2;
}

static inline int compare_variable_name_and_index( const void *v1, const void *v2){
	variable_t *x = (variable_t *)v1, *y = (variable_t *)v2;
	int d1, d2, d3;
	ASSERT( v1 != NULL && v2 != NULL, "Error: Variables are NULL" );
	d1 = strcmp( x->proto, y->proto );
	d2 = strcmp( x->att,   y->att );
	d3 = x->ref_index - y->ref_index;
	if( d1 == 0 && d2 == 0 && d3 == 0 )
		return 0;
	else if( d1 != 0 )
		return d1;
	else if( d2 != 0 )
		return d2;
	else
		return d3;
}
#endif /* SRC_LIB_EXPRESSION_H_ */
