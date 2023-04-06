/*
 * expression.c
 *
 *  Created on: 21 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 */

#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <mmt_core.h>

#include "../lib/mmt_lib.h"
#include "mmt_utils.h"
#include "expression.h"

#define MAX_STR_SIZE 10000

/**
 * Convert from MMT_DPI_DATA_TYPE to MMT_SEC_DATA_TYPE that is either a MMT_SEC_MSG_DATA_TYPE_STRING or a MMT_SEC_MSG_DATA_TYPE_NUMERIC
 */
inline int convert_data_type( int type ){
	switch( type ){
	case MMT_U8_DATA:
	case MMT_U16_DATA:
	case MMT_U32_DATA:
	case MMT_U64_DATA:
	case MMT_DATA_PORT:
	case MMT_DATA_CHAR:
	case MMT_DATA_FLOAT:
		return MMT_SEC_MSG_DATA_TYPE_NUMERIC;

	case MMT_DATA_MAC_ADDR:
	case MMT_STRING_DATA:
	case MMT_STRING_LONG_DATA:
	case MMT_BINARY_VAR_DATA:
	case MMT_BINARY_DATA:
	case MMT_HEADER_LINE:
	case MMT_DATA_IP_ADDR:
	case MMT_DATA_IP6_ADDR:
	case MMT_STRING_DATA_POINTER:
		return MMT_SEC_MSG_DATA_TYPE_STRING;

	case MMT_DATA_POINTER:
	case MMT_DATA_PATH:
	case MMT_DATA_TIMEVAL:
	case MMT_DATA_BUFFER:
	case MMT_DATA_POINT:
	case MMT_DATA_PORT_RANGE:
	case MMT_DATA_DATE:
	case MMT_DATA_TIMEARG:
	case MMT_DATA_STRING_INDEX:
	case MMT_DATA_IP_NET:
	case MMT_DATA_LAYERID:
	case MMT_DATA_FILTER_STATE:
	case MMT_DATA_PARENT:
	case MMT_STATS:
		return MMT_SEC_MSG_DATA_TYPE_BINARY;
	default:
		return UNKNOWN;
	}
	return MMT_SEC_MSG_DATA_TYPE_STRING;
}

size_t str_trim( uint8_t *string, size_t size ){
	return size;
}
/**
 * public API
 */
size_t get_variables_inside_expression( const expression_t *expr ){
	size_t size = 0;
	return size;
}

/** Public API */
constant_t *expr_create_a_constant( enum data_type type, size_t data_size, void *data ){
	constant_t *cont = mmt_mem_alloc( sizeof( constant_t ));
	cont->data_type = type;
	cont->data_size = data_size;
	cont->data = data;
	return cont;
}

static inline void _to_lower( char *str){
	char *p;
	//change name to lower case
	for( p = str; *p != '\0'; p ++)
		*p = tolower( *p );
}

/** Public API */
variable_t *expr_create_a_variable( char *proto, char *attr, uint16_t ref_index ){
	variable_t *var = mmt_mem_alloc( sizeof( variable_t ));
	//_to_lower( proto );
	//_to_lower( attr );
	var->proto = proto;
	var->att   = attr;
	var->ref_index = ref_index;
	var->proto_id  = get_protocol_id_by_name( var->proto );
	ASSERT( var->proto_id != 0, "Error: Unknown protocol \"%s\".", proto );
	var->att_id    = get_attribute_id_by_protocol_id_and_attribute_name( var->proto_id, var->att );
	ASSERT( var->att_id != 0, "Error: Unknown attribute \"%s\" of protocol \"%s\".", attr, proto );
	var->dpi_type  = get_attribute_data_type( var->proto_id, var->att_id  );
	var->data_type = convert_data_type( var->dpi_type );

	ASSERT( var->data_type != UNKNOWN, "Error 2: Data type for %s.%s has not implemented yet.", proto, attr);

	return var;
}

