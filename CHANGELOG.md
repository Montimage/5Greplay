# Version 1.0.0
- Add running parameter,`-U`, allowing to forward user input to rules
- Add more rules: 
    - Rules to modify Ethernet and IP via user input parameters:
    - Rule to emulate DDoS attack by randomizing a huge number of IP source or destination
    - Rule to delay Ethernet packet by a given microseconds
- Fixed bugs:
    - Rule order has been changed after exluding some rules
    - No forward packet if user does explicity call `forward_packet` embdded function, even rule's type is `FORWARD`

# Version 0.0.7
- Bind SCTP socket in a specific IP. Fixed #43, fixed #45

# Version 0.0.7
- HTTP2 mutation and forwarding packets

# Version 0.0.6
- SCTP socket uses `SCTP_NODELAY` to generate UDP packets as fast as possible. It prevents SCTP to cumulate multiple chunks in a single UDP packet.
- reconnect to SCTP server once it closed the connection
- enhance statistics: print total number of injected packets

# Version 0.0.5
- extended `ngap.ran_ue_id` to store 64-bit values
- add config parameters for SCTP connection medium
- fix bug when dumping packet without forwarding
- fix bug in `-Xegine.exclude-rules` parameter

# Version 0.0.4
- dump output packets to a pcap file
- support `#fuzz` embedded function
- fixed bug when wrong display info of `FORWARD` rules