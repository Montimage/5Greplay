# Filtering and replaying NGAP/NAS-5G Security Mode Complete messages

This tutorial reproduces the Scenario 3 in the ARES [paper](../../publications/ares2021-paper.pdf).
Its main objective is to test whether an AMF tolerates an SMC replay attack.

In general, an use case of 5Greplay will be performed via 3 main steps:

1. Design a rule that filter the desired packets to be replayed
2. Configure 5Greplay the through its [configuration](../../references/configuration-file) file, 
to determine which rules will be applied and where the traffic will be sent
3. Run and observe the replayed traffic, and the 5Greplay logs

<img width="499" alt="image" src="https://user-images.githubusercontent.com/45805561/129385771-d266f04c-0011-4685-9bb6-3ddb933550aa.png">


## 1. Design a 5Greplay filtering rule 

The first thing we need to define is **what packets we desire to forward, drop or modify**. 
For that we need to make a 5Greplay XML rule, that normally will be located in `rules/` folder.

We explain in detail all the attributes of 5Greplay rules [here](../../references/rule-xml-semantics),
and how to write a rule [here](../../references/rule)

To have reference of the name of the protocol attributes that 5Greplay uses, 
and that we can use to write rules (e.g. `nas_5g.message_type, nas_5g.security_type`),
please refers to [this](../../references/protocols-attributes-list).

For this example we will be using the `nas-smc-replay-attack.xml` located in `rules/` folder of the source code:

```xml
<beginning>
<!-- Property 90: Forwarding NAS security mode COMPLETE that answers to NAS security mode COMMAND.-->
<property value="THEN"  delay_units="ms" delay_min="0" delay_max="10" property_id="90" type_property="FORWARD" 
    description="Forwarding NAS security mode COMPLETE that answers to NAS security mode COMMAND " >
    <event value="COMPUTE" event_id="1" description="NAS Security mode COMMAND"
           boolean_expression="(nas_5g.message_type == 93)"/>
    <event value="COMPUTE" event_id="2" description="NAS Security mode COMPLETE"
           boolean_expression="(nas_5g.security_type == 4)"/>
</property>
</beginning>
```

- The property is compose of *2 events*, that must occur with a minimun delay of 0 ms (i.e., they are in the same packet)
and a maximun delay of 10 ms (they are 2 different packets), as stablished in: 

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
    <event value="COMPUTE" event_id="2" description="NAS Security mode COMPLETE"
           boolean_expression="(nas_5g.security_type == 4)"/>
```

With the lines before we have determined that we want to filter the _NAS Security mode Complete_ messages. 
By default 5Greplay will forward them into the destination that we will define in the configuration file. 
However, in this step we can also what action should be performed if the property is satisfied, 
for example, by adding the field `if_satisfied` in the property.

After have written wer rule, we can compile it, by doing:

```bash
# to generate .so file
./5greplay compile rules/forward-localhost.so rules/forward-localhost.xml
# to generate code c (only need for debug)
./5greplay compile rules/forward-localhost.c rules/forward-localhost.xml -c
```

Or we can compile all rules existing in the `rules/` folder of the source code, 
use the following command: `make sample-rules`.

## 2. Configure 5Greplay

After having written the rules and verifying that they were not syntax errors during compilation. 
We need to indicate to 5Greplay **where to forward the traffic and what to do with the packets that did not fulfill the properties** 
that we defined in wer rules. 
This will be defined by default in the `5greplay.conf` file. 
We explain in detail all the fields of the configuration file [here](../../references/configuration-file).

For this example we will use 2 alternative configuration files:

- [5greplay-udp.conf](5greplay-udp.conf)
- [5greplay-sctp.conf](5greplay-sctp.conf) 

### Configuration file 1: `5greplay-udp.conf`

The change we have done to the default configuration file (and that in general we must do for each usecase) are as follow:

- In the `exclude-rules` field of the `engine` section, 
we have excluded all the properties from 0 to 89 and from 91 to 200, 
in order to **only** take into account the property 90. 

   ```bash
