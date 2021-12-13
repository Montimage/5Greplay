Quickstart with 5Greplay

# Installation

Docker images of 5Greplay are available [https://github.com/orgs/Montimage/packages?repo_name=5Greplay](here)
A new image is created and pushed automatically when having a new release.

You need to install Docker firstly: [https://docs.docker.com/get-docker](get docker)

```bash
# pull the latest image of 5Greplay
docker pull ghcr.io/montimage/5greplay:latest

# run 5Greplay 
docker run --rm -it ghcr.io/montimage/5greplay
```

In the docker image, there are some sample pcap files inside [https://github.com/Montimage/5Greplay/tree/main/pcap](`pcap/`) folder.


For example, to replay packets inside `pcap/sa.pcap` and each packet is copied 2000 times,
 to an AMF that is listening at `10.0.0.2:38412` using `SCTP` protocol, run:

```
docker run --rm -it ghcr.io/montimage/5greplay replay -t pcap/sa.pcap -Xforward.target-ports=38412 -Xforward.target-hosts=10.0.0.2 -Xforward.nb-copies=2000 -Xforward.default=FORWARD
```


# Usage

Please refer to [../../tutorial/replay-open5gs](../../tutorial/replay-open5gs)
