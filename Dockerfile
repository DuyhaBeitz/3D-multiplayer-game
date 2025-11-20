FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    git \
    build-essential \
    pkg-config \
    libx11-dev \
    libxi-dev \
    libxrandr-dev \
    libgl1-mesa-dev \
    libasound2-dev \
    libxcursor-dev \
    libxinerama-dev \
    libwayland-dev \
    wayland-protocols \
    libglfw3-dev \
    && rm -rf /var/lib/apt/lists/*

ADD https://cmake.org/files/v3.30/cmake-3.30.0-linux-x86_64.sh /cmake-3.30.0-linux-x86_64.sh
RUN mkdir /opt/cmake
RUN sh /cmake-3.30.0-linux-x86_64.sh --prefix=/opt/cmake --skip-license
RUN ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
RUN cmake --version

RUN apt update &&\
    apt install -y libfmt-dev zlib1g-dev

EXPOSE 7777