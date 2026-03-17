# syntax=docker/dockerfile:1

FROM debian:12 AS builder
ARG CMAKE_BUILD_TYPE=Asan
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    git \
    clang \
    cmake \
    ninja-build

WORKDIR /root/src
COPY CMakeLists.txt CMakeLists.txt
COPY cmake cmake
COPY test test
COPY lib lib
COPY app app
COPY LICENSE LICENSE

WORKDIR build
RUN cmake \
  -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
  -DUNTANGLE_TEST=ON \
  -G Ninja \
  .. && cmake --build . -j$(nproc)

# Disable crash reporting.
# We have tests that are expected exit with SIGTRAP, and crash reporting will slow these down.
RUN echo -e 'Storage=none\nProcessSizeMax=0' > /etc/systemd/coredump.conf

RUN time ASAN_OPTIONS=detect_invalid_join=0 ctest -j$(nproc) --output-on-failure


