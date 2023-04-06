/*
 * mmt_fsm.h
 *
 *  Created on: 3 oct. 2016
 *  Created by: Huu Nghia NGUYEN <huunghia.nguyen@montimage.com>
 *
 * This is a simple finite fsm_state_struct machine implementation.
 * It supports grouped states, guarded transitions, events
 * with payload, entry and exit actions, transition actions and access to
 * user-defined state data from all actions.
 *
 * The user must build the state machine by linking together states and
 * transitions arrays with pointers.
 * A pointer to an initial state and an error state are given to fsm_init()
 * to initialize a machine object.
 *
 * The machine is run by passing events to it with the function fsm_handle_event().
 * The return value of the function will give an indication to what has happened.
 * https://github.com/misje/stateMachine
 */

#ifndef SRC_LIB_FSM_H_
#define SRC_LIB_FSM_H_


#include "../lib/mmt_lib.h"
#include "rule.h"
#include "message_t.h"

typedef struct fsm_delay_struct{
	/**
	 * Defines the validity period ([time_min, time_max]) of the left branch (e.g. context).
	 * Unit: micro-second.
	 * default is 0,
	 * - if value is < 0 then event needs to be satisfied before,
	 * - if = 0 then in same packet,
	 * - if > 0 then after
	 */
	uint64_t time_min, time_max;
	int time_min_sign, time_max_sign;
	/**
	 * Similar to [time_min, time_max] we can de ne [counter_min, counter_max] where the unit is the number of packets analysed.
	 * note that either delay or counter needs to be used not both
	 */
	uint64_t counter_min, counter_max;
	int counter_min_sign, counter_max_sign;
}fsm_delay_t;

typedef struct fsm_struct fsm_t;

#define FSM_EVENT_TYPE_TIMEOUT 0

enum fsm_action_type {
	FSM_ACTION_DO_NOTHING      = 0,
	FSM_ACTION_CREATE_INSTANCE = 1,
	FSM_ACTION_RESET_TIMER     = 2
};

/**
 * Events trigger transitions from a state to another.
 * Event types are defined by the user.
 * Any event may optionally contain a user-defined date.
 *
 */
typedef struct fsm_event_struct{
   /** Type of event. Defined by user. */
   uint16_t type;
   /**
    * Event payload.
    *
    * How this is used is entirely up to the user.
    * This data is always passed together with #type in order to make it possible
    * to always cast the data correctly.
    */
   void *data;
}fsm_event_t;

/* pre-defined state that will be detailed later */
struct fsm_state_struct;

/**
 * Outgoing transition from a state to another state.
 *
 * All states that are not final must have at least one transition.
 * The transition may be guarded or not.
 * Transitions are triggered by events.
 * If a state has more than one transition with the same type of event (and the
 * same condition), the first transition in the array will be run.
 */
typedef struct fsm_transition_struct
{
   /**  The event that will trigger this transition. */
   uint16_t event_type;
   /**
    *  Check if data passed with event fulfills a condition.
    *
    * A transition may be conditional. If so, this function, if non-NULL, will
    * be called.
    *
    * - Input:
    * 	+ event: the event passed to the fsm_state_struct machine.
    *		+ fsm: the fsm containing this transition
    * - Return
    * 	+ YES if the event's data fulfills the condition, otherwise NO.
    */
   int ( *guard )( const message_t *msg, const fsm_t *fsm );

   /**
    * When the #guard is fulfilled, some pre-defined action will be performed.
    * For instant, the #action can be one of the following:
    * + FSM_ACTION_DO_NOTHING : nothing to do
    * + FSM_ACTION_CREATE_INSTANCE: create an instance from the current fsm (that is also an instance).
    *    The current fsm will not change. The instance is created by cloning the fsm and updating
    *    its current state to the target state #target_state of this transition.
    */
   int action;
   /**
    *  The next state
    *
    * This must point to the next state that will be entered. It cannot be NULL.
    * If it is, the machine will detect it and enter the #error_state.
    */
   struct fsm_state_struct *target_state;
}fsm_transition_t;

/**
 *  States of machine
 *
 * The current state in a machine moves to a new state when one of the
 * #transitions in the current state triggers on an event.
 * An optional #exit_action is called when the state is left,
 * 	and an #entry_action is called when the machine enters a new state.
 * If a state returns to itself, neither #exit_action nor #entry_action
 * will be called.
 *
 */
