# Example 1: Filtering and replaying NGAP/NAS-5G Security Mode Complete messages

This document aims to show the main functionalities of 5Greplay through an example. For more details please use the 5Greplay [User Manual](./5Greplay_Manual.pdf) as a reference.

In general, an use case of 5Greplay It will be composed of 3 main steps:
1. Design a rule that filter the desired packets to be replayed
2. Configure 5Greplay the through its configuration [file](../../mmt-5greplay.conf), to determine which rules will be appliyed and where the traffic will be sent
3. Run and observe the replayed traffic, and the 5Greplay logs

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

After having written the rules and verifying that they were not syntax errors during compilation. You must indicate to 5Greplay **where to forward the traffic and what to do with the packets that did not fulfill the properties** that you defined in your rules. This will be defined by default in the ````5GReplay/mmt-5Greplay.conf``` file. Section 3 of the [User Manual](./5Greplay_Manual.pdf) explains in detail all the fields of the configuration file.

For this example we will use 2 alternative configuration files saved in ````5GReplay/examples/example1_ngap_smc/```. 

### `mmt-5greplay-udp.conf`: a 5Greplay configuration file to replay the input traffic into a specific UDP port 

The change we have done to the default configuration file (and that in general you must do for each usecase) are as follow:

- In ```the exclude-rules``` field of the ```engine``` section, we have excluded all the properties from 0 to 89 and from 91 to 200, in order to only take into account the property 90, that we have define in the Section 1 of this example. 

```bash
	exclude-rules = "0-89,91-200"  # Range of rules to be excluded from the verification
```
- In the ```forward``` Section we have defined that (i) the network interface where we will send the traffic is ```ens38```, in the ```output-nic``` field,  (ii) the number of copies we desire to make of the forwarded packets are 2, in the ```nb-copies``` field, (iii) the destination IP address, port and transport layer protocol, will be ```192.168.49.7:2152```and the UDP protocol will be used, and (iv) the default action that will be taken for the packets that do not fufill the property 90 will be ```FORWARD```, defined in the ```default``` field.

```bash
forward
{
    ...
    
	output-nic = "ens38"
	nb-copies  = 2 #number of copies of a packet to be sent
    
	...
    
    default    = FORWARD #default action when packets are not selected/satisfied by any rule
		# either FORWARD to forward the packets or DROP to drop the packets
   
    ...
    
	#forward packets to a target using SCTP protocol: MMT will be a SCTP client, 
	# - it connects to the given "sctp-host" at "sctp-port"
	# - the SCTP packets' payload will be sent to the target using this SCTP connection
	target-protocols = { UDP}
	target-hosts     = { "192.168.49.7" }
	target-ports     = { 2152 }
}
```

### `mmt-5greplay-sctp.conf` : a 5Greplay configuration file to replay filter and replay NGAP SMC messages into a running AMF in a specific SCTP port in a specific IP address

In this case, we kept all the configuration as in the example before, but we changed (i) the default action to ```DROP```, so only the NAS-5G SMC packets filtered by the property 90 will be forwarded, (ii) we added a another destination for the packets using SCTP protocol, so they will be forwarded to the following address: ```192.168.49.3:38412```, where ideally you will have an AMF running so you could see its response to the forwarded packets.

```bash
forward
{
	...
    
	default    = DROP #default action when packets are not selected/satisfied by any rule
		# either FORWARD to forward the packets or DROP to drop the packets
	
    ...
    
	#forward packets to a target using SCTP protocol: MMT will be a SCTP client, 
	# - it connects to the given "sctp-host" at "sctp-port"
	# - the SCTP packets' payload will be sent to the target using this SCTP connection
	target-protocols = { SCTP, UDP}
	target-hosts     = { "192.168.49.3", "192.168.49.3" }
	target-ports     = { 38412, 2152 }
}
```


## 3. Run and Observe
