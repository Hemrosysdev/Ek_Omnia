############################################################
# Dockerfile to provide Jenkins Build Environment
# Based on Ubuntu
############################################################

# Set the base image
FROM ubuntu:18.04

# Avoid questions during apt-get command
ARG DEBIAN_FRONTEND=noninteractive

# Prequisites for Espressif
RUN apt-get update && apt-get install -y \
    git \
    wget \
    flex \
    bison \
    gperf \
    python3 \
    python3-pip \
    python3-setuptools \
    cmake \
    ninja-build \
    ccache \
    libffi-dev \
    libssl-dev \
    dfu-util \
    libusb-1.0-0 \
    zip

RUN mkdir -p /opt/esp
RUN chmod 777 /opt/esp

ENV HOME /home/jenkins
RUN groupadd -g 10000 jenkins
RUN useradd -c "Jenkins user" -d $HOME -u 10000 -g 10000 -m jenkins
RUN git config --global user.name "Jenkins" && git config --global user.email "jenkins@u-experten.de"
USER jenkins

# For SDK installation we need /opt to be writable

RUN cd /opt/esp;git clone -b v4.4.3 --single-branch --recursive https://github.com/espressif/esp-idf.git esp-idf-v4.4.3
RUN cd /opt/esp/esp-idf-v4.4.3;./install.sh

