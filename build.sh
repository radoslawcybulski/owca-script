mkdir build
cd build
CXX=clang++ CC=clang cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja -DOWCA_SCRIPT_TESTING=ON
ninja

