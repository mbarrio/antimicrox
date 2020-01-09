FROM ubuntu:bionic
ARG USER=docker
ARG UID=1000
ARG GID=1000

MAINTAINER Jagoda Górska <juliagoda.pl@protonmail.com>


RUN apt-get -y update && apt-get install -y \
    g++ \
    make \
    build-essential \
    git \
    mesa-utils \
    libgl1-mesa-glx \
    libgl1-mesa-dri \
    gettext \
    autoconf \
    pkg-config \
    cmake \
    extra-cmake-modules \
    libtool \
    curl \
    libsdl2-dev \
    qttools5-dev \
    qttools5-dev-tools \
    libxi-dev \
    libxtst-dev \
    libx11-dev \
    libxrender-dev \
    libxext-dev \
    itstool \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* \
    && apt-get autoremove -y



RUN git clone https://github.com/juliagoda/antimicroX.git && cd antimicroX

RUN addgroup --gid ${GID} ${USER} && \
    adduser --disabled-password --gecos '' --uid ${UID} --gid ${GID} ${USER} && \
    usermod -a -G input ${USER} && \
    usermod -a -G uucp ${USER} && \
    usermod -a -G tty ${USER} && \
    usermod -a -G games ${USER} 
  

      
COPY . /opt

WORKDIR /opt

# finally build project from github
RUN cmake -DCMAKE_INSTALL_PREFIX=/usr . && \ 
    make && \ 
    make install
    

USER ${UID}:${GID}

WORKDIR /home/${USER}

RUN mkdir -p /home/${USER}/.config/antimicroX

CMD /usr/bin/antimicroX