# 1. Introduction
Generally, MMT-Security takes as input a sequence of message and output alerts. It loads first a set of rules to be verified and be compiled as dynamic libraries. Each time a message coming, it verifies the message against the rules. If some rules are satisfied, it output alerts.

```
     rules.so ----> --------------
                   |              |
==== messages ===> | MMT-Security | ==== alerts ===>
                   |              |
                    --------------
```

# 2. Workflow
The workflow of MMT-Security multi-threading is represented as Figure below. Details of the functions will be introduced subsequently.
![Workflow](smp_workflow.svg)

[Edit](https://knsv.github.io/mermaid/live_editor/#/edit/c2VxdWVuY2VEaWFncmFtClVzZXItPj4gbW10X3NtcF9zZWN1cml0eTogbW10X3NtcF9zZWNfcmVnaXN0ZXIoKQphY3RpdmF0ZSBtbXRfc21wX3NlY3VyaXR5Cmxvb3AgZm9yIGVhY2ggdGhyZWFkCm1tdF9zbXBfc2VjdXJpdHktPj4gbW10X3NpbmdsZV9zZWN1cml0eTogbW10X3NpbmdsZV9zZWNfcmVnaXN0ZXIKYWN0aXZhdGUgbW10X3NpbmdsZV9zZWN1cml0eQptbXRfc2luZ2xlX3NlY3VyaXR5LT4-IHJ1bGVfdmVyaWZfZW5naW5lOiBydWxlX2VuZ2luZV9pbml0CmFjdGl2YXRlIHJ1bGVfdmVyaWZfZW5naW5lCmVuZAoKVXNlci0-PiBtbXRfc21wX3NlY3VyaXR5OiBtbXRfc21wX3NlY19wcm9jZXNzIChtZXNzYWdlKQptbXRfc21wX3NlY3VyaXR5IC0-PiBtbXRfc21wX3NlY3VyaXR5OiByaW5nX3B1c2goIG1lc3NhZ2UgKQoKbG9vcCBmb3JldmVyCm1tdF9zaW5nbGVfc2VjdXJpdHkgLT4-IG1tdF9zbXBfc2VjdXJpdHk6IHJpbmdfcG9wX2J1cnN0KCBtZXNzYWdlcyApCgpsb29wIGZvciBlYWNoIG0gaW4gbWVzc2FnZXMKbW10X3NtcF9zZWN1cml0eSAtPj4gbW10X3NpbmdsZV9zZWN1cml0eTogbW10X3NpbmdsZV9zZWNfcHJvY2VzcyggbSApCm1tdF9zaW5nbGVfc2VjdXJpdHkgLT4-IHJ1bGVfdmVyaWZfZW5naW5lOiBydWxlX2VuZ2luZV9wcm9jZXNzKCBtICkKcnVsZV92ZXJpZl9lbmdpbmUgLS0-PiBtbXRfc2luZ2xlX3NlY3VyaXR5OiB2ZXJkaWN0CgpvcHQgdmVyZGljdCAhPSB1bmtub3duCm1tdF9zaW5nbGVfc2VjdXJpdHkgLT4-IG1tdF9zaW5nbGVfc2VjdXJpdHk6IGNhbGxiYWNrKCB2ZXJkaWN0ICkKZW5kCgplbmQKZW5kCgpVc2VyLT4-IG1tdF9zbXBfc2VjdXJpdHk6IG1tdF9zbXBfc2VjX3VucmVnaXN0ZXIKbG9vcCBmb3IgZWFjaCB0aHJlYWQKbW10X3NtcF9zZWN1cml0eS0-PiBtbXRfc2luZ2xlX3NlY3VyaXR5OiBtbXRfc2luZ2xlX3NlY191bnJlZ2lzdGVyCm1tdF9zaW5nbGVfc2VjdXJpdHktPj4gcnVsZV92ZXJpZl9lbmdpbmU6IHJ1bGVfZW5naW5lX2ZyZWUKZGVhY3RpdmF0ZSBydWxlX3ZlcmlmX2VuZ2luZQptbXRfc2luZ2xlX3NlY3VyaXR5IC0tPj4gbW10X3NtcF9zZWN1cml0eTogYWxlcnRzX2NvdW50IG9mIGVhY2ggcnVsZQpkZWFjdGl2YXRlIG1tdF9zaW5nbGVfc2VjdXJpdHkKZW5kCgptbXRfc21wX3NlY3VyaXR5IC0tPj4gVXNlcjogYWxlcnRzX2NvdW50CmRlYWN0aXZhdGUgbW10X3NtcF9zZWN1cml0eQ)

## 2.1. Initialize

This process need to be done only once before calling any other functions.

### 2.2.1 Load rules

````C
int mmt_sec_init( const char * excluded_rules_id )
````

- **Description**:
	This function initialises MMT-Security to analyse a set of rules. By defaults, these rules must be compiled and located on the folder `./rules` or `/opt/mmt/security/rules`.

- **Input**:
	This function allows to exclude some rules from verification. The ID of excluded rules are set by the parameter `excluded_rules_id`. The parameter is a null-terminated byte string

	The IDs are separated by comma `,`. A continue range of IDs is represented by the smallest and the biggest ID and they are separated by a dash `-`.

	For example, `mmt_sec_init("1-5,7,9")`, will exclude rules having id 1,2,3,4,5,7 and 9 from verification.

- **Return**:
   - 0 if no error
   - 1 if MMT-Security found no rules to verify


### 2.2.2 Register

```C
// A function to be called when a rule is validated
typedef void (*mmt_sec_callback)(
		const rule_info_t *rule,         //rule being validated
		enum verdict_type verdict,       //DETECTED, NOT_RESPECTED
		uint64_t timestamp,              //moment (by time) the rule is validated
		uint64_t counter,                //moment (by order of packet) the rule is validated
		const mmt_array_t * const trace, //historic of messages that validates the rule
		void *user_data                  //#user-data being given in mmt_sec_register_rules
);
mmt_sec_handler_t* mmt_sec_register( size_t threads_count, const uint32_t *cores_id, 
                         const char *rules_mask, bool verbose, mmt_sec_callback callback, void *args );
```

- **Description**: Register some rules to validate.
  
  After calling this function, depending on number of threads given in `threads_count` parameter, MMT-Security can create several threads and starts them. The processing of the threads is represented by the loop `forever` block in Figure above. This block always check the MMT-Security buffer to get out a message to verify. Each time it found a rule being validated, it will call the function given by `callback` parameter. It is stoped by calling `mmt_sec_unregister` function.

- **Input**:
    + `threads_count`: number of threads
    + `core_mask`    : a string indicating logical cores to be used,
   						  e.g., "1-4,11-12,19" => we use cores 1,2,3,4,11,12,19. Core number starts from 1.
    + `rule_mask`    : a string indicating special rules being attributed to special threads
   						e.g., "(1:10-13)(2:50)(4:1007-1010)".
    						The other rules will be attributed equally to the rest of threads.
    + `callback`     : a function to be called when a rules is validated
    + `user_data`    : data will be passed to the `callback`
- **Return**:
    + `NULL` if there are error, e.g., insufficient memory, parameters are incorrect
    + a handler pointer, otherwise
- **Note**:
	The function callback can be called from different threads. (Thus if it accesses	to a global variable or a static one, the access to these variables must be synchronous)




## 2.2. Process

This function need to call every time having a message to be verified.

```C
void mmt_sec_process( mmt_sec_handler_t *handler, message_t *msg );
```

This function simply put the message into a buffer of MMT-Security. If the buffer is not full, the function return immediately, otherwise it will always verify the buffer until having an available place to put the message.

## 2.3. Finish

This process need to be done once only when finishing. 

### 2.3.1 Unrgister
```C
size_t mmt_sec_unregister( mmt_sec_handler_t* );
```
 - **Description**: Stop and free every threads used by MMT-Security.
 - **Input**: a pointer points to the handler being created by `mmt_sec_register`
 - **Return**: Number of alerts generated

### 2.3.2 Unload rules

The following function will unload rules. This is called only at the end of processing. After this function, no more mmt-security functions can be called.

```C
void mmt_sec_close();
```