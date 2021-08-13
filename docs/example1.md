# Example 1: Filtering and replaying NGAP/NAS-5G Security Mode Complete messages

This document aims to show the main functionalities of 5Greplay through an example. For more details please use the 5Greplay [User Manual](./5Greplay_Manual.pdf) as a reference.

In general, an use case of 5Greplay It will be composed of 3 main steps:
1. Design a rule that filter the desired packets to be replayed
2. Configure 5Greplay the through its configuration [file](../../mmt-5greplay.conf), to determine which rules will be appliyed and where the traffic will be sent
3. Observe the replayed traffic, and the 5Greplay logs

<img width="499" alt="image" src="https://user-images.githubusercontent.com/45805561/129385771-d266f04c-0011-4685-9bb6-3ddb933550aa.png">


## 1. Design a 5Greplay filtering rule 

The first thing you need to define is **what packets you desire to forward, drop or modify**. For that you must make a 5Greplay XML rule, that normally will be located in ```5GReplay/rules/```.

In  Section 4 of the [User Manual](./5Greplay_Manual.pdf) we explain in detail all the attributes of 5Greplay rules. To have reference of the name of the protocol attributes that 5Greplay uses, and that you can use to write you rules (e.g. ```nas_5g.message_type, nas_5g.security_type```),please refers to the [list of attributes](./5Greplay_attributes.txt).

For this example we will be using the [property 90](../rules/nas-smc-replay-attack.xml) located in ```5GReplay/rules/nas-smc-replay-attack.xml```:

```xml
<beginning>
<!-- Property 90: Forwarding NAS security mode COMPLETE that answers to NAS security mode COMMAND.-->
<property value="THEN"  delay_units="ms" delay_min="0" delay_max="10" property_id="90" type_property="FORWARD" 
    description="Forwarding NAS security mode COMPLETE that answers to NAS security mode COMMAND " >
    <event value="COMPUTE" event_id="1" 
           description="NAS Security mode COMMAND"
           boolean_expression="(nas_5g.message_type == 93)"/>
    <event value="COMPUTE" event_id="2" 
           description="NAS Security mode COMPLETE"
           boolean_expression="(nas_5g.security_type == 4)"/>
</property>
</beginning>
```

- The property is compose for **2 events**, that must occur with a minimun delay of 0 ms and a maximun delay of 10 ms, as stablished in: 

```xml
<property value="THEN"  delay_units="ms" delay_min="0" delay_max="10" property_id="90" type_property="FORWARD" 
```

- The **event 1** identifies a _NAS Security mode Command_ message, by its NAS message type, that must be equal to 93:

```xml
    <event value="COMPUTE" event_id="1" 
           description="NAS Security mode COMMAND"
           boolean_expression="(nas_5g.message_type == 93)"/>
```


- The **event 2** identifies a _NAS Security mode Complete_ message that is expected to be recieve after the message described in the event 1. This message is identified by having a NAS security type equal to 4:

```xml
    <event value="COMPUTE" event_id="2" 
           description="NAS Security mode COMPLETE"
           boolean_expression="(nas_5g.security_type == 4)"/>
```

With the lines before we have determined that we want to filter the _NAS Security mode Complete_ messages. By default 5Greplay will forward them into the destination that we will define in the configuration file. However, in this step we can also what action should be performed if the property is satisfied, by adding the field ``if_satisfied`` in the property line:

```xml
<property value="THEN"  delay_units="ms" delay_min="0" delay_max="10" property_id="90" type_property="FORWARD"  description="Increase RAN IDs of packets with RAN ID < 10" if_satisfied="#update(ngap.ran_ue_id , .2+100))">
```

After have written your rule, you can compile it, by doing:

```bash
#to generate .so file
./mmt-5greplay compile rules/forward-localhost.so rules/forward-localhost.xml
 
#to generate code c (for debug)
./mmt-5greplay compile rules/forward-localhost.c rules/forward-localhost.xml -c

```

Or you can compile all rules existing in the folder `rules`, use the following command: `make sample-rules`.

## 2. Configure 5Greplay

After having written the rules and verifying that they were not syntax errors during compilation. You must indicate to 5Greplay **where to forward the traffic and what to do with the packets that did not fulfill the properties** that you defined in your rules.

## 3. Observe
