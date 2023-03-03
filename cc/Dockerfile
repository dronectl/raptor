FROM ubuntu:latest

WORKDIR /app
COPY . .

RUN apt-get update && \
    apt-get upgrade -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y gcc-arm-none-eabi git cmake openocd

RUN mkdir build && \
    cd build &&\
    cmake .. -DCMAKE_BUILD_TYPE=Release &&\
    make -j`nproc`
