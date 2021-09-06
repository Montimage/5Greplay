
This document explains how we can check quickly a rule that can be verified when we know a message `msg`.
We know that a rule contains a set of events. Each event contains a set of proto.atts.
A rule is verified when we have a message that contains at least one event.

# Problem statement

Let examine the following rule:

```xml
<property value="THEN" delay_units="s" delay_min="0+" delay_max="6" property_id="3" type_property="ATTACK" 
    description="TCP SYN requests on microsoft-ds port 445 with SYN ACK.">
    <event value="COMPUTE" event_id="1"
           description="SYN request"
           boolean_expression="((tcp.flags == 2)&amp;&amp;(tcp.dest_port == 445))"/>
    <event value="COMPUTE" event_id="2" 
           description="SYN ACK reply"
           boolean_expression="((tcp.flags == 18)&amp;&amp;(ip.src == ip.dst.1))"/>
</property>
```

This rule has 2 events. 
The first event, having `event_id=1`, requires the present of `tcp.flags`, `tcp.dest_port`  and `ip.dst` (it will be used in event 2).
The second one requires `tcp.flags`, `ip.src`.
This rule is verified against a message `msg` if the message contains:

- either `set_1` = ( `tcp.flags`, `tcp.dest_port`, `ip.dst`)
- or `set_2` = (`tcp.flags`, `ip.src`)
- or `set_3` = `set_1` v `set_2`

Thus, given a message `msg` having a set of any proto.atts. We need to find the fastest way to check if the message containing one of the 3 sets above.

# Solution

Using bit hash to check the present of each proto.att in message. Specifically,

- Each proto.att is represented by a unique id starting from 0. 
    
  This assignment is done when calling `mmt_sec_init` function. At that moment, MMT-5Greplay knows the set of rules to verify, thus the set of proto.atts presented in these rules. The set of proto.atts is sorted by ascending of ID of protocols and attributes, then indexed from 0 to the last proto.atts. The ID numbers are designed by MMT-DPI.
  
  
  For example, the ids of the proto.att above are as the following:
    
  |   0     |    1     |       2         |     3       |
  | -------:| --------:| ---------------:| -----------:|
  |`ip.src` | `ip.dst` | `tcp.dest_port` | `tcp.flags` |
    
- A `unint_64 hash` number is used to present a set of proto.atts by turning on the corresponding bit of each proto.att.

  Each message `msg` has a hash number, `msg->hash`, representing its set of proto.atts. The hash number is updated each time a new proto.att is added into `msg` by the function `set_element_data_message_t`.
  
  For example, to represent the presence of `tcp.dest_port` and `tcp.flags` of message `msg`, we have `msg->hash = (2^2 + 2^3) = 12`
   
  Each rule `r` has an array of hash numbers, `r->events_hash`, represent its requirement set of proto.atts.
  This is calculated only once, when initializing MMT-5Greplay, by the function `_calculate_hash_number`.
  
  For example, to represent the requirement `set_1` of event 1, we have `r->event_hash[1] = (2^1 + 2^2 + 2^3) = 14`
   
  The message `m` can be used to verify the event 1 if `(msg->hash & r->events_hash[1]) == r->events_hash[1]`


- A rule has a hash number representing its requirement of proto.atts. 
  This number is calculated only once, when initializing MMT-5Greplay, inside the function `mmt_single_sec_register`.
  
  For example, `hash = (r->event_hash[1] | r->event_hash[2]) = (14 | 9) = 15` 
  
  A message `msg` can be verified against a rule `r` if `msg` contains at least one proto.att required by `r`. Consequently, the condition is `(hash & msg->hash != 0)`.
  

## Limit
 As `uint64_t` is used to represent hash of a message, a message can contain a set of maximal 64 proto.atts.
  