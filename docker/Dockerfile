FROM clang-debian10:12

LABEL maintainer="dmitry@kernelgen.org"

ENV DEBIAN_FRONTEND noninteractive
ENV LC_ALL C.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

RUN apt-get update && \
	apt-get -y --no-install-recommends install \
		cmake \
		ninja-build \
		libtbb-dev \
		libpcre3-dev && \
	apt-get clean

COPY . /root/

WORKDIR /root