/** Public API */
operation_t *expr_create_an_operation( char *name, enum operator operator ){
	operation_t *opt = mmt_mem_alloc( sizeof( operation_t ));
	opt->name = name;
	opt->operator = operator;
	opt->params_list = NULL;
	opt->data_type = UNKNOWN;
	opt->params_size = 0;
	return opt;
}

/** Public API */
expression_t *expr_create_an_expression( enum expression type, void *data ){
	expression_t *expr = mmt_mem_alloc( sizeof( expression_t));
	expr->type   = type;
	expr->father = NULL;
	switch( type ){
		case VARIABLE:
			expr->variable = data;
			break;
		case CONSTANT:
			expr->constant = data;
			break;
		case OPERATION:
			expr->operation = data;
			break;
	}
	return expr;
}

/** Public API */
void expr_free_a_constant( constant_t *x, bool free_data){
	if( free_data == YES )
		mmt_free_and_assign_to_null( x->data );
	mmt_free_and_assign_to_null( x );
}

/** Public API */
void expr_free_a_variable( variable_t *x, bool free_data){
	if( free_data == YES ){
		mmt_free_and_assign_to_null( x->proto );
		mmt_free_and_assign_to_null( x->att );
	}
	mmt_free_and_assign_to_null( x );
}

/** Public API */
void expr_free_an_operation( operation_t *x, bool free_data){
	link_node_t *ptr, *q;
	//free data and parameters of this operation
	if( free_data == YES){
		mmt_free_and_assign_to_null( x->name );

		ptr = x->params_list;
		while( ptr != NULL ){
			q = ptr;
			ptr = ptr->next;

			//free data of a node of linked-list
			expr_free_an_expression( (expression_t *) q->data, free_data );
			q->data = NULL;
			//free a node of linked-list
			mmt_free_and_assign_to_null( q );
			q = NULL;
		}
	}
	mmt_free_and_assign_to_null( x );
}

/** Public API */
void expr_free_an_expression( expression_t *expr, bool free_data){
	switch( expr->type ){
		case VARIABLE:
			expr_free_a_variable( expr->variable, free_data);
			break;
		case CONSTANT:
			expr_free_a_constant( expr->constant, free_data );
			break;
		case OPERATION:
			expr_free_an_operation( expr->operation, free_data );
			break;
	}
	mmt_free_and_assign_to_null( expr );
}

/**
 * Get index of the first character in string such that it is not a space
 * - Return:
 * 	+ 0 if there is no space
 * 	+ str_size if the string contains only space
 * 	+ index where the character is not a space
 */
static inline size_t _jump_space( const char *string, size_t str_size ){
	size_t i=0;
	if( (string == NULL) || (str_size == 0) )
			return 0;
	//a pointer created by mmt_malloc always have \0 at the end
	for( i=0; i<str_size; i++ )
		if( ! isspace( string[i] ) )
			return i;
	return 0;
}


size_t _parse_a_proto_name( char **name, const char *string, size_t str_size, bool is_allow_number_prefix ){
	size_t i = 0, index = 0;
	const char *temp;
	char *p;
	*name = NULL;
	if( string == NULL || str_size == 0 )
		return 0;

	index = _jump_space( string, str_size );

	//all of string are spaces
	if( index == str_size ) return index;

	temp = string + index;

	//whether allow a name start by: number/alphabet/_
	if( !is_allow_number_prefix )
		//first character must be an alphabet
		if( ! (isalpha(*temp ) || *temp == '_') )
			return index;

	//the next characters can be alphabet or digit
	while( (isalnum( *(temp+i) ) || *(temp +i) == '_' ) && index < str_size ){
		i++;
		index ++;
	}

	//duplicate the name
	*name = mmt_mem_dup( temp, i );

	return index;
}


size_t _parse_a_name( char **name, const char *string, size_t str_size ){
	return _parse_a_proto_name( name, string, str_size, false );
}
/**
 * Parse a string that is put inside by ' ' or " "
 */
