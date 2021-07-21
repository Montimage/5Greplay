# MMT-5Greplay

This repository contains the following folders:

- `src` : C code of mmt-5greplay
- `rules`: set of official XML rules. An encoded version (*.so) of these rules will be distributed with mmt-sec when using `make deb`. All rules (official and for testing purposes) are stored in rules/properties_all 
- `check`: sample pcap files and expected results to validate mmt-5greplay
- `docs`: [documentation](docs/)
- `test`: diversity of testing code

# Build

## Pre-requires

Suppose on your machine, you have:

- *libxml2-dev, libpcap-dev, libconfuse-dev* :  `sudo apt-get install libxml2-dev libpcap-dev libconfuse-dev`

- `mmt-sdk`: `sudo dpkg -i lib/mmt-dpi*.deb`

## Clean

- Do `make clean` to clean compiled objects


## Compile


- compile on its local directory: `make`

- compile sample rules existing in `rules` folder: `make sample-rule`

- enable debug using gdb: `make DEBUG=1`


- if you want to use Valgrind DRD or Helgrind, you should do `make DEBUG=1 VALGRIND=1`. The option `VALGRIND=1` adds some instruction allowing Valgrind bypass atomic operations that usually causes false positive errors in Valgrind.

# Execution

mmt-5greplay command uses the following form: `command [option]`

 - command : is one of the following `compile`, `info`, `replay`
 - option  : run "./mmt-5greplay command -h" to get option of each command
 

## compile
This command parses rules in .xml file, then compile to a plugin .so file.

```bash
#to generate .so file
./mmt-5greplay compile rules/40.TCP_SYN_scan.so rules/40.TCP_SYN_scan.xml
 
#to generate code c (for debug)
./mmt-5greplay compile rules/40.TCP_SYN_scan.c rules/40.TCP_SYN_scan.xml -c

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
./mmt-5greplay replay -t check/pcap/16.two_successive_SYN.pcap 

```


# Documentation

[docs](docs/)
