FROM ubuntu:latest
LABEL authors="mguigue"

RUN apt update
RUN apt install -y build-essential &&\
    apt install -y cmake git python-dev-is-python3 pkg-config

COPY . /opt/pyfasterac-src

RUN mkdir -p /opt/pyfasterac-src/build /opt/pyfasterac
WORKDIR /opt/pyfasterac-src/build
RUN cmake ../ -D CMAKE_INSTALL_PREFIX=/opt/pyfasterac && \
    make -j4 &&\
    make install
