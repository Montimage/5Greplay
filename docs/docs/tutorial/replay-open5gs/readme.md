# Replay 5G traffic against open5Gs

# Install 5Greplay

In this tutorial, we use executable version of 5Greplay. Further information about installation of 5Greplay, please refer to [docs](../../../docs.html)

```bash
sudo apt update && sudo apt install -y wget
# Download 5Greplay, version 0.0.1
wget https://github.com/Montimage/5GReplay/releases/download/v0.0.1/5greplay-0.0.1_Linux_x86_64.tar.gz
# Decompress 5Greplay
tar -xzf 5greplay-0.0.1_03501eb_Linux_x86_64.tar.gz
# View 5Greplay's parameter
cd 5greplay-0.0.1
./5greplay replay -h
```

# Install open5Gs

Please refer to [open5Gs](https://open5gs.org/open5gs/) for further details about its installation.

For this tutorial, we can used the following commands to install open5Gs on Ubuntu 18.04:  

```bash
sudo apt update && sudo apt install software-properties-common
sudo add-apt-repository ppa:open5gs/latest
sudo apt update
sudo apt install open5gs
```

# Replay 5G traffic

```bash
# Download pcap file
wget https://github.com/Montimage/5GReplay/raw/main/docs/docs/tutorial/replay-open5gs/5g-sa.pcap
 