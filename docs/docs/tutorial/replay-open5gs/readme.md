# Replay 5G traffic against open5Gs

This tutorial has been done in Ubuntu 18.04 as shown below:

```bash
montimage@montimage:~$ uname -a
Linux montimage 4.15.0-156-generic #163-Ubuntu SMP Thu Aug 19 23:31:58 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
montimage@montimage:~$ lsb_release -a
No LSB modules are available.
Distributor ID:	Ubuntu
Description:	Ubuntu 18.04.5 LTS
Release:	18.04
Codename:	bionic
```

# Objective

This tutorial is used to reproduce the Scenario 4 in the ARES [paper](../../publications/ares2021-paper.pdf).
We will use 5Greplay to replay the traffic that was saved into a pcap file. 
The generated traffic will be amplified by creating several copies and sent them to open5Gs. 
We will observe thatthe AMF componant of open5Gs will be crashed.



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

The following commands will try to replay traffic that was captured and saved into a [pcap file](https://github.com/Montimage/5GReplay/raw/main/docs/docs/tutorial/replay-open5gs/5g-sa.pcap).
We first download the pcap file. We then run 5greplay in background to be able to view the execution log of open5Gs AMF.

```bash
# Download pcap file
wget https://github.com/Montimage/5GReplay/raw/main/docs/docs/tutorial/replay-open5gs/5g-sa.pcap
# Replay traffic in background
sudo ./5greplay replay -t 5g-sa.pcap -Xforward.nb-copies=2000 -Xforward.default=FORWARD > log.txt 2>&1
# View log of open5Gs
sudo tail -f /var/log/open5gs/amf.log
``` 

# Screenshot

![screenshot](screenshot.gif)

# Analyse result

We got the following error of open5Gs AMF (Open5GS daemon v2.3.3):

```
09/09 12:23:54.635: [mem] ERROR: cluster_alloc: Expectation `buffer' failed. (../lib/core/ogs-pkbuf.c:314)
09/09 12:23:54.635: [mem] ERROR: ogs_pkbuf_alloc() failed [size=8192] (../lib/core/ogs-pkbuf.c:198)
09/09 12:23:54.635: [core] ERROR: ogs_asn_encode: Expectation `pkbuf' failed. (../lib/asn1c/util/message.c:31)
09/09 12:23:54.635: [ngap] ERROR: ogs_ngap_encode: Expectation `pkbuf' failed. (../lib/ngap/message.c:34)
09/09 12:23:54.635: [amf] ERROR: ngap_send_error_indication: Expectation `ngapbuf' failed. (../src/amf/ngap-path.c:577)
09/09 12:23:54.640: [amf] FATAL: amf_state_operational: Assertion `OGS_OK == ngap_send_error_indication( gnb, NULL, NULL, NGAP_Cause_PR_protocol, NGAP_CauseProtocol_abstract_syntax_error_falsely_constructed_message)' failed. (../src/amf/amf-sm.c:691)
09/09 12:23:54.640: [core] FATAL: backtrace() returned 7 addresses (../lib/core/ogs-abort.c:37)
/usr/bin/open5gs-amfd(+0x1c895) [0x55dd35a19895]
/usr/lib/x86_64-linux-gnu/libogscore.so.2(ogs_fsm_dispatch+0x16) [0x7f0eaab13436]
/usr/bin/open5gs-amfd(+0x6056) [0x55dd35a03056]
/usr/lib/x86_64-linux-gnu/libogscore.so.2(+0xdc28) [0x7f0eaab0ac28]
/lib/x86_64-linux-gnu/libpthread.so.0(+0x76db) [0x7f0ea8e686db]
/lib/x86_64-linux-gnu/libc.so.6(clone+0x3f) [0x7f0ea8b91a3f]
```

After digging in github issue of [open5Gs repository](https://github.com/open5gs/open5gs), we found that
[`ogs_pkbuf_alloc`](https://github.com/open5gs/open5gs/issues?q=ogs_pkbuf_alloc) error occurs when number of UEs pass a [pre-configured number](https://github.com/open5gs/open5gs/pull/423).

# Links

- [Command line parameters](../../references/commands)
- [Configuration file](../../references/configuration-file)
- [XML rule file](../../references/rule)