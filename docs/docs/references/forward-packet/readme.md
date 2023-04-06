# Forward packets in 5Greplay

This document details how 5Greplay forwards packets

5Greplay can forward raw packets as-is into the outgoing NIC using 
[libpcap](https://github.com/the-tcpdump-group/libpcap) or 
[DPDK](https://www.dpdk.org/) (if the tool is compiled with DPDK).

The implementation is in [src/forward](https://github.com/Montimage/5GReplay/tree/main/src/forward) folder.

To avoid packets being considered as duplication, then rejected by the endpoint's protocol, 
5Greplay supports to create a new stream between it and the endpoint, then use this stream to inject the packet's payloads.

Currently it supports SCTP and UDP streams. The information of endpoints is given via a configuration file
by using the following parameters:
 
- `forward.target-protocols`: list of stream protocols, for example, `{SCTP,UDP}`
- `forward.target-hosts`: IPs of the targets corresponding to each protocol
- `forward.target-ports`: Port numbers of the targets corresponding to each protocol.

# Example

If we want to use a new SCTP connection to forward SCTP packets' payloads, 
and other packets (including UDP ones) are forwarded as they-are, then we can use the following configuration:

```bash
forward
{
	enable     = true
	output-nic = "lo"
	nb-copies  = 1
	snap-len   = 0
	promisc    = 1
	default    = DROP
	target-protocols = { SCTP }
	target-hosts     = { "127.0.0.5" }
	target-ports     = { 38412 }
}
```

Using these config parameters, all the payloads of SCTP packets will be sent to `127.0.0.5` at port `38412`.
The other packets (excluding the packets containing SCTP protocol) will be injected into `lo` interface.