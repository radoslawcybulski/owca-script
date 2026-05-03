mkdir -p build
cd build
CXX=clang++ CC=clang cmake .. -DCMAKE_BUILD_TYPE=${MODE:-Release} -G Ninja -DOWCA_SCRIPT_TESTING=ON
mv compile_commands.json ..
ninja