typedef struct fsm_state_struct{
	const fsm_delay_t delay;

	/* delay = 0*/
	bool is_temporary;

	char *description;

   /**
    *  An array of outgoing transitions of the state.
    */
   const struct fsm_transition_struct *transitions;
   /**
    *  Number of outgoing transitions in the #transitions array above.
    */
   size_t transitions_count;
   /**
    *  This function is called whenever the state is being entered. May be NULL.
    */
   int entry_action;
   /**
    *  This function is called whenever the state is being left. May be NULL.
    */
   int exit_action;
}fsm_state_t;


/**
 *  Finite State Machine
 */
struct fsm_struct{
	uint64_t time_min, time_max;
//	uint64_t counter_min, counter_max;

	//id of the FSM
	uint16_t id;

	/** ID of event to be verified */
	uint16_t current_event_id;

   /**  Pointer to the current fsm_state_struct */
   const fsm_state_t *current_state;

   const fsm_state_t *init_state;

   const fsm_state_t *error_state;

   const fsm_state_t *incl_state;

   const fsm_state_t *success_state;

   mmt_array_t *execution_trace;

   //this is for internal usage. It points to _rule_engine_t
   void *user_data;
} __aligned;

/**
 *  Initialize the machine
 *
 * This function creates and initializes the machine. No actions are performed until
 * fsm_handle_event() is called.
 *
 * - Note:
 * 	 The #entry_action for #init_state will not be called.
 *
 * - Input:
 * 	+ init_state the initial state of the machine.
 * 	+ error_state pointer to a state that acts a final state and notifies
 * 		the system/user that an error has occurred.
 * 	+ final_state
 * 	+ incl_state
 */
fsm_t *fsm_init( const fsm_state_t *init_state, const fsm_state_t *error_state, const fsm_state_t *final, const fsm_state_t *incl_state, size_t events_count, size_t message_size );

/**
 * Reset the machine as being created.
 *
 * It is safe to call this function numerous
 * times, for instance in order to reset/restart the machine if a final
 * state has been reached.
 *
 * - Input:
 * 	+ fsm: the machine to be reseted
 */
void fsm_reset( fsm_t *fsm );

/**
 *  fsm_handle_event() return values
 */
enum fsm_handle_event_value{
   /**  Erroneous arguments were passed */
	FSM_ERR_ARG = -2,
   /**
    *  The error state was reached
    *
    * This value is returned either when the machine enters the error state
    *  itself as a result of an error, or when the error state is the
    *  target state as a result of a successful transition.
    *
    * The machine enters the state machine if any of the following
    * happens:
    * - The current state is NULL
    * - A transition for the current event did not define the target state
    */
	FSM_ERROR_STATE_REACHED,
   /**  The current state changed into a non-final state */
	FSM_STATE_CHANGED,
   /**
    *  The state changed back to itself
    *
    * The state can return to itself either directly or indirectly.
    * An indirect path may include a transition from a parent state and the use of #entry_state
    */
	FSM_STATE_LOOP_SELF,
   /**
    *  The current state did not change on the given event
    *
    * If any event passed to the machine should result in a state change,
    * 	this return value should be considered as an error.
    */
	FSM_NO_STATE_CHANGE,
   /**  A final state was reached */
	FSM_FINAL_STATE_REACHED,

	/** current_state of #fsm is #incl_state */
	FSM_INCONCLUSIVE_STATE_REACHED,
};

/**
 *  Pass an event to the machine
 *
 * The event will be passed to the current state.
 * If the event triggers a transition, a new state will be entered.
 * If the transition has an action defined, it will be called.
 * If the transition is to a state other
 * than the current state, the current state's exit_action
 * is called (if defined). Likewise, if the state is a new
 * state, the new state's "entry action" is called (if defined).
 *
 * - Input:
 * 	+ fsm: the state machine to pass an event to.
 * 	+ transition_index: indicates the transition T at index #transition_index of
 * 		the current state of #fsm will be verified to fire.
 * 	+ message_data: data to be stored in the execution trace of the machine if T fired.
 * - Output:
 * 	+ new_fsm: points to a new instance of #fsm if the #action of the transition T
 * 		is #FSM_ACTION_CREATE_INSTANCE and the transition has been fired, otherwise
 * 		#new_fsm = NULL
 *	- Return:
 * 	+ fsm_handle_event_value
 *
 * - Note:
 * 	From the current state of #fsm, any transition having type == #FSM_EVENT_TYPE_TIMEOUT will
 * 	be verified firstly, before the transition having index == #transition_index. Thus if one of them
 * 	can fire, the machine will fire that transition, not the one having index == #transition_index
 */