size_t _parse_a_string( char **name, const char *string, size_t str_size ){
	size_t i = 0, index = 0;
	const char *temp;
	*name = NULL;

	if( string == NULL || str_size == 0 )
		return 0;

	index = _jump_space( string, str_size );

	//all of string are spaces
	if( index == str_size ) return index;

	temp = string + index;

	//first character must be the open bracket: " or '
	if( *temp  != '"' && *temp != '\'')
		return index;

	i = 1;
	index ++;
	//find a close bracket
	while( *(temp+i) != *temp && index < str_size ){
		i++;
		index ++;
	}

	//must be found the closer
	ASSERT( index < str_size, "Error 3.a: String is in incorrect format: %s", temp );

	//duplicate the name
	if( i-1 == 0)
		*name = mmt_mem_dup( "", 1 );
	else
		*name = mmt_mem_dup( temp + 1, i - 1 );

	//jump over the closer
	return index + 1;
}

/**
 * Parse a number: integer or float
 */
size_t _parse_a_number( double **num, const char *string, size_t str_size ){
	size_t i = 0, index = 0;
	bool has_dot = NO;
	const char *temp;
	char *str;
	*num = NULL;
	if( string == NULL || str_size == 0 )
		return 0;

	index = _jump_space( string, str_size );

	//all of string are spaces
	if( index == str_size ) return index;

	temp = string + index;
	//first character must be an number
	if( ! isdigit(*temp ))
		return index;

	//the next characters can be a dot or a digit
	while( (isdigit(*(temp+i)) || *(temp+i) == '.' ) && index < str_size ){
		if( *(temp+i) == '.' ){
			//already saw a dot then we break at the second dot
			if( has_dot == YES )
				break;
			else
				has_dot = YES;
		}

		i++;
		index ++;
	}

	//duplicate the name
	str  = mmt_mem_dup( temp, i );
	*num  = mmt_mem_alloc( sizeof( double ));
	**num = atof( str );

	mmt_mem_free( str );

	return index;
}

/**
 * get number of digits of an integer
 */
size_t _num_digits( int n ){
	if (n < 0) return _num_digits ((n == INT_MIN) ? INT_MAX : -n);
	if (n < 10) return 1;
	return 1 + _num_digits (n / 10);
}

/**
 * A constant is either a number or a string
 */
size_t _parse_constant( constant_t **expr, const char *string, size_t str_size ){
	size_t index;
	char *name = NULL;
	double *number = NULL;
	*expr = NULL;
	if( string == NULL  || str_size == 0 )
		return 0;

	index = _parse_a_number( &number, string, str_size );
	//found a number
	if( number != NULL ){
		*expr = expr_create_a_constant(MMT_SEC_MSG_DATA_TYPE_NUMERIC, sizeof( double), number);
		return index;
	}

	//not found any number => find a string
	index = _parse_a_string( &name, string, str_size );
	if( name != NULL ){
		*expr = expr_create_a_constant(MMT_SEC_MSG_DATA_TYPE_STRING, mmt_mem_size( name ), name);
		return index;
	}
	return index;
}

/**
 * Parse a variable that is in format: proto.att[.index], e.g., TCP.SRC or TCP.SRC.1
 */
size_t _parse_variable( variable_t **expr, const char *string, size_t str_size ){
	size_t index, old_index;
	bool is_start_by_number;
	char *str_1 = NULL, *str_2 = NULL;
	uint16_t ref_index = UNKNOWN_REF_INDEX;
	char const *temp;
	double *num = NULL;

	*expr = NULL;

	index = _jump_space( string, str_size );
	if( (string == NULL) || (str_size == 0) || index == str_size )
		return index;

	temp = string + index;

	//mark: if started by a number => must be PROTO.ATT
	is_start_by_number = isdigit( *temp );
	old_index = index;

	index = _parse_a_proto_name( &str_1, temp, str_size, true );

	//has proto?
	if( str_1 != NULL ){
		//must have a dot
		if( string[ index ] == '.' ){
			index ++; //jump over this dot
			temp = string + index ;
			index += _parse_a_proto_name( &str_2, temp,  str_size - index, true );
			//has att ?
			if( str_2 != NULL ){
				//has index ?
				if( string[ index ] == '.' ){
					index ++; //jump over the second dot
					temp = string + index; //2 dots
					index += _parse_a_number( &num, temp, str_size - index );
					ref_index = (uint16_t ) (*num);
					mmt_free_and_assign_to_null( num );
				}

				*expr = expr_create_a_variable( str_1, str_2, ref_index );
				return index;
			}
			else
				mmt_free_and_assign_to_null( str_1 );
		} else {
			//this is not a proto.att
			//=> do not allow to start by a number
			if( is_start_by_number ){
				mmt_free_and_assign_to_null( str_1 );
				return old_index;
			}
		}
	}

	return index;
}

