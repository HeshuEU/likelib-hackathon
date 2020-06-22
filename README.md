# LikeLib 2.0

To run the node, a configuration file must be specified for it.
By default it is config.json file of the following format:

```
{
    "net": {
        "listen_addr": "0.0.0.0:20203",
        "public_port": 20203,
        "peers_db": "likelib/peers"
    },
    "web_socket": {
        "listen_addr": "0.0.0.0:50051"
    },
    "miner": {
        "threads": 4
    },
    "nodes": [
        "127.0.0.1:20204",
        "127.0.0.1:20205"
    ],
    "keys_dir": ".",
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
* `net.peers_db` - folder, in which peers database will be stored;
* `rpc.grpc_address` - address on which RPC (GRPC) is listening on. Enabled when the field is present;
* `rpc.http_address` - address on which RPC (HTTP) is listening on. Enabled when the field is present;
* `miner.threads` - optional parameter, sets the number of threads that miner is using;
* `nodes` - list of known nodes.
* `keys_dir` - key(public and private that was generated by client) folder path. 
if file not exists generate new key pair and save by this path.
* `database.path` - path to folder with database files (will be created if not exists).
* `database.clean` - if true - cleans database; otherwise does nothing.


## Build
1. Run: `$ sudo .doc/prepare_build.sh`. It will install conan and its dependencies.
2. To generate CMake files use `cmake` command. 
3. To build project use `make` command.
4. Go to the bin folder and create `config.json` with the node settings in it.
5. Run `node` executable. Keys for the node will be created automatically. If you want to use 
already existing ones - put your key in `node` executable folder with the name `lkkey`.
