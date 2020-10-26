rm -rf libwebp-0.5.2
tar -xf libwebp-0.5.2.tar.gz
cd libwebp-0.5.2
./configure --prefix=$PREFIX
make install CFLAGS='-O2 -mstackrealign'