static inline char _get_the_next_char( const char *string ){
	while( isspace( *string )) string ++;
	return *string;
}

static inline bool _parse_a_boolean_expression( bool is_first_time, expression_t *expr, const char *string ){
	//parse expression and create sub-tree, expr->operator = top operator
	//((ARP.OPCODE == 2)&&(ARP.SRC_PROTO == ARP.SRC_PROTO.1))
	//OR, AND, NEQ, EQ, GT, GTE, LT, LTE, THEN, COMPUTE, XC, XCE, XD, XDE, XE, ADD, SUB, MUL, DIV
	size_t index;

	const char *temp = string;
	const char *temp2 = string;
	double *new_number = NULL;
	expression_t *new_expr;
	operation_t *new_op;
	variable_t *new_var;
	char *new_string;

	//jump
	while ( isspace(*temp))temp++;

	if (*temp == '(') {
		temp2 = temp + 1;
		if ( is_first_time == YES) {
			_parse_a_boolean_expression(NO, expr, temp2);
		} else {
			//create new_expr
			//we have not known yet the operator of new_op
			//it will be determined after
			new_op = expr_create_an_operation(NULL, UNKNOWN );
			new_expr = expr_create_an_expression( OPERATION, new_op );
			new_expr->father = expr;
			//append new_expr to expr->params_list
			expr->operation->params_list = append_node_to_link_list( expr->operation->params_list, new_expr );
			expr->operation->params_size ++;

			_parse_a_boolean_expression(NO, new_expr, temp2);
		}
	} else if (*temp == ')') {
		ASSERT( expr->father != NULL || _get_the_next_char(temp + 1) == '\0', "Error 37d: Unexpected: %s", temp + 1 );
		_parse_a_boolean_expression(NO, expr->father, temp + 1);
	} else if (*temp == '\0') {
		//do nothing
	}  else if (*temp == '\'') {
		//a 'string'
		index = _parse_a_string( &new_string, temp, MAX_STR_SIZE );

		new_expr = expr_create_an_expression( CONSTANT,
				expr_create_a_constant(MMT_SEC_MSG_DATA_TYPE_STRING, mmt_mem_size( new_string ), new_string) );
		new_expr->father = expr;

		//append new_expr to expr->params_list
		expr->operation->params_list = append_node_to_link_list( expr->operation->params_list, new_expr );
		expr->operation->params_size ++;
		_parse_a_boolean_expression(NO, expr, temp + index);
	} else if (isdigit(*temp) || isalpha(*temp) || *temp == '_') {

		//a variable: PROTO.FIELD.EVENT
		index = _parse_variable( &new_var, temp, MAX_STR_SIZE );

		//not a variable
		//=> give one more chance to check constant: (1) a number or (2) an id (true/false)
		if( new_var == NULL ){
			//(1) check number
			if( isdigit( *temp )){
				//a number 1.0
				index = _parse_a_number( &new_number, temp, MAX_STR_SIZE );
				//found a number
				//create a new expression
				new_expr = expr_create_an_expression( CONSTANT, expr_create_a_constant(MMT_SEC_MSG_DATA_TYPE_NUMERIC, sizeof( double), new_number) );
				new_expr->father = expr;
				//append new_expr to expr->params_list
				expr->operation->params_list = append_node_to_link_list( expr->operation->params_list, new_expr );
				expr->operation->params_size ++;
			} else {
				//(2) check id
				index = _parse_a_name( &new_string, temp, MAX_STR_SIZE );
				new_number = mmt_mem_alloc( sizeof( double ) );
				if( strcmp( new_string, "true") == 0 ){
					*new_number = true;
				}else if( strcmp( new_string, "false") == 0 ){
					*new_number = false;
				}else {
					mmt_mem_free( new_number );
					ABORT("Error 37c: Illegal name:\"%s\".\nExpected either \"true\", \"false\", or proto.att or proto.att.ref", temp );
				}
				new_expr = expr_create_an_expression( CONSTANT, expr_create_a_constant(MMT_SEC_MSG_DATA_TYPE_NUMERIC, sizeof( double), new_number) );
				new_expr->father = expr;
				//append new_expr to expr->params_list
				expr->operation->params_list = append_node_to_link_list( expr->operation->params_list, new_expr );
				expr->operation->params_size ++;
			}
		}else{
			new_expr = expr_create_an_expression( VARIABLE, new_var );
			new_expr->father = expr;
			//append new_expr to expr->params_list
			expr->operation->params_list = append_node_to_link_list( expr->operation->params_list, new_expr );
			expr->operation->params_size ++;
		}

		_parse_a_boolean_expression(NO, expr, temp + index);

	} else if (*temp == '#') {
		//an embedded function: #func(param_1, param_2)
		temp ++;
		index = _parse_a_name( &new_string, temp, MAX_STR_SIZE );

		new_op = expr_create_an_operation( new_string, FUNCTION );
		new_expr = expr_create_an_expression( OPERATION, new_op );
		new_expr->father = expr;

		//append new_expr to expr->params_list
		expr->operation->params_list = append_node_to_link_list( expr->operation->params_list, new_expr );
		expr->operation->params_size ++;
		//parse the parameters of this function
		_parse_a_boolean_expression(YES, new_expr, temp + index );
	} else if (isdigit(*temp)) {

	} else if (*temp == '&' && *(temp + 1) == '&') {
		// &&
		//must no change operator: either unknown for the first element in the list or the same
		//=> to explain: a && b || c
		//   need ((a && b) || c)
		ASSERT( (expr->type == OPERATION && (expr->operation->operator == AND || expr->operation->operator == UNKNOWN)),
						"Error 39a: Unexpect \"%s\"\n", temp );
		temp2 = temp + 2;
		expr->type = OPERATION;
		expr->operation->operator = AND;
		expr->operation->name = mmt_mem_dup( "&&", 2);
		_parse_a_boolean_expression(NO, expr, temp2);
	} else if (*temp == '|' && *(temp + 1) == '|') {
		// ||
		ASSERT( (expr->type == OPERATION && (expr->operation->operator == OR || expr->operation->operator == UNKNOWN)),
						"Error 39a: Unexpect \"%s\"\n", temp );
		temp2 = temp + 2;
		expr->type = OPERATION;
		expr->operation->operator = OR;
		expr->operation->name = mmt_mem_dup( "||", 2);
		_parse_a_boolean_expression(NO, expr, temp2);
	} else if (*temp == '=' && *(temp + 1) == '=') {
		// ==
		ASSERT( (expr->type == OPERATION && expr->operation->operator == UNKNOWN ),
						"Error 39a: Unexpect \"%s\"\n", temp );
		temp2 = temp + 2;
		// set value
		expr->type = OPERATION;
		expr->operation->operator = EQ;
		expr->operation->name = mmt_mem_dup( "==", 2);
		//continue
		_parse_a_boolean_expression(NO, expr, temp2);
	} else if (*temp == '!' && *(temp + 1) == '=') {
		// !=
		ASSERT( (expr->type == OPERATION && expr->operation->operator == UNKNOWN ),
						"Error 39a: Unexpect \"%s\"\n", temp );
		temp2 = temp + 2;
		expr->type = OPERATION;
		expr->operation->operator = NEQ;
		expr->operation->name = mmt_mem_dup( "!=", 2);
		_parse_a_boolean_expression(NO, expr, temp2);
	} else if (*temp == '>' && *(temp + 1) != '=') {
		// >
		ASSERT( (expr->type == OPERATION && expr->operation->operator == UNKNOWN ),
						"Error 39a: Unexpect \"%s\"\n", temp );
		temp2 = temp + 1;
		expr->type = OPERATION;
		expr->operation->operator = GT;
		expr->operation->name = mmt_mem_dup( ">", 1);
		_parse_a_boolean_expression(NO, expr, temp2);
	} else if (*temp == '>' && *(temp + 1) == '=') {
		// >=
		ASSERT( (expr->type == OPERATION && expr->operation->operator == UNKNOWN ),
						"Error 39a: Unexpect \"%s\"\n", temp );
		temp2 = temp + 2;
		expr->type = OPERATION;
		expr->operation->operator = GTE;
		expr->operation->name = mmt_mem_dup( ">=", 2);
		_parse_a_boolean_expression(NO, expr, temp2);
	} else if (*temp == '<' && *(temp + 1) != '=') {
		// <
		ASSERT( (expr->type == OPERATION && expr->operation->operator == UNKNOWN ),
						"Error 39a: Unexpect \"%s\"\n", temp );
		temp2 = temp + 1;
		expr->type = OPERATION;
		expr->operation->operator = LT;
		expr->operation->name = mmt_mem_dup( "<", 1);
		_parse_a_boolean_expression(NO, expr, temp2);
	} else if (*temp == '<' && *(temp + 1) == '=') {
		// <=
		ASSERT( (expr->type == OPERATION && expr->operation->operator == UNKNOWN ),
						"Error 39a: Unexpect \"%s\"\n", temp );
		temp2 = temp + 2;
		expr->type = OPERATION;
		expr->operation->operator = LTE;
		expr->operation->name = mmt_mem_dup( "<=", 2);
		_parse_a_boolean_expression(NO, expr, temp2);
	} else if (*temp == '+') {
		// +
		ASSERT( (expr->type == OPERATION && (expr->operation->operator == ADD || expr->operation->operator == UNKNOWN) ),
						"Error 39a: Unexpect \"%s\"\n", temp );
		temp2 = temp + 1;
		expr->type = OPERATION;
		expr->operation->operator = ADD;
		expr->operation->name = mmt_mem_dup( "+", 1);
		_parse_a_boolean_expression(NO, expr, temp2);
	} else if (*temp == '-') {
		// -
		ASSERT( (expr->type == OPERATION && (expr->operation->operator == SUB || expr->operation->operator == UNKNOWN)),
						"Error 39a: Unexpect \"%s\"\n", temp );
		temp2 = temp + 1;
		expr->type = OPERATION;
		expr->operation->operator = SUB;
		expr->operation->name = mmt_mem_dup( "-", 1);
		_parse_a_boolean_expression(NO, expr, temp2);
	} else if (*temp == '*') {
		// *
		ASSERT( (expr->type == OPERATION && (expr->operation->operator == MUL || expr->operation->operator == UNKNOWN)),
						"Error 39a: Unexpect \"%s\"\n", temp );
		temp2 = temp + 1;
		expr->type = OPERATION;
		expr->operation->operator = MUL;
		expr->operation->name = mmt_mem_dup( "*", 1);
		_parse_a_boolean_expression(NO, expr, temp2);
	} else if (*temp == '/') {
		// '/'
		ASSERT( (expr->type == OPERATION && (expr->operation->operator == DIV || expr->operation->operator == UNKNOWN)),
						"Error 39a: Unexpect \"%s\"\n", temp );
		temp2 = temp + 1;
		expr->type = OPERATION;
		expr->operation->operator = DIV;
		expr->operation->name = mmt_mem_dup( "/", 1);
		_parse_a_boolean_expression(NO, expr, temp2);
	} else if( *temp == ',' && expr->type == OPERATION && expr->operation->operator == FUNCTION ){
		temp ++;
		while( isspace( *temp)) temp ++;
		//waiting for another parameter
		ASSERT( *temp != ')', "Error 37b: Illegal delimiter of function %s: ,%c", expr->operation->name, *temp );
		//*tmp != ')' ==> continue parsing the next parameter of function
		_parse_a_boolean_expression(NO, expr, temp);
	}else{
		(void)fprintf(stderr, "Error 37: Illegal character found in boolean expression: %c%c.\n", *temp, *(temp + 1));
		exit(-1);
	}
	return 0;
}

