# LikeLib 2.0

To run the node, a configuration file must be specified for it.
By default it is config.json file of the following format:

```
{
    "net": {
        "listen_addr": "0.0.0.0:20203",
        "public_port": 20203
    },
    "rpc": {
        "address": "0.0.0.0:50051"
    },
    "miner": {
        "threads": 4
    },
    "nodes": [
        "127.0.0.1:20204",
        "127.0.0.1:20205"
    ],
    "keys": {
        "public_path": "rsa.pub",
        "private_path": "rsa"
    }
    "database": {
        "path": "likelib/database",
        "clean": false
    }
}
```

Notes on parameters:
* `net.listen_addr` - specifies local address and port that the server will listen on;
* `net.public_port` - when node is connected to a remote machine over Internet, its 
public IP gets known, but port - doesn't. We only know the client-socket IP address.
Such things as port-forwarding with NAT, may change the port we need to connect to;
* `rpc.address` - address on which RPC is listening on;
* `miner.threads` - optional parameter, sets the number of threads that miner is using;
* `nodes` - list of known nodes.
* `keys.public_path` - public key file path. if file not exists generate new key pair and save by this path.
* `keys.private_path` - private key file path. if file not exists generate new key pair and save by this path.
* `database.path` - path to folder with database files (will be created if not exists).
* `database.clean` - if true - cleans database; otherwise does nothing.

---

Client json config file format:

```
{
    "nodes": [
        "localhost:50051"
    ],
    "addresses": [
        "00000000000000000000000000000000",
        "11111111111111111111111111111111"
    ]
}
```

Notes on parameters:
* `addresses` - list of known addresses
* `nodes` - list of known nodes

## Build
1. Go to ./doc folder.
2. Run: sudo ./prepare_build.sh . It will install vcpkg package manager to /opt folder.
3. Restart terminal.
4. To generate CMake files in current directory simply run `lkgen` command. It will
search for CMakeLists in current directory and its parent, and will generate CMake
files. If current and parent folders don't contain CMakeLists.txt, then the path to
the root of the project, from which ./prepare_build.sh was run, will be used to
get CMakeLists.
5. install solidity compiler:
```
    sudo add-apt-repository ppa:ethereum/ethereum
    sudo apt-get update
    sudo apt-get install solc
```