Quickstart with 5Greplay

# Installation

The current executable versions of 5Greplay should be executed in Ubuntu18.04 in which it was compiled. 
If you want to run 5Greplay in another OS, please compile it from source code or use its docker container.

The executable versions of 5Greplay are avaiable at [https://github.com/Montimage/5GReplay/releases](https://github.com/Montimage/5GReplay/releases)

## For example:
```bash
# Install wget to be able to download 5Greplay
sudo apt update && sudo apt install -y wget
# Download 5Greplay, version 0.0.1
wget https://github.com/Montimage/5GReplay/releases/download/v0.0.1/5greplay-0.0.1_Linux_x86_64.tar.gz
# Decompress 5Greplay
tar -xzf 5greplay-0.0.1_Linux_x86_64.tar.gz
# View 5Greplay's parameter
cd 5greplay-0.0.1
./5greplay -h
```

## Screenshot

![screenshot](screenshot.gif)

# Usage

Please refer to [../../tutorial/replay-open5gs](../../tutorial/replay-open5gs)