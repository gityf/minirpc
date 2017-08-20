# minirpc
  This is mini rpc depend on google protobuf.

## Intalling
  build gtest/protobuf source code for unittest.
```bash
cd minirpc/tools
unzip gtest-1.7.0.zip
cd gtest-1.7.0
./configure
make
```
  to build protobuf
```bash
tar -xvf protobuf-2.6.1.tar.gz
cd protobuf-2.6.1
./configure --prefix=`pwd`/inbin
make ; make install
```

  build and install common , client, and agent directory code.
```bash
cd common
make clean; make
cd ../client
make clean ; make
cd ../agent
make clean; make
```
