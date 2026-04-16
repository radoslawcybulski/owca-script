mkdir build
cd build
CMAKE_CXX_FLAGS=-g CXX=clang++ CC=clang cmake .. -DCMAKE_BUILD_TYPE=Debug -G Ninja -DOWCA_SCRIPT_TESTING=ON
ninja

