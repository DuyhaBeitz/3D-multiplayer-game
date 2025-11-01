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

RUN git clone https://github.com/raysan5/raylib.git && \
    cd raylib && \
    mkdir build && cd build && \
    cmake \
        -DBUILD_SHARED_LIBS=ON \
        -DSUPPORT_DRM=OFF \
        .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig

RUN git clone https://github.com/DuyhaBeitz/RaylibRetainedGUI.git &&\
    cd RaylibRetainedGUI &&\
    ./rebuild.sh &&\
    ./install.sh

RUN apt update &&\
    apt install -y libfmt-dev zlib1g-dev

RUN git clone https://github.com/DuyhaBeitz/EasyNet.git &&\
    cd EasyNet &&\
    ./rebuild.sh &&\
    ./install.sh


COPY . .

RUN ./rebuild.sh


EXPOSE 7777
#CMD ["build/server"]

