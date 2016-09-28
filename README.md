# minirpc
This is mini rpc depend on google protobuf.
#1. build
1.1. build gtest/protobuf source code for unittest.
###cd minirpc/tools
###unzip gtest-1.7.0.zip
###cd gtest-1.7.0
###./configure
###make

###cd ..
###tar -xvf protobuf-2.6.1.tar.gz
###cd protobuf-2.6.1
###./configure --prefix=`pwd`/inbin
###make ; make install

##1.2. build common , client, and agent directory code.
###   cd common
###   make clean; make
###cd ../client
###make clean ; make
###cd ../agent
###make clean; make