exclude-rules = "0-89,91-200"  # Range of rules to be excluded from the verification
```

- In the `forward` Section we have defined that

   + the network interface where we will send the traffic is `ens38`, in the `output-nic` field,
   + the number of copies we desire to make of the forwarded packets are 2, in the `nb-copies` field,
   + the destination IP address, port and transport layer protocol, will be `192.168.49.7:2152`and the UDP protocol will be used, and
   + the default action that will be taken for the packets that do not fufill the property 90 will be `FORWARD`, defined in the `default` field.

   ```bash
forward {
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

### Configuration file 2: `5greplay-sctp.conf` 

In this case, we kept all the configuration as in the example before, but we changed

- the default action to `DROP`, so only the _NAS Security mode Complete_ packets filtered by the property 90 will be forwarded,
- we added a another destination for the packets using SCTP protocol, 
  so they will be forwarded to the following address: `192.168.49.3:38412`, 
  where ideally we will have an AMF running so we could see its response to the forwarded packets.

   ```bash
forward {
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

After having we rules and configuration file, we can run 5Greplay and check if everything goes as desired. 
5Greplay can process online and offline traffic. However, for testing purposes, we recommend starting by processing a pcap file.

In this tutorial, we provide [ue_authetication.pcapng](ue_authetication.pcapng) file that contains the authetication process between an UE and an AMF, 
so our _NAS Security mode Complete_ packet will be present, to check if the rule actually filter it, 
and if it is forwarded to the desired addresses, depending on the configuration file used.

### Configuration file 1
Let's run 5Greplay using `5greplay-udp.conf` and with the `ue_authetication.pcapng` file as the input:

```bash
sudo ./5greplay replay -c 5greplay-udp.conf -t ue_authetication.pcapng
```

We should be able to observe the interface `ens38` with a packet analyzer, 
such as [Wireshark](https://www.wireshark.org/) or 
[tcpdump](https://www.tcpdump.org/) and to see all the packets in the file forwarded, 
as we have defined as `FORWARD` the default action. 
Moreover, 5Greplay log will report the amount of forwarded and dropped packets.

```bash
5greplay: 5Greplay 0.0.1 (40dab57 - Jul 22 2021 06:17:58) is verifying 1 rules having 2 proto.atts using the main thread
5greplay: Analyzing pcap file ue_authetication.pcapng
Loaded successfully rule 1 - rule 90 generated 1 verdicts
          13 packets received
          13 messages received
           1 alerts generated
5greplay: Number of packets being successfully forwarded: 26, dropped: 0
Number of packets being successfully forwarded: 26, dropped: 0
```

### Configuration file 2

Let's run 5Greplay using `5greplay-sctp.conf` and with the `ue_authetication.pcapng` file as the input:

```bash
sudo ./5greplay replay -c 5greplay-sctp.conf -t ue_authetication.pcapng 
```

We should be able to observe the interface `ens38` with a packet analyzer, such as _Wireshark_ or _tcpdump_ and to see only 2  packets forwarded, the _NAS Security mode Complete_ filtered and forwarded, and a copy, as we have defined as `DROP` the default action. Moreover, 5Greplay log will report the amount of forwarded and dropped packets.

```bash
5greplay: 5Greplay 0.0.1 (40dab57 - Jul 22 2021 06:17:58) is verifying 1 rules having 2 proto.atts using the main thread
5greplay: Analyzing pcap file ue_authetication.pcapng
Loaded successfully rule 1 - rule 90 generated 1 verdicts
          13 packets received
          13 messages received
           1 alerts generated
5greplay: Number of packets being successfully forwarded: 2, dropped: 12
Number of packets being successfully forwarded: 2, dropped: 12
```

**Notice** that we only will be able to run this example if we have a SCTP server running at the defined IP address and port, otherwise we will have the following error:

```bash
5greplay: [_sctp_connect:49] Cannot connect to 192.168.49.3:38412 using SCTP
5greplay: Interrupted by signal 6
```

We recommend to use the open source projects [free5gc](https://github.com/free5gc/free5gc) or 
[open5gs](https://github.com/open5gs/open5gs) to have we own 5G SA core network. 
Also, we can used [UERANSIM](https://github.com/aligungr/UERANSIM) to have the UE and RAN implementation. 
You can design rules to filter and forward packets between the core and the RAN netwroks, **and much more!**


# Links

- [Command line parameters](../../references/commands)
- [Configuration file](../../references/configuration-file)
- [XML rule file](../../references/rule)