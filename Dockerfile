FROM debian:bookworm-slim

WORKDIR /opt

ENV ARM_COMPILER_VERSION=14.3.rel1
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        git \
        xz-utils \
        wget \
        make \
        python3 \
        python3-venv \
        python3-pip \
        sudo \
        libjim-dev \
        libtool \
        pkg-config \
        autoconf \
        automake \
        xxd

RUN ARCH=$(uname -m) && \
    wget -O toolchain.tar.xz "https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_COMPILER_VERSION}/binrel/arm-gnu-toolchain-${ARM_COMPILER_VERSION}-${ARCH}-arm-none-eabi.tar.xz" && \
    tar xf toolchain.tar.xz && \
    rm -f toolchain.tar.xz

COPY requirements.txt /opt/requirements.txt
COPY zelda3/requirements.txt /opt/zelda3/requirements.txt

RUN python3 -m venv /opt/venv && \
    /opt/venv/bin/pip install --upgrade pip && \
    /opt/venv/bin/pip install -r /opt/requirements.txt

RUN useradd -m docker && echo "docker:docker" | chpasswd && \
    chown docker:docker /opt && \
    echo "docker ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

RUN echo 'export PATH="/opt/venv/bin:$PATH"' >> /etc/profile

RUN ARCH=$(uname -m) && \
    echo "export GCC_PATH=/opt/arm-gnu-toolchain-${ARM_COMPILER_VERSION}-${ARCH}-arm-none-eabi/bin" >> /etc/profile


# Install openocd nightly
RUN git clone https://github.com/sylverb/ubuntu-openocd-git-builder.git && \
    cd ubuntu-openocd-git-builder && \
    ./build.sh && \
    sudo dpkg -i openocd-git_*.deb && \
    cd .. && \
    rm -rf ubuntu-openocd-git-builder && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

USER docker

ENV OPENOCD="/opt/openocd-git/bin/openocd"

WORKDIR /opt/workdir

CMD ["/bin/bash", "-l"]