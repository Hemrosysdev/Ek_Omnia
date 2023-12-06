############################################################
# Dockerfile to provide Jenkins Build Environment
# Based on Ubuntu
############################################################

# Set the base image
FROM ubuntu:20.04

# Avoid questions during apt-get command
ARG DEBIAN_FRONTEND=noninteractive

# Prequisites for Espressif
RUN apt-get update
RUN apt-get install -y build-essential
RUN apt-get install -y git

# for demonstrator Qt5 desktop
RUN apt-get install -y dpkg-dev
RUN apt-get install -y devscripts
RUN apt-get install -y qt5-default
RUN apt-get install -y qt5-qmake
RUN apt-get install -y qtbase5-dev-tools
RUN apt-get install -y libqt5serialport5-dev
RUN apt-get install -y qtdeclarative5-dev
RUN apt-get install -y libqt5quickshapes5
RUN apt-get install -y libqt5quickcontrols2-5
RUN apt-get install -y libqt5quicktemplates2-5
RUN apt-get install -y qml-module-qtquick-controls2
RUN apt-get install -y qml-module-qtqml-models2
RUN apt-get install -y qml-module-qtquick-shapes
RUN apt-get install -y qml-module-qtquick-controls
RUN apt-get install -y qml-module-qtgraphicaleffects

# For SDK installation we need /opt to be writable
RUN mkdir -p /opt
RUN chmod 777 /opt

COPY sdk_HEM_EKX.mmia7.140.tgz .

ENV HOME /home/jenkins
RUN groupadd -g 10000 jenkins
RUN useradd -c "Jenkins user" -d $HOME -u 10000 -g 10000 -m jenkins
RUN git config --global user.name "Jenkins" && git config --global user.email "jenkins@u-experten.de"
USER jenkins

RUN tar -xzf sdk_HEM_EKX.mmia7.140.tgz -C /
