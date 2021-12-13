FROM ubuntu:18.04

LABEL maintainer="Montimage <contact@montimage.com>"

ENV INSTALL_DIR  ${INSTALL_DIR:-/opt/mmt/5greplay}

RUN apt-get update && apt-get install --yes \
       libxml2-dev \
       libpcap-dev \
       libsasl2-2 \
       gcc

ADD ./* $INSTALL_DIR/

WORKDIR $INSTALL_DIR
ENTRYPOINT ["5greplay"]
