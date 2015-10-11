rm -rf libwebp-0.4.3
tar -xf libwebp-0.4.3.tar.gz
cd libwebp-0.4.3
./configure --prefix=$PREFIX
make install CFLAGS='-O2 -mstackrealign'
