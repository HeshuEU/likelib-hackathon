## LikeLib 2.0
To run the node, a configuration file must be specified for it.
By default it is config.json file of the following format:

```
{
    "listen_address": "0.0.0.0:20203",
    "public_server_port": 20203,
    "nodes": [
        "127.0.0.1:20204",
        "127.0.0.1:20205"
    ]
}
```

Notes on parameters:
* `listen_address` - specifies local address and port that the server will listen on
* `public_server_port` - when node is connected to a remote machine over Internet, its 
public IP gets known, but port - doesn't. We only know the client-socket IP address.
Such things as port-forwarding with NAT, may change the port we need to connect to
* `nodes` - list of known nodes

### Build
1. Go to ./doc folder: right now it is a staring point for scripts.
2. Run ./prepare_build.sh. It will install vcpkg packet manager to /opt folder. It may
require sudo permissions.
3. Run ./build.sh. It will execute CMake, that will generate makefiles in ./build folder.
4. Go to ./build folder and run make. 
