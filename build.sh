mkdir build
cd build
CXX_FLAGS="-g" CXX=clang++ CC=clang cmake .. -DCMAKE_BUILD_TYPE=Debug -G Ninja -DOWCA_SCRIPT_TESTING=ON
ninja

