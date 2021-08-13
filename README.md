5GReplay

**5Greplay** is a 5G network traffic fuzzer that enables the evaluation of 5G components by replaying and modifying 5G network traffic, by creating and injecting network scenarios into a target that can be a 5G core service (e.g., AMF, SMF) or a RAN network (e.g., gNodeB). The tool provides the ability to alter network packets online or offline in both control and data planes in a very flexible manner.

This repository contains the following folders:

- `src` : C code of mmt-5greplay
- `rules`: set of example XML rules
- `docs`: [documentation](docs/)
- `test`: diversity of testing code

# Installation

## Pre-requires

Suppose on your machine, you have:

- **libxml2-dev, libpcap-dev, libconfuse-dev libsctp-dev** :  `sudo apt-get install libxml2-dev libpcap-dev libconfuse-dev libsctp-dev`
- **gcc, make, git**: `sudo apt install gcc make git`
- **Clone the source code on your machine**: `git clone https://github.com/Montimage/5GReplay.git`
- **mmt-sdk**: `cd 5GReplay; sudo dpkg -i lib/mmt-dpi*.deb; sudo ldconfig`

## Clean

- Do `make clean` to clean compiled objects


## Compile


- compile on its local directory: `make`

- compile sample rules existing in `rules` folder: `make sample-rules`

- enable debug using gdb: `make DEBUG=1`


- if you want to use Valgrind DRD or Helgrind, you should do `make DEBUG=1 VALGRIND=1`. The option `VALGRIND=1` adds some instruction allowing Valgrind bypass atomic operations that usually causes false positive errors in Valgrind.

# Usage

mmt-5greplay command uses the following form: `command [option]`

 - command : is one of the following `compile`, `info`, `replay`
 - option  : run "./mmt-5greplay command -h" to get option of each command
 

## compile
This command parses rules in .xml file, then compile to a plugin .so file.

```bash
#to generate .so file
./mmt-5greplay compile rules/forward-localhost.so rules/forward-localhost.xml
 
#to generate code c (for debug)
./mmt-5greplay compile rules/forward-localhost.c rules/forward-localhost.xml -c

```

To compile all rules existing in the folder `rules`, use the following command: `make sample-rules`

## info

This command prints information of rules encoded in a binary file (.so).

```bash
#print information of all available plugins
./mmt-5greplay info
#print information of rules encoded in `rules/nas-smc-replay-attack.so`
./mmt-5greplay info rules/nas-smc-replay-attack.so
```

## replay 

This command can analyze
 
- either real-time traffic by monitoring a NIC,
- or traffic saved in a pcap file. The verdicts will be printed to the current screen.

```bash
./mmt-5greplay replay [<options>]
Option:
	-v               : Print version information, then exits.
	-c <config file> : Gives the path to the configuration file (default: ./mmt-5greplay.conf).
	-t <trace file>  : Gives the trace file for offline analyse.
	-i <interface>   : Gives the interface name for live traffic analysis.
	-X attr=value    : Override configuration attributes.
	                    For example "-X output.enable=true -Xoutput.output-dir=/tmp/" will enable output to file and change output directory to /tmp.
	                    This parameter can appear several times.
	-x               : Prints list of configuration attributes being able to be used with -X, then exits.
	-h               : Prints this help, then exits.
   
#online analysis on eth0
./mmt-5greplay replay -i eth0
#to see all parameters, run ./mmt-5greplay replay -h
#verify a pcap file
./mmt-5greplay replay -t ~/pcap/5G-traffic.pcap 

```


# Documentation

[docs](docs/)
