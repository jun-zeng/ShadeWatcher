# Parser Setup
We implement the audit log parser using C++. The setup has been tested using g++ 8.4.0. The required packages are as follow:

1. Installation Path: "LIB_INSTALL_PATH"
```bash
cd lib
LIB_INSTALL_PATH=$PWD
```

2. g++ (optional)
```bash
wget https://ftp.gnu.org/gnu/gcc/gcc-8.4.0/gcc-8.4.0.tar.gz
tar xzvf gcc-8.4.0.tar.gz
cd gcc-8.4.0
contrib/download_prerequisites
./configure -v --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu --prefix=$LIB_INSTALL_PATH/lib -enable-checking=release --enable-languages=c,c++,fortran --disable-multilib
make -j8
make install
cd ..
```

3. neo4j
```bash
sudo add-apt-repository ppa:cleishm/neo4j
sudo apt-get update
sudo apt-get install libssl-dev neo4j-client libneo4j-client-dev
```

4. libpqxx (c++ postgresql)
```bash
sudo apt install libpq-dev
git clone https://github.com/jtv/libpqxx.git
cd libpqxx
git checkout master
./configure --disable-documentation --prefix=$LIB_INSTALL_PATH
make -j8
make install
cd ..
```

5. libconfig
```bash
wget https://hyperrealm.github.io/libconfig/dist/libconfig-1.7.2.tar.gz
tar xzvf libconfig-1.7.2.tar.gz
cd libconfig-1.7.2/
./configure --prefix=$LIB_INSTALL_PATH
make -j8
make install
cd ../
```

6. jsoncpp
```bash
sudo apt-get install libjsoncpp-dev
```

7. nlohmann json
```bash
cd $LIB_INSTALL_PATH/include
wget https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
cd ../
```

8. librdkafka
```bash
git clone https://github.com/edenhill/librdkafka.git
cd librdkafka/
# Install disabled lib (e.g., zlib) in ./configure.
./configure --prefix=$LIB_INSTALL_PATH
make -j8
make install
cd ../
```

Setup system system library path (Optional)
```bash
echo export CPLUS_INCLUDE_PATH=$LIB_INSTALL_PATH/include:$CPLUS_INCLUDE_PATH >> ~/.bashrc
echo export PATH=$LIB_INSTALL_PATH/bin:$PATH >> ~/.bashrc
echo export LD_LIBRARY_PATH=$LIB_INSTALL_PATH/lib:$LIB_INSTALL_PATH/lib64:$LD_LIBRARY_PATH >> ~/.bashrc
source ~/.bashrc
```

# [Kafka setup](kafka_setup.md) (optional)

# [Postgresql setup](postgresql_setup.md) (optional)

# [Neo4j setup](https://datawookie.dev/blog/2016/09/installing-neo4j-on-ubuntu-16.04) (optional)
