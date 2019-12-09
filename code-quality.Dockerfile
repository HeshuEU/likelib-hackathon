# Create docker environment for code cuality
FROM likelib2_build

WORKDIR /project/src

RUN clang-tidy node/main.cpp -- -I/opt/vcpkg/installed/x64-linux/include -I${PWD} -I/build/grpc/gen -std=c++17