enum fsm_handle_event_value fsm_handle_event( fsm_t *fsm, uint16_t transition_index, message_t *message, fsm_t **new_fsm );


bool fsm_is_verifying_single_packet( const fsm_t *fsm );
/**
 * This is specific function for the rules that verify only one a single packet
 * @param fsm
 * @param message_data
 * @param event_data
 * @return FSM_ERR_ARG if this fsm verifies on multiple packets
 */
enum fsm_handle_event_value fsm_handle_single_packet( fsm_t *fsm, message_t *message );
/**
 *  Get the current state
 *
 *	- Input:
 * 	+ fsm_struct the state machine to get the current state from.
 *	- Return:
 *		+ a pointer to the current state, otherwise, NULL if fsm is NULL.
 */
static inline const fsm_state_t *fsm_get_current_state( const fsm_t *fsm) {
	__check_null( fsm, NULL );

	return fsm->current_state;
}

/**
 *  Check if the state machine has stopped
 *
 * - Input:
 *		+ the state machine to test.
 *	- Return:
 *		+ true if the state machine is at a state having no outgoing transition,
 *			otherwise, false
 */
static inline bool fsm_is_stopped( const fsm_t *fsm) {
	__check_null( fsm, YES );
	return (fsm->current_state->transitions_count == 0);
}

/**
 * Free the machine created by #fsm_init function
 *
 * - Input:
 *		+ the state machine to free.
 */
void fsm_free( fsm_t *fsm );

/**
 * Clone the machine and its current state
 *
 * - Input:
 *		+ the state machine to clone.
 */
fsm_t * fsm_clone( const fsm_t *fsm );

/**
 * Get the current execution trace of the machine
 *
 * - Input:
 *		+ the state machine to get.
 *	- Return:
 *		+ a map of events and its data. Each element of the map is indexed by event_id
 *		  of the machine, and its data has type #message_t that validates the event.
 */
static inline const mmt_array_t* fsm_get_execution_trace( const fsm_t *fsm ){
#ifdef DEBUG_MODE
	__check_null( fsm, NULL );
#endif
	return( fsm->execution_trace );
}


/**
 * Public API
 */
static inline const message_t *fsm_get_history( const fsm_t *fsm, uint32_t event_id ){
#ifdef DEBUG_MODE
	__check_null( fsm, NULL );
#endif
	if( unlikely( event_id >= fsm->execution_trace->elements_count )){
		ABORT("Access outside of array");
	}

	return fsm->execution_trace->data[ event_id ];
}

/**
 * Public API
 */
static inline uint16_t fsm_get_id( const fsm_t *fsm ){
#ifdef DEBUG_MODE
	__check_null( fsm, -1 );
#endif
	return fsm->id;
}

/**
 * Public API
 */
static inline void fsm_set_id( fsm_t *fsm, uint16_t id ){
#ifdef DEBUG_MODE
	__check_null( fsm,  );
#endif
	fsm->id = id;
}
/**
 * Free a #fsm_event_t object
 */
static inline void fsm_free_event( fsm_event_t *event, bool free_data ){
	if( event == NULL ) return;
	if( free_data == YES )
		mmt_free_and_assign_to_null( event->data );
	mmt_mem_free( event );
}


static inline void fsm_set_user_data( fsm_t *fsm, void *data){
#ifdef DEBUG_MODE
	__check_null( fsm,  );
#endif
	fsm->user_data = data;
}

static inline void * fsm_get_user_data( const fsm_t *fsm ){
#ifdef DEBUG_MODE
	__check_null( fsm, NULL );
#endif
	return fsm->user_data;
}

#endif /* SRC_LIB_FSM_H_ */
