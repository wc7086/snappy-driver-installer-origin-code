rm -rf libwebp-0.6.1
tar -xf libwebp-0.6.1.tar.gz
cd libwebp-0.6.1
./configure --prefix=$PREFIX
make install CFLAGS='-O2 -mstackrealign'
