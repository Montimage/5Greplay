# Replay 5G traffic against OpenAirInterface 5G core

We recently participated into [Fall 2021 OpenAirInterface Workshop: Hands-On with the OAI Architects](https://openairinterface.org/fall-2021-openairinterface-workshop/).
It was a great moment to be able to test the OAI RAN and core 5G systems. After that workshop, we tried to replay 5G traffic against OAI core. A bug was found and [shared to OAI](https://github.com/OPENAIRINTERFACE/openair-epc-fed/issues/28).

The test was done on December 15, 2021.

# Install OAI 5G core

OAI provides an easy way to install 5G core via a [python script](https://gitlab.eurecom.fr/oai/cn5g/oai-cn5g-fed/-/blob/master/docs/DEPLOY_SA5G_MINI_DS_TESTER_DEPLOYMENT.md#7-deploying-oai-5g-core-network). For further information, please see https://gitlab.eurecom.fr/oai/cn5g/oai-cn5g-fed/

Basically, we can use the following commands:

```bash
#prepare environment
sudo sysctl net.ipv4.conf.all.forwarding=1
sudo iptables -P FORWARD ACCEPT
docker network create \
  --driver=bridge \
  --subnet=192.168.70.128/26 \
  -o "com.docker.network.bridge.name"="demo-oai" \
  demo-oai-public-net

#download 5G core
git clone https://gitlab.eurecom.fr/oai/cn5g/oai-cn5g-fed.git
cd oai-cn5g-fed/docker-compose

#start docker compose "scenario 1"
python3 ./core-network.py --type start-mini --fqdn no --scenario 1
```

# Start 5Greplay

```bash
#Install 5Greplay
sudo apt update && sudo apt install gcc make git libxml2-dev libpcap-dev libconfuse-dev libsctp-dev
git clone https://github.com/montimage/5greplay.git
cd 5greplay; sudo dpkg -i lib/mmt-dpi*.deb; sudo ldconfig
make
make sample-rules

#Start 5Greplay
sudo ./5greplay replay -t pcap/sa.pcap -Xforward.target-ports=38412 -Xforward.target-hosts=192.168.70.132 -Xforward.nb-copies=1 -Xforward.default=FORWARD
```

# Screencast

<video controls="true" allowfullscreen="true">
    <source src="screencast.mp4" type="video/mp4">
  </video>

# Result

A null pointer dereference was detected in AMF:

```
ASAN:DEADLYSIGNAL
=================================================================
==1==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000000 (pc 0x000000000000 bp 0x7fbbf10fb410 sp 0x7fbbf10fb3c8 T4)
==1==Hint: pc points to the zero page.
==1==The signal is caused by a READ memory access.
==1==Hint: address points to the zero page.

AddressSanitizer can not provide additional info.
SUMMARY: AddressSanitizer: SEGV (<unknown module>) 
Thread T4 created by T0 here:
   #0 0x7fbbfd640d2f in __interceptor_pthread_create (/usr/lib/x86_64-linux-gnu/libasan.so.4+0x37d2f)
   #1 0x56037c7b6a00 in ngap::ngap_app::ngap_app(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, unsigned short) (/openair-amf/bin/oai_amf+0x34ba00)
   #2 0x56037c715bf7 in amf_application::amf_n2::amf_n2(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, unsigned short) (/openair-amf/bin/oai_amf+0x2aabf7)
   #3 0x56037c6d4e49 in amf_application::amf_app::amf_app(config::amf_config const&) (/openair-amf/bin/oai_amf+0x269e49)
   #4 0x56037c6702ff in main (/openair-amf/bin/oai_amf+0x2052ff)
   #5 0x7fbbfa8a0bf6 in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x21bf6)

==1==ABORTING
```

The full execution logs and captured traffic can be found [here](./dataset.zip)
