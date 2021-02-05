rm -rf libwebp-1.1.0
tar -xf libwebp-1.1.0.tar.gz
cd libwebp-1.1.0
./configure --prefix=$PREFIX
make install CFLAGS='-O2 -mstackrealign'
