FROM ubuntu:18.04

LABEL maintainer="Montimage <contact@montimage.com>"

ENV INSTALL_DIR  ${INSTALL_DIR:-/opt/mmt/5greplay}

RUN apt-get update && apt-get install --yes \
       git gcc make libxml2-dev libpcap-dev libconfuse-dev libsctp-dev

ADD .   ${INSTALL_DIR}/
WORKDIR ${INSTALL_DIR}

#ADD https://github.com/Montimage/mmt-dpi/releases/download/v1.7.4/mmt-dpi_1.7.4_c5a4a6b_Linux_x86_64.deb mmt-dpi.deb
RUN dpkg -i lib/mmt-dpi*.deb && ldconfig
RUN  make sample-rules


ENTRYPOINT ["./5greplay"]
# default parameter
CMD ["-h"]
