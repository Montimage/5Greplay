FROM ubuntu:18.04

LABEL maintainer="Montimage <contact@montimage.com>"

ENV INSTALL_DIR  ${INSTALL_DIR:-/opt/mmt/5greplay}

RUN apt-get update && apt-get install --yes \
       libxml2-dev \
       libpcap-dev \
       libsasl2-2 \
       gcc

COPY 5greplay*.tar.gz /tmp/package.tar.gz

RUN echo "Install 5Greplay" && \
        mkdir -p $INSTALL_DIR && \
        tar -xzf /tmp/package.tar.gz -C $INSTALL_DIR && \
        rm /tmp/package.tar.gz

WORKDIR $INSTALL_DIR
ENTRYPOINT ["5greplay"]