/**
 * public API
 */
int parse_expression( expression_t **expr, const char *string, size_t str_size ){
	expression_t *ret = NULL;
	operation_t *new_op;
	*expr = NULL;
	if( string == NULL )
		return 0;

	//we have not known yet the operator of new_op
	//it will be determined after
	new_op = expr_create_an_operation(NULL, UNKNOWN );
	ret    = expr_create_an_expression( OPERATION, new_op );
	*expr  = ret;
	return _parse_a_boolean_expression( YES, ret, string );
}

/**
 * public API
 */
size_t expr_stringify_constant( char **string, const constant_t *expr){
	char buff[ MAX_STR_SIZE ];
	int size;
	double d;

	if( expr == NULL ){
		string = NULL;
		return 0;
	}

	if( expr->data_type == MMT_SEC_MSG_DATA_TYPE_NUMERIC ){
		d = *(double *)expr->data;
		//integer
		size = snprintf(buff, sizeof(buff), "%.2f", d);
		size --; //jump over last '\0';
		//remove zero at the end, e.g., 10.00 ==> 10
		while( size > 1 && buff[ size - 1 ] == '0' )
				size --;
		if( buff[ size - 1 ] == '.' ) size --;
	}else if( expr->data_type == MMT_SEC_MSG_DATA_TYPE_STRING ){
		size = snprintf( buff, sizeof(buff), "\"%s\"", (char *)expr->data );
	}else{
		size = snprintf( buff, sizeof(buff), "\"__na__\"");
	}

	*string = mmt_mem_dup( buff, size );

	return size;
}

