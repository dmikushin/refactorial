set -e -x
cd /project
mkdir -p build-docker
cd build-docker
cmake -DCMAKE_BUILD_TYPE=Debug \
	-DLLVM_DIR=/usr/local/lib/cmake/llvm -DClang_DIR=/usr/local/lib/cmake/clang \
	-DCMAKE_CXX_COMPILER=g++-11 -DCMAKE_LINKER=mold -DCMAKE_CXX_FLAGS=-fuse-ld=mold \
       	-G Ninja .. && \
cmake --build .
