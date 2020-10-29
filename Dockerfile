FROM ubuntu:18.04

RUN apt-get -y update
RUN apt-get -y install  g++-7=7.3.0-16ubuntu3 \
                        gcc-7=7.3.0-16ubuntu3 \
                        libstdc++-7-dev=7.3.0-16ubuntu3 \
                        gcc-7-base=7.3.0-16ubuntu3 \
                        cpp-7=7.3.0-16ubuntu3 \
                        libgcc-7-dev=7.3.0-16ubuntu3 \
                        libasan4=7.3.0-16ubuntu3 \
                        libubsan0=7.3.0-16ubuntu3 \
                        libcilkrts5=7.3.0-16ubuntu3
RUN apt-get -y install make
RUN apt-get -y install gcc gdb

RUN useradd -s /bin/bash -mN os

RUN mkdir /src

USER os

WORKDIR /src