# Parameters in the configuration file

5Greplay requires a configuration file for setting different options:

- Online or offline mode
- Default action to be taken with the non-filtered packets

By default, 5Greplay will try to load the configuration from `./5greplay.conf` file in the current folder. 
A configuration file can be given to 5Greplay by using `-c` parameter, for example:
`./5greplay replay -c /home/tata/probe.conf`

*Note*: An attribute in the configuration file can be overrided by `-X` parameter. 
Multiple `-X` parameters are accepted. For example, the following command will override source attribute of input block to the coresponding values given after `=` sign.

`./5greplay replay -c /home/tata/smc-replay-attack.conf -Xinput.source=/tmp/a.pcap`

To list the attributes that can be overriden, run: `./5greplay replay -x`

A comment line inside a configuration starts by `#` sign. The options are listed in the following:


## 1. `input` block

This block configures the input of 5Greplay.

- `mode` can be either `ONLINE` or `OFFLINE` to indicate that 5Greplay will analyze respectively either traffic in realtime from a NIC or the traffic being stocked in a pcap file.
- `source` indicates the source of traffic to be analyze. It can be a pcap file or a NIC depending respectively on `OFFLINE` or `ONLINE` mode is used.
- `snap-len` indicates maximal size of an IP packet, by default 65355 bytes. It is used only in `ONLINE` mode.

The options mode and source need to be specified according to the requirements.

- `mode = ONLINE` allows near real-time analysis of network traffic. In PCAP mode, the NIC's network interface name needs to be identified.

   For example:
```bash
input{
   mode = ONLINE
   # For PCAP it is interface name source = "eth0"
}
```

- `mode = OFFLINE` allows analysis of a PCAP trace file. The source identifies the name of the trace file. The offline analysis is only available for the PCAP mode. 

   For example:
```bash
input{
   mode = OFFLINE
   source = "/home/tata/wow.PCAP"
}
```


## 2. `output` block

This block configures general output parameters. 

- `enable`: either `true` or `false`to indicate that when a rule is satisfied, its information and the one of satisfied packets will be output to a file or not.
- `output-dir`: path to the folder where output files are written
- `sample-interval`: period in seconds to create a new sample file
- `report-description`: `true` or `false` to include or not the rule's description into the alert reports.
 If the description is not included, it will be an empty string in the reports.
 Excluding rules's descriptions will reduce the size of reports.
 
 For example:
 ```bash
file-output {
   enable = true
   output-dir = "./"
   sample-interval = 5 #a new sample file is created each 5 seconds
   report-description = true 
}
```

*Note*: Disable reports output can gain some execution performance of 5Greplay.

## 3. `engine` block

The engine configuration block allows configure functionalities related with the 5Greplay rules.

- `thread-nb`: The option indicates the number of threads that 5Greplay will use for processing packets. 
It must be a positive number. Use 0 to have only one thread to read then analyze packets. 
Use x to have one thread to read packets and x threads to analyze the packets.

- `exclude-rules`: indicates the range of rules to be excluded from the verification.

   The range of rules is in BNF format: `exclude-rules = "x,y-z"`, in which x,y, z are positive numbers.
For example, `exclude-rules = "1,3-5,7,50-100"` will exclude the rules having id 1,3,4,5,7,50,51,...,100.

- `rules-mask`: It indicates the range of rules should be distributed to a specific analysis thread.
By default, the rules will be distributed increasingly to each thread. 
For example, given five rules having id 1, 5, 6, 7, 8 and two analysis threads, 
then, the first thread will analyzes rules 1, 5 while the second one analyzes rules 6, 7, 8. 
This can be represented by: 
```rules-mask = "(1:1,5)(2:6-8)"```

   Generally, the rules-mask uses the following BNF format: `rules-mask = (thread-index:rule-range)`
in which, `thread-index` is a positive integer; `rule-range` is either
a positive integer, or a range of numbers (see `exclude-rules`).

   For example,if we have `thread-nb = 3` and `rules-mask = "(1:1,4-6)(2:3)"`, then, 
