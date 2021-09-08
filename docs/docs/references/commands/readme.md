# Command Usage

5Greplay command uses the following form: `command [option]`

 - command : is one of the following `compile`, `info`, `list`, `replay`
 - option  : run "./5greplay command -h" to get option of each command, for example, `./5greplay replay -h`
 

## compile
This command parses rules in an .xml file, then compile to a plugin .so file.

This command has 3 parameters:

1. output location: it should be ended by `.so` or `.c` when generating binary or code C respectively
2. input xml file containing rules to be compiled
3. optional `-c` to generate code C, or any string that will be transfer to gcc compilation.

To be able to compile a rule, you should install gcc: `sudo apt install gcc`

```bash
#to generate .so file
./5greplay compile rules/forward-localhost.so rules/forward-localhost.xml
#to generate code c (for debug)
./5greplay compile rules/forward-localhost.c rules/forward-localhost.xml -c
```

To compile all rules existing in the folder `rules`, use the following command: `make sample-rules`

## info

This command prints information of rules encoded in a binary file (.so).

This command has one optional parameter. When it is ignored, all rules inside `rules` folder will be visited.
Otherwise if it is present and points to a .so rule file, then only rules inside the file will be visited.

```bash
#print information of all available plugins
./5greplay info
#print information of rules encoded in `rules/nas-smc-replay-attack.so`
./5greplay info rules/nas-smc-replay-attack.so
```

## list

This command lists all protocols and their attributes supported by 5Greplay. This command has no parameter.

```bash
./5greplay list
```

## replay 

This command can replay
 
- either real-time traffic by capturing traffic from a given NIC,
- or traffic saved in a pcap file.

```bash
#Get list of parameters
./5greplay replay -h
./5greplay replay [<options>]
Option:
	-v               : Print version information, then exits.
	-c <config file> : Gives the path to the configuration file (default: ./5greplay.conf).
	-t <trace file>  : Gives the trace file for offline analyse.
	-i <interface>   : Gives the interface name for live traffic analysis.
	-X attr=value    : Override configuration attributes.
	                    For example "-X output.enable=true -Xoutput.output-dir=/tmp/" will enable output to file and change output directory to /tmp.
	                    This parameter can appear several times.
	-x               : Prints list of configuration attributes being able to be used with -X, then exits.
	-h               : Prints this help, then exits.

#Note: you may want to change parameters inside mmt-5greplay.conf
#replay online traffic comming from eth0
sudo ./5greplay replay -i eth0
#replay offline traffic being stored inside a pcap file
sudo ./5greplay replay -t ~/pcap/5G-traffic.pcap 
```

# Screenshot

![screenshot](screenshot.gif)