/**
 * public API
 */
size_t expr_stringify_variable( char **string, const variable_t *var){
	size_t size = 0;
	char buff[ 250 ];
	*string = NULL;

	if( var == NULL ){
		return 0;
	}

	if( var->ref_index != UNKNOWN_REF_INDEX ){
		size = snprintf(buff, sizeof( buff ), "_%s_%s_%d", var->proto, var->att, var->ref_index);
	}else{
		size = snprintf(buff, sizeof( buff ), "_%s_%s", var->proto, var->att );
	}
	*string = mmt_mem_dup( buff, size );
	return size;
}

static inline bool _is_comparison_operator( int op ){
	return op == NEQ || op == EQ || op == GT || op == GTE || op == LT || op == LTE;
}

static inline bool _is_string_variable( const operation_t *opt ){
	link_node_t *ptr;
	expression_t *expr;
	ptr = opt->params_list;
	while( ptr != NULL ){
		expr = (expression_t *) ptr->data;
		if( expr->type != VARIABLE || expr->variable->data_type != MMT_SEC_MSG_DATA_TYPE_STRING )
			return NO;
		ptr = ptr->next;
	}
	return YES;
}

static inline bool _is_string_param( const operation_t *opt ){
	link_node_t *ptr;
	expression_t *expr;
	ptr = opt->params_list;
	while( ptr != NULL ){
		expr = (expression_t *) ptr->data;
		if( expr->type == VARIABLE && expr->variable->data_type != MMT_SEC_MSG_DATA_TYPE_STRING )
			return NO;
		if( expr->type == CONSTANT && expr->variable->data_type != MMT_SEC_MSG_DATA_TYPE_STRING )
			return NO;
		ptr = ptr->next;
	}
	return YES;
}
/**
 * Public API
 */
