This is panorama image display demo using EGL zero-copy technology. 

Build method:
1、 cd build
2、 cmake -D CMAKE_TOOLCHAIN_FILE=${your buildroot toolchain file} ..
    for example: 
    cmake -D CMAKE_TOOLCHAIN_FILE="home/px3-se/buildroot/output/host/usr/share/buildroot/toolchainfile.cmake" ..
3、 make
