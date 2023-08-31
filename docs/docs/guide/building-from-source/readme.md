# Installation

## Pre-requires

Suppose on your machine installed Ubuntu 18.04, you have:

- *gcc, make, git, libxml2-dev, libpcap-dev, libconfuse-dev libsctp-dev* :  `sudo apt update && sudo apt install gcc make git libxml2-dev libpcap-dev libconfuse-dev libsctp-dev`
- *the source code on your machine*: `git clone https://github.com/montimage/5greplay.git`
- [*mmt-dpi*](https://github.com/Montimage/mmt-dpi):
  -  If you use latest version of Ubuntu, then you can use its pre-compiled [Debian packages](https://github.com/Montimage/mmt-dpi/releases), for example : `wget https://github.com/Montimage/mmt-dpi/releases/download/v1.7.9/mmt-dpi_1.7.9_8694eaa_Linux_x86_64.deb && sudo dpkg -i ./mmt-dpi*.deb && sudo ldconfig`
  -  Otherwise, you can [compile MMT-DPI from its source code](https://github.com/Montimage/mmt-dpi/blob/master/docs/Compilation-and-Installation-Instructions.md), for example: `sudo apt-get update && sudo apt-get install -y gcc make build-essential git cmake libxml2-dev && git clone https://github.com/montimage/mmt-dpi && cd mmt-dpi/sdk && make && sudo make install`

## Clean

- Do `make clean` to clean compiled objects


## Compile


- compile on its local directory: `make`

- compile sample rules existing in `rules` folder: `make sample-rules`

- enable debug using gdb: `make DEBUG=1`


- if you want to use Valgrind [DRD](https://valgrind.org/docs/manual/drd-manual.html) or 
[Helgrind](https://valgrind.org/docs/manual/hg-manual.html), you should do `make DEBUG=1 VALGRIND=1`. The option `VALGRIND=1` adds some instruction allowing Valgrind bypass atomic operations that usually causes false positive errors in Valgrind.

## Screenshot

![screenshot](screenshot.gif)

# Usage

Please refer to [../../tutorial/replay-open5gs](../../tutorial/replay-open5gs)
