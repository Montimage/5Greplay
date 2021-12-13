FROM ubuntu:18.04

LABEL maintainer="Montimage <contact@montimage.com>"

ENV INSTALL_DIR  ${INSTALL_DIR:-/opt/mmt/5greplay}

RUN apt-get update && apt-get install --yes \
       git gcc make libxml2-dev libpcap-dev libconfuse-dev libsctp-dev

ADD .   ${INSTALL_DIR}/
WORKDIR ${INSTALL_DIR}

RUN dpkg -i lib/*.deb && ldconfig
RUN  make sample-rules


ENTRYPOINT ["./5greplay"]
# default parameter
CMD ["-h"]
