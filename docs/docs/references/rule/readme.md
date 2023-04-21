# 1. Sample rules

There are sample rules in [`rules`](https://github.com/Montimage/5GReplay/tree/main/rules) folder of source code.


# 2. Embedded functions

Embedded functions are functions that allow implementing calculations
that are too complicated to define using only classical operators on fields in the Boolean expressions of security rules.
One can either use existing embedded functions or implement a new function.
In both cases, they can be used in the Boolean expressions by using the syntax:

  `#<name_of_function>(<list of parameters>)`

For example: `(#em_is_search_engine( http.user_agent ) == true)`

where `http` is the protocol name and `user_agent` is the attribute name (i.e., packet meta-data).

- Please refer [here](../protocols-attributes-list) to get the full list of protocols and attributes.

- Protocol and attribute names used in xml rules are **case-sensitive**. They are normally in lower case.

- To **avoid any confusion**, a new embedded function name should start by a prefix `em_`.

## 2.1. Special terms

1. `true` will be replaced by the number 1.

   For example: `#em_check( tcp.src_port ) == true`

2. `false` will be replaced by the number 0.

   For example: `#em_check( tcp.src_port ) == false`


## 2.2 Implement a new embedded function
In each rule file, there exists a section allowing user to add an embedded function.

```xml
<embedded_functions><![CDATA[
//code C
static inline bool em_check( double port ){
   if( port == 80 || port == 8080 )
      return true;
   return false;
}
]]></embedded_functions>
```

In side this tag, one can also implement 2 other functions as the followings:

1. `void on_load(){ ... }` being called when the rules inside the xml file being loaded into 5Greplay

2. `void on_unload(){ ... }` being called when exiting 5Greplay


## 2.3 Pre-installed embedded functions

In boolean expressions of rules, one can use one or many embedded functions

1. `is_exist( proto.att )`  checks whether an event has an attribute of a protocol, e.g., `is_exist( http.method )` will return `true` if the current event contains protocol `HTTP` and attribute method has a non-null value, otherwise it will return `false`.

	Normally 5Greplay has a filter that allow an event in a rule to be verified only if any proto.att used in its boolean expression contains value. If one of them has not, the rule will not be verified. This allows to reduce number of verification of boolean expression, thus increases the performance.

	For example, given an event having the following boolean expression:

	`((ip.src != ip.dst) && (#em_check_URI(http.uri) == 1))`

	This event is verified only if `ip.src` and `ip.dst` and `http.uri` are not null, hence only HTTP packets are verified (it does not verify every IP packets).

	However, if one use the following expression, that is totally having the same meaning with the previous one:

	`((ip.src != ip.dst) && ((#is_exist(http.uri) == true ) && (#em_check_URI(http.uri) == 1)))`

	5Greplay needs to verify the expression against any IP packet as `is_exist` tell 5Greplay to exclude `http.uri` from its filter.

2. `is_null( proto.att )`, e.g., `#is_null(http.uri)` check whether the value of `http.uri` is null

3. `is_empty( proto.att )`, e.g., `#is_empty(http.uri)` checks whether the value is null or the string value is empty, i.e., its length is zero.

4. `is_same_ipv4(const uint32_t*, const char *)`, e.g., `#is_same_ipv4(ip.src, "10.0.0.1")` checks whether the value of `ip.src` is `"10.0.0.1"`.

5. We can use any standard C functions as embedded function, e.g., `(#strstr( http.user_agent, 'robot') != 0)` to check if `http.user_agent` contains a sub-string `"robot"`.

   Please note that, before using a C function the library containing that embedded functions need to be included.
   The following libraries have been pre-included:

```c
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mmt_lib.h"
#include "pre_embedded_functions.h"
```

Consequently when using a function that does not defined inside these libraries, we need to include its library.
For example:

```xml
<embedded_functions><![CDATA[
#include <math.h>
static inline bool function em_check( double port ){
    double x = sqrt( port );
    return x >= 5;
}
]]></embedded_functions>
```

# 3. Reactive functions

Reactive functions allow us to perform some action when a rule is satisfied.
The functions will be called each time their rules are satisfied.

To implement and use a reactive function, we need:

- implement a C function inside `<embedded_functions>` tag.

   The function name should be prefixed by `em_` to avoid any confusion with the ones existing internally in 5Greplay.

   The function has the following format:

```c
typedef void (*mmt_rule_satisfied_callback)(
		const rule_t *rule,		          //rule being validated
		int verdict,                     //DETECTED, NOT_RESPECTED
		uint64_t timestamp,              //moment (by time) the rule is validated
		uint64_t counter,                //moment (by order of message) the rule is validated
		const mmt_array_t * const trace  //historic of messages that validates the rule
);
```

- put the function name in attribute `if_satisfied` of the rule you want to react. For example: `if_satisfied="em_print_out"`


```xml
<beginning>
<embedded_functions><![CDATA[
static void em_print_out(
      const rule_info_t *rule, int verdict, uint64_t timestamp,
      uint64_t counter, const mmt_array_t * const trace ){
   const char* trace_str = mmt_convert_execution_trace_to_json_string( trace, rule );
   printf( "detect rule %d\n%s\n", rule->id, trace_str );
   //you can call a system command, for example:
   //char command[1001];
   //snprintf( command, 1000, "echo rule %d is validated by %s", rule->id, trace_str );
   //system( command );
}
]]></embedded_functions>

<!-- Property 10: HTTP using a port different from 80 and 8080.-->
<property value="THEN" delay_units="s" delay_min="0" delay_max="0" property_id="10" type_property="EVASION"
    description="HTTP using a port different from 80 and 8080." if_satisfied="em_print_out">
    <event value="COMPUTE" event_id="1"
        description="HTTP packet using a port different from 80 and 8080"
           boolean_expression="((http.method != '')&amp;&amp;((tcp.dest_port != 80)&amp;&amp;(tcp.dest_port != 8080)))"/>
    <event value="COMPUTE" event_id="2"
           description="HTTP packet"
           boolean_expression="(ip.src != ip.dst)"/>
</property>
</beginning>
```

## 3.1 Supported functions

We implement several functions to suport modifying protocol attributes' values, drop a packet, and forward a packet or it copies to the output NIC.
New functions will be implemented and add time by time.

The following functions **must** be called inside a reactive function.

1. `get_numeric_value( proto_id, att_id, event_id, trace)` returns a `uint64_t` value of `proto_id.att_id` field of `event_id` from the satisfied trace.

2. `set_numeric_value( proto_id, att_id, uin64_t )` marks to change `proto_id.att_id` to a `uint64_t` value. The change will be applied only at the moment of sending the packet, e.g., by explicitly call `forward_packet`.

3. `forward_packet()` forwards immediatly the current packet

4. `drop_packet()` does not forward the current packet

5. `update_sctp_param( uint32_t ppid, uint32_t flags, uint16_t stream_no, uint32_t timetolive )` updates parameters in `sctp_sendmsg` function that is used to create a SCTP communication channel between 5Grepay and its target. See example in [rules/11.modify-sctp-params.xml](../../../rules/11.modify-sctp-params.xml)

## 3.2 Predefined reactive functions

The following functions have been already defined

1. `#drop()` to drop the current packet

2. `#update( proto_id.att_id, expression )` to alter the current packet then forward it to the outgoing NIC.

- `proto_id` and `att_id` epresent respectively protocol and attribute to be altered
- `expression` is in format of `expression` to get value to assign to the protocol attribute
- Example:
   + `#update(ngap.ran_ue_id, (ngap.ran_ue_id + 100)` will increase `ran_ue_id` by 100
   + `#update(ngap.ran_ue_id, (ngap.ran_ue_id.1 * 2)` will replace `ran_ue_id` of the current packet
   by the double of `ran_ue_id` of the packet having `event_id=1`

3. `#fuzz(proto.att, proto.att, ...)` to alter the current packet then forward its copies to the outgoing NIC.

- Currently (Feb 10, 2022) only numeric values are supported
- Each proto.att will be set a new random value generated by C `random` function.
 See an example in [rules/7.fuzz-ngap.xml](../../../rules/7.fuzz-ngap.xml) on how to override the `random` function.

## 3.3 Examples

The following XML file defines 3 rules:


```xml
<beginning>
<embedded_functions><![CDATA[

static void em_modif_then_forward(
      const rule_info_t *rule, int verdict, uint64_t timestamp,
      uint64_t counter, const mmt_array_t * const trace ){
   const char* trace_str = mmt_convert_execution_trace_to_json_string( trace, rule );
   //forward the original packet (without any modification)
   forward_packet();
   //get old value ran_ue_id
   uint64_t ran_ue_id = get_numeric_value( PROTO_NGAP, NGAP_ATT_RAN_UE_ID, 2, trace );

   //clone 9 times the current packet
   for( int i=1; i<10; i++ ){
      //increase ran_ue_id
      set_numeric_value( PROTO_NGAP, NGAP_ATT_RAN_UE_ID, ran_ue_id + i );
      //forward the packet having the modified ran_ue_id
      forward_packet();
   }
}
]]></embedded_functions>

<property value="THEN"  delay_units="s" delay_min="0+" delay_max="1" property_id="100"
    description="Forwarding NAS security mode COMPLETE that answers to NAS security mode COMMAND "
    if_satisfied="em_modif_then_forward">
    <event value="COMPUTE" event_id="1" description="NAS Security mode COMMAND"
           boolean_expression="(nas_5g.message_type == 93)"/>
    <event value="COMPUTE" event_id="2" description="NAS Security mode COMPLETE"
           boolean_expression="(nas_5g.security_type == 4)"/>
</property>

<property property_id="101" if_satisfied="#drop()"
    description="Do not forward any UDP packets having dest_port other than 2152">
    <event event_id="1" description="From UE and GTP packets"
           boolean_expression="( (udp.dest_port != 2152 ) )"/>
</property>

<property property_id="102" description="" if_satisfied="#update(ngap.procedure_code, (ngap.procedure_code.1 + 100))">
    <event event_id="1" description="having RAN UE ID"
           boolean_expression="(ngap.ran_ue_id != 0)"/>
</property>
```

1. Rule 100 will match 2 different packets (as it has 2 events and `delay_min="0+"`):

   - the first packet is NAS security mode COMMAND (having `nas_5g.message_type == 93`)
   - the second packet is NAS security mode COMPLETE (having `nas_5g.message_type == 4`)

At the moment of getting the second packet, the `em_modif_then_forward` function is called.
In this function we can modify the second packet's content then inject it into the outgoing network.
The function fistly forwards the second packet without any modification by calling `forward_packet()`.
It then get the current value of `NGAP_ATT_RAN_UE` attribute of `NGAP` protocol in the second packet by calling `get_numeric_value`.
It then marks the incrasing of this value, by calling `set_numeric_value`, then forward the packet within this change into the network.

   *Note*: if the function `set_numeric_value` is called twice to modify the same attribute of a protocol, then only the second call is performed.

2. Rule 101 does not forward the UDP packets that have `dest_port` other than 2152 by explicitly call `drop()` function.

3. Rule 102 checks whether a packet contains not-zero `ran_ue_id` of `ngap`. If so, it increases the `procedure_code` value by 100.
(and then, by default, it forwards the modified packet to the outgoing NIC)

# 4. Compile rules

5Greplay rules are specified in plain text following a XML format.
These rules need to be encoded in a suitable format, that is a dynamic C library, before being able to use by 5Greplay.

The compiled rules must put in `./rules` folder.

## 4.1 Compile rules

5Greplay provides a compiler to do the such of task. To do so, we need to install `gcc` beforehand, e.g., `sudo apt install gcc`.

- use `compile` to compile rules in a XML file. For example:

   `./5greplay compile rules/1.so rules/1.xml`

   The program uses 3 parameters in form: `output_file property_file [gcc parameters]`

   where:

   + `output_file`: is the path of file containing the result that can be either a .c file or .so file.
   + `property_file`: is the path where the property file can be found.
   + `options`:

      - `-c`: will generate only the C code. This option allows manually modifying the generated code before compiling it.
       After generating the C code, the tool prints out the command that needs to be executed for compiling it.

       - gcc parameters: used to generate the C code, and compile it to obtain the .so file.
                      These parameters will be directly transmitted to the gcc compiler, for example, `"-I /tmp -lmath"`

## 4.2 Obtain information inside compiled rules

To get some basic information about a compiled rule (such as, ID, description), we use `info` command.

   By default, the tool will print out the information on all rules, for example:

```bash
./5greplay info
mmt-5greplay: 5Greplay v0.0.1-5c9a333 using DPI v1.7.0.0 (a8ad3c2) is running on pid 24680
mmt-5greplay: Ignore duplicated rule id 103 (Inject only packet from UE -> Core but not inversed direction)
Found 3 rules.
1 - Rule id: 103
	- type            : DROP
	- events_count    : 2
	- variables_count : 2
	- variables       : sctp.ch_type (304.5), sctp.dest_port (304.2)
	- description     : Inject only SCTP packets from UE -> Core but not inversed direction
	- if_satisfied    : 0x7f84c2d95db5
	- version         : 0.0.1 (5c9a333 - 2021-9-9 11:36:44), dpi version 1.7.0.0 (a8ad3c2)
...
```

The tool can also be used to inspect a specific compiled rule by giving the rule path as parameter, for example:

```bash
./5greplay info rules/forward-localhost.so
mmt-5greplay: 5Greplay v0.0.1-5c9a333 using DPI v1.7.0.0 (a8ad3c2) is running on pid 21983
Found 2 rules.
1 - Rule id: 103
	- type            : DROP
	- events_count    : 2
	- variables_count : 2
	- variables       : sctp.ch_type (304.5), sctp.dest_port (304.2)
	- description     : Inject only SCTP packets from UE -> Core but not inversed direction
	- if_satisfied    : 0x7f2925125db5
	- version         : 0.0.1 (5c9a333 - 2021-9-9 11:36:44), dpi version 1.7.0.0 (a8ad3c2)
...
```

# 5. Default values

In the XML file of a rule, if an attribute of an XML tag is absent then its value is set by default:

- `property`, `operator`:
   + `type_property`: `FORWARD`
   + `value` : `COMPUTE` (=> only one `<event>` is required. Thus `delay_min` and `delay_max` must be 0)
   + `delay_units`: `s`
   + `delay_min`: 0
   + `delay_max`: 0

# 6. Write a high performance rule

 To write a rule having a high performance one need to:

 - use only the proto.att in boolean expression when need.

   - Please refer to the usage of function `is_exist` to get an example.
   - Use explicitly the following tcp flags to filter out unwanted verification:
   `tcp.fin`, `tcp.syn`, `tcp.rst`, `tcp.psh`, `tcp.ack`, `tcp.urg`, `tcp.ece`, `tcp.cwr`.

   For example, the 2 following boolean expressions have the same meaning:

   `(tcp.flags == 4)` and `((tcp.flags == 4) && (tcp.rst == 1))`

   They both return `true` when only RST flag of a TCP packet is on, but the latter is better as 5Greplay verifies its rule only when `tcp.flags` and `tcp.rst` are not zero. Usually less than about 1% packets having `tcp.rst != 0`, consequently the rule using the second expression will be verified against only 1% packets.

 - reduce `delay_max` of a rule to a suitable value.

   When having a higher value of `delay_max` 5Greplay creates more rule instances to correlate different events of different packets.
   When `delay_max` is zero, we call *simple rule*, 5Greplay verifies the rule and gives verdict immediately without creating any rule instances.
   A simple rule is verified much faster than a complex one that has non-zero `delay_max`.

   At 10Gbps, 5Greplay can verify upto 12400 simple rules or 600 complex rules.

 - optimize implementation of embedded functions.

	 - The embedded functions are called each time their boolean expressions are verified. Consequently, rather than initialize something, for example, connection to database, inside these functions, one can do such a task, only once, inside function `on_load` then store the connection into a static local variable that will be used inside the embedded functions.

	 - alway use embedded function with `static inline` keyword. For more information about advantage of `inline`, please refer to document of gcc: [An Inline Function is As Fast As a Macro](https://gcc.gnu.org/onlinedocs/gcc/Inline.html)

# Links

- [Rule XML semantics](../rule-xml-semantics)
- [Replay Example](../../tutorial/replay-open5gs)
