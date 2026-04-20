mkdir build
cd build
CXX_FLAGS="-g -O2" CXX=clang++ CC=clang cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja -DOWCA_SCRIPT_TESTING=ON
ninja

