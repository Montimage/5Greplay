# Installation

## Pre-requires

Suppose on your machine, you have:

- **libxml2-dev, libpcap-dev, libconfuse-dev libsctp-dev** :  `sudo apt-get install libxml2-dev libpcap-dev libconfuse-dev libsctp-dev`
- **gcc, make, git**: `sudo apt install gcc make git`
- **Clone the source code on your machine**: `git clone https://github.com/Montimage/5GReplay.git`
- **mmt-sdk**: `cd 5GReplay; sudo dpkg -i lib/mmt-dpi*.deb`

## Clean

- Do `make clean` to clean compiled objects


## Compile


- compile on its local directory: `make`

- compile sample rules existing in `rules` folder: `make sample-rules`

- enable debug using gdb: `make DEBUG=1`


- if you want to use Valgrind DRD or Helgrind, you should do `make DEBUG=1 VALGRIND=1`. The option `VALGRIND=1` adds some instruction allowing Valgrind bypass atomic operations that usually causes false positive errors in Valgrind.
