FROM ubuntu:18.04

LABEL maintainer="Montimage <contact@montimage.com>"

ENV INSTALL_DIR  ${INSTALL_DIR:-/opt/mmt/5greplay}

RUN apt-get update && apt-get install --yes \
       git gcc make libxml2-dev libpcap-dev libconfuse-dev libsctp-dev

ADD .   ${INSTALL_DIR}/
WORKDIR ${INSTALL_DIR}

# Install DPI from source
RUN rm -rf mmt-dpi
RUN git clone --depth 1 https://github.com/Montimage/mmt-dpi.git \
         && cd mmt-dpi/sdk                                       \
         && make -j2                                             \
         && make install && ldconfig                             \
         && cd ../../ && rm -rf mmt-dpi

RUN  make sample-rules


ENTRYPOINT ["./5greplay"]
# default parameter
CMD ["-h"]