size_t expr_stringify_operation( char **string, const operation_t *opt ){
	char *tmp = NULL;
	const char *delim;
	link_node_t *node;
	size_t index = 0;
	expression_t *expr;

	char str[ MAX_STR_SIZE ];
	//change comparison of string to function: "a" == "b" ==> 0 == strcmp("a", "b")
	if( _is_comparison_operator( opt->operator ) && _is_string_param ( opt ) ){
		//delimiter
		delim = ",";
		index += snprintf( &str[ index ], MAX_STR_SIZE, "0 %s %s(", opt->name,
				_is_string_variable( opt) ? "mmt_mem_cmp" : "strcmp" );
	}else if( opt->operator == FUNCTION ){
		delim = ",";
		index += snprintf( &str[ index ], MAX_STR_SIZE, "%s(", opt->name );
	}else{
		delim = opt->name;
		index += snprintf( &str[ index ], MAX_STR_SIZE, "(" );
	}

	//parameters
	node = opt->params_list;
	//no parameter
	if( node == NULL ){
		index += snprintf( &str[ index ], MAX_STR_SIZE - index, ")");
	}
	else while( node != NULL ){
		expr = (expression_t *) node->data;
		(void) expr_stringify_expression( &tmp, expr );
		//the last parameter ==> no need delimiter but a close-bracket
		if( node->next == NULL ){
			index += snprintf( &str[ index ], MAX_STR_SIZE - index, "%s)", tmp);
		}else{
			index += snprintf( &str[ index ], MAX_STR_SIZE - index, "%s %s ", tmp , delim);
		}
		//tmp was created in expr_stringify_expression( &tmp ...
		mmt_mem_free( tmp );
		node = node->next;
	};

	//clone string
	*string = mmt_mem_dup( str, index );

	return index;
}
/**
 * public API
 */
