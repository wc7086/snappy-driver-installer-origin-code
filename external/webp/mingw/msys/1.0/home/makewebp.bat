tar -xf libwebp-0.4.3.tar.gz
cd libwebp-0.4.3
./configure --prefix=/mingw
make install CFLAGS='-O2 -g -mstackrealign'
