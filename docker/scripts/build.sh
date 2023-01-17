set -e -x
cd /project
mkdir -p build-docker
cd build-docker
cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja .. && \
cmake --build .
