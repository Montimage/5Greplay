
Given a property `if A then B`, 

- `FSM_ERROR_STATE_REACHED` if `A` is validated but not `B` (or we have not `B` in a given delay)

- `FSM_FINAL_STATE_REACHED` if both `A` and `B` are validated

```c
static inline enum verdict_type _get_verdict( int rule_type, enum fsm_handle_event_value result ){
   switch ( rule_type ) {
   case RULE_TYPE_TEST:
      switch( result ){
      case FSM_ERROR_STATE_REACHED:
         return VERDICT_NOT_RESPECTED;
      case FSM_FINAL_STATE_REACHED:
         return VERDICT_RESPECTED;
      default:
         return VERDICT_UNKNOWN;
      }
      break;

   case RULE_TYPE_SECURITY:
      switch( result ){
      case FSM_ERROR_STATE_REACHED:
         return VERDICT_NOT_RESPECTED;
      case FSM_FINAL_STATE_REACHED:
//       return VERDICT_RESPECTED;
         return VERDICT_UNKNOWN;
      default:
         return VERDICT_UNKNOWN;
      }
      break;
   case RULE_TYPE_ATTACK:
   case RULE_TYPE_EVASION:
      switch( result ){
      case FSM_ERROR_STATE_REACHED:
         return VERDICT_UNKNOWN; //VERDICT_NOT_DETECTED;
      case FSM_FINAL_STATE_REACHED:
         return VERDICT_DETECTED;
      default:
         return VERDICT_UNKNOWN;
      }
      break;
   default:
      ABORT("Error 22: Property type should be a security rule or an attack.\n");
   }//end of switch
   return VERDICT_UNKNOWN;
}
```