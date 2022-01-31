/*
 * rule.h
 *
 *  Created on: 20 sept. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *  Modified 19 apr 2021:
 *    + support embedded function in if_satisfied
 *    + add DROP, FORWARD rules to support mmt-5greplay
 *
 *  Parse .xml property file
 */

#ifndef SRC_LIB_RULE_H_
#define SRC_LIB_RULE_H_

#include <stdint.h>
#include "expression.h"

typedef struct rule_event_struct{
	uint16_t id;	//identifier of event
	char* description;
	expression_t* expression;
}rule_event_t;

/**
 * Delay either by timer or counter (number of packets)
 */
typedef struct rule_delay_struct{
	/**
	 * Defines the validity period ([time_min, time_max]) of the left branch (e.g. context).
	 * default is 0,
	 * - if value is < 0 then event needs to be satisfied before,
	 * - if = 0 then in same packet,
	 * - if > 0 then after
	 */
	uint64_t time_min, time_max;
	/**
	 * Similar to [time_min, time_max] we can de ne [counter_min, counter_max] where the unit is the number of packets analysed.
	 * note that either delay or counter needs to be used not both
	 */
	uint64_t counter_min, counter_max;
} rule_delay_t;

enum delay{
  BEFORE, AFTER, SAME
};


/**
 * A node of a property/operator is either an event or an operator
 */
typedef struct rule_node_struct{
	enum { RULE_NODE_TYPE_EVENT, RULE_NODE_TYPE_OPERATOR } type;
	union{
		struct rule_event_struct    *event;
		struct rule_operator_struct *operator;
	};
}rule_node_t;

enum value {RULE_VALUE_COMPUTE, RULE_VALUE_THEN, RULE_VALUE_BEFORE, RULE_VALUE_OR, RULE_VALUE_AND, RULE_VALUE_NOT};

typedef struct rule_operator_struct{
	/**
	 * Operators are used to combine different events and build complex events.
	 * - THEN  operator is used to describe ordered events,
	 * - AND  operator is used to describe two events without any order and
	 * - OR  operator is used to describe the occurrence of a least one of the events.
	 * - NOT  negates the underlying sub-tree.
	 */
	enum value value;
	/**
	 * Gives the text that clearly explains what the complex event is about.
	 */
	char *description;
	/**
	 * Defines the validity period ([delay_min, delay_max]) of the left branch (e.g. context).
	 * - If we have [a,b] with a=b=0 then the left branch needs to occur in the same time as the right branch.
	 * - If we have a<b<=0 then the right branch needs to have occurred in the past time interval with respect to the left branch.
	 * - If we have 0<=a<b then the right branch needs to occur in the future with respect to the left branch.
	 */
	rule_delay_t *delay;
	/**
	 * Allows detecting the repetition of a complex event occurrence a number of times.
	 */
	uint8_t repeat_times; //optional, default 1.

	/**
	 * Context
	 */
	rule_node_t *context;
	/**
	 * Consequence
	 */
	rule_node_t *trigger;
} rule_operator_t;

static const char *rule_type_string[] = {
		"ATTACK", "SECURITY", "EVASION", "TEST", "FORWARD"
};

typedef struct rule_struct{
	/**
	 * Identifier of the property
	 * should go from 1 to n where n is the total number of properties in the XML
	 */
	uint16_t id;
	/**
	 * Indicates that the property specifies a potential attack (or abnormal behavior)
	 * or that the property specifies a engine rule that needs to be respected.
	 */
	enum {RULE_TYPE_ATTACK, RULE_TYPE_SECURITY, RULE_TYPE_EVASION, RULE_TYPE_TEST, RULE_TYPE_FORWARD} type;

	enum value value;
	/**
	 * Gives the text that clearly explains what the property is about.
	 */
	char *description;
	/**
	 * Defines the validity period ([delay_min, delay_max]) of the left branch (e.g. context).
	 * - If we have [a,b] with a=b=0 then the left branch needs to occur in the same time as the right branch.
	 * - If we have a<b<=0 then the right branch needs to have occurred in the past time interval with respect to the left branch.
	 * - If we have 0<=a<b then the right branch needs to occur in the future with respect to the left branch.
	 */
	rule_delay_t *delay;
	/**
	 * Defines what action should be performed if the property is satisfied (or not, in the case of if_not_satisfied).
	 * The string gives the name of the script that should be executed, using the format: name(parameters, if any, separated by commas).
	 * The parameters can be constants, attributes of the event that provoked the verdict or attributes of a past event defined in the property.
	 * If the name starts with a  # , then the name refers to an embedded function
	 */
	char *if_satisfied, *if_not_satisfied;

	/**
	 * Allows indicating what event states should be kept.
	 * This is used when a property, for which the context becomes valid,
	 * should be able to detect all the triggers that occur in the defined time interval and not be  consumed  by the  first one.
	 */
	uint8_t *keep_state;

	/**
	 * Context
	 */
	rule_node_t *context;
	/**
	 * Consequence
	 */
	rule_node_t *trigger;

}rule_t;

/**
 * Read a list of rules from an xml property file
 * - Input
 * 	+ file_name: path of property file
 * - Output:
 * 	+ properties_arr    : array of pointers point to properties
 * 	+ embedded_functions: string representing C code that will be inserted into the generated code
 * - Return:
 * 	+ Number of properties being read
 * - Error:
 * 	+ Exist the system if there exist some syntax errors
 */
size_t read_rules_from_file( const char * file_name, rule_t *** properties_arr, char **embedded_functions);

void free_a_rule( rule_t *, bool free_data);
void rule_free_an_operator( rule_operator_t *, bool free_data);
void rule_free_an_event( rule_event_t *, bool free_data);


/**
 * Get unique events using in a rule. An event is identified by its id.
 *	- Input:
 *		+ rule
 *	- Output:
 *		+ create and assign a map containing unique events
 *	- Return:
 *		+ number of unique events
 */
size_t get_unique_events_of_rule( const rule_t *rule, mmt_map_t **events_map );

#endif /* SRC_LIB_RULE_H_ */