size_t expr_stringify_expression( char **string, const expression_t *expr){
	//nothing to do
	if( expr == NULL ){
		string = NULL;
		return 0;
	}
	switch( expr->type ){
	case CONSTANT:
		return expr_stringify_constant( string, expr->constant );
	case VARIABLE:
		return expr_stringify_variable( string, expr->variable );
	case OPERATION:
		return expr_stringify_operation( string, expr->operation );
	default:
		DEBUG( "Undefined" );
		return 0;
	}
}


size_t _get_unique_variables_of_expression( const expression_t *expr, mmt_map_t *map ){
	size_t var_count = 0;
	void *ptr;
	link_node_t *p;

	if( expr == NULL ) return 0;

	switch( expr->type ){
	case VARIABLE:
		ptr = mmt_map_set_data( map, expr->variable, expr->variable, NO );
		if( ptr == NULL )
			var_count ++;
		break;
	case CONSTANT:
		break;
	case OPERATION:
		p = expr->operation->params_list;
		//get variables in parameters of the operation
		while( p != NULL ){
			var_count += _get_unique_variables_of_expression( (expression_t *) p->data, map );
			p = p->next;
		}
		break;
	}
	return var_count;
}

/**
 * Public API
 */
size_t get_unique_variables_of_expression( const expression_t *expr, mmt_map_t **variables_map, bool has_index ){
	size_t var_count = 0;
	mmt_map_t *map;

	*variables_map = NULL;
	if( expr == NULL ) return 0;
	if( has_index == YES )
		map = mmt_map_init( compare_variable_name_and_index );
	else
		map = mmt_map_init( compare_variable_name );

	var_count = _get_unique_variables_of_expression( expr, map );

	//free the map being allocated
	if( var_count == 0 )
		mmt_map_free( map, NO );
	else
		*variables_map = map;

	return var_count;
}
/**
 * public API
 */
constant_t *evaluate_expression( const expression_t *expr, const constant_t **constants, size_t const_size ){
	constant_t *ret = (constant_t *) mmt_mem_alloc( sizeof( constant_t ));
	return ret;
}


void expr_update_data_type( expression_t *expr ){

}
