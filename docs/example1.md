# Example 1: Filtering and replaying NGAP Security Mode Complete messages

This document aims to show the main functionalities of 5Greplay through an example. For more details please use the 5Greplay [User Manual](./5Greplay_Manual.pdf) as a reference.

In general, an use case of 5Greplay It will be composed of 3 main steps:
1. Design a rule that filter the desired packets to be replayed
2. Configure 5Greplay the through its configuration [file](../../mmt-5greplay.conf), to determine which rules will be appliyed and where the traffic will be sent
3. Observe the replayed traffic, and the 5Greplay logs

[arch.pdf](https://github.com/Montimage/5GReplay/files/6983612/arch.pdf)


## 1. Design a 5Greplay filtering rule 

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

## 2. Configure 5Greplay

## 3. Observe
