# abrite-pos
## Build `abrite-pos`
From the top of the `abrite-pos` repo execute the following
```
cmake .. && cmake --build . && ./abrite-pos
```

## Install Google-Test
```
sudo apt install libgtest-dev
```

## Install MongoDB
```
sudo apt install gnupg curl
```
```
curl -fsSL https://www.mongodb.org/static/pgp/server-8.0.asc | \
   sudo gpg -o /usr/share/keyrings/mongodb-server-8.0.gpg \
   --dearmor
```
```
echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-8.0.gpg ] https://repo.mongodb.org/apt/ubuntu noble/mongodb-org/8.0 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-8.0.list
```
```
sudo apt update && sudo apt upgrade
```
```
sudo apt-get install -y mongodb-org
```
```
sudo systemctl enable mongod.service
```
```
sudo systemctl start mongod.service
```
If the `start` fails, execute the following, then try again
```
sudo systemctl daemon-reload
```

## Install the MongoDB C++ Driver (mongo-cxx-driver)
```
sudo apt install cmake pkg-config gcc g++ libssl-dev
```
```
cd /
```
```
sudo curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r4.0.0/mongo-cxx-driver-r4.0.0.tar.gz
```
```
sudo tar -xzf mongo-cxx-driver-r4.0.0.tar.gz
```
```
cd mongo-cxx-driver-r4.0.0/build
```
```
cmake ..                                \
    -DCMAKE_BUILD_TYPE=Release          \
    -DCMAKE_CXX_STANDARD=17
```
```
cmake --build .
```
```
sudo cmake --build . --target install
```
```
sudo rm -rf mongo-cxx-driver-r4.0.0 mongo-cxx-driver-r4.0.0.tar.gz
```
Execute the following to verify the installation
```
ls /usr/local/lib/mongocxx/ | grep mongocxx
ls /usr/local/lib | grep bsoncxx
ls /usr/local/include/mongocxx
ls /usr/local/include/bsoncxx
```