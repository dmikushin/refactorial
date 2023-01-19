FROM clang12-ubuntu:jammy

LABEL maintainer="dmitry@kernelgen.org"

ENV DEBIAN_FRONTEND noninteractive
ENV LC_ALL C.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

RUN apt-get update && \
	apt-get -y --no-install-recommends install \
		cmake \
		git \
		ca-certificates \
		ninja-build \
		libtbb-dev \
		libpcre3-dev \
		zlib1g-dev \
		g++-11 && \
	apt-get clean

# Install mold as recommended here: https://dev-docs.kicad.org/en/build/linux/
RUN git clone https://github.com/rui314/mold.git && \
	mkdir mold/build && \
	cd mold/build && \
	git checkout v1.7.1 && \
	../install-build-deps.sh && \
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-11 -G Ninja .. && \
	cmake --build . && \
	cmake --install .

COPY . /root/

WORKDIR /root