thread 1 verifies rules 1,4,5,6; thread 2 verifies only rule 3;
and thread 3 verifies the rest.

   *Note*: if we have `thread-nb = 2` and `rules-mask = "(1:1)(2:3)"`, 
then only rules 1 and 3 are verified, the other rules are not.


- `ip-encapsulation-index`: this option selects which IP layer will be analyzed in case there exist IP encapsulation. 
Its value can be one of the followings:
   + `FIRST`: first IP in the protocol hierarchy 
   + `LAST`: last IP in the protocol hierarchy 
   + *i*: *ith* IP in the protocol hierarchy.

   For example, given `ETH.IP.UDP.GTP.IP.TCP.VPN.IP.SSL`:
   + `FIRST`, or 1, indicates `IP` after `ETH` and before `UDP`
   + `LAST`, or any number >= 3, indicates `IP` after `VPN`
   + 2 indicates `IP` after `GTP` and before `TCP`

- `max-instances`: maximum number of instances of a rule. Default = 100000

For example:

```bash
engine{
   thread-nb = 0 
   exclude-rules = "(50-100)" 
   rules-mask = "" 
   ip-encapsulation-index = LAST 
   max-instances = 100000
}
```

## 4. `mempool`block

The mempool configuration block sets the maximum elements of a pool of memory blocks.

- `max-bytes`: This parameter set the maximum bytes of a pool. For example, for 2 GBytes write 2000000000.
- `max-elements`: Max number of elements in a pool.
- `max-message-size`: Maximum size, in bytes, of an input.
- `smp-ring-size`: Number of reports can be stored in a ring buffer.

For example:

```bash
mempool{
   max-bytes = 2000000000 
   max-elements = 1000 
   max-message-size = 3000 
   smp-ring-size = 1000
}
```

## 5. `forward` block

The forward configuration block configures parameters related with the forwarding capability of 5Greplay.

- `enable`: Set to `true` or `false` to enable or  disable respectively.
- `output-nic`: Output network interface to forward the network traffic.
- `nb-copies`: Number of copies of a packet to be sent
- `snap-len`: Specifies the snapshot length to be set on the handle.
- `promisc`: Specifies whether the interface is to be put into promiscuous mode. If promisc is non-zero, promiscuous mode will be set, otherwise it will not be set.
- `default`: Default action when packets are not selected/satisfied by any rule. 
Either `FORWARD` to forward the packets or `DROP` to drop the packets.
- `target-protocols`: List of transport protocols over which the forwarded packets' payload will be sent. 

   Currently support either `SCTP`, if the forwarded packets work over SCTP, `UDP` if the forwarded packets work over UDP or `HTTP2`if the forwardeds packets has HTTP2 payload, or all of them if these types of packets are present.
- `target-hosts`: List of IP addresses to forward the packets. Each address matches with the transport protocol selected before.
- `target-ports`: List of ports to forward the packets. Each address matches with the transport protocol selected before.



In the following example, STCP traffic will be send over `127.0.0.5:38412` and UDP traffic over `127.0.0.7:2152`:

```bash
forward {
   enable     = true
   output-nic = "lo"
   nb-copies  = 2 #number of copies of a packet to be sent
   snap-len   = 0 #specifies the snapshot length to be set on the handle.
   promisc    = 1
   default    = DROP 
   
   #forward packets to a target using SCTP protocol: MMT will be a SCTP client, 
   # - it connects to the given "sctp-host" at "sctp-port"
   # - the SCTP packets' payload will be sent to the target using this SCTP connection
   target-protocols = { SCTP, UDP}
   target-hosts     = { "127.0.0.5", "127.0.0.7" }
   target-ports     = { 38412, 2152 }
}
```
# Links

- [Command line parameters](../commands)
- [Replay Example](../../tutorial/replay-open5gs)
