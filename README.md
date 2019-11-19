## LikeLib 2.0
To run the node, a configuration file must be specified for it.
By default it is config.json file of the following format:

```
{
    "listen_address": "0.0.0.0:20203",
    "rpc_address": "0.0.0.0:50051"
    "nodes": [
        "127.0.0.1:20204",
        "127.0.0.1:20205"
    ]
}
```

### Build
1. Go to ./doc folder: right now it is a staring point for scripts.
2. Run ./prepare_build.sh. It will install vcpkg packet manager to user home folder. Note: run
this script without sudo permissions. This script also creates "zmake" function in ~/.bashrc file.
To find more detailed information about the usage of this script, use ./prepare_build.sh --help. 
3. Run ./build.sh. It executes CMake, that will generate makefiles in ../build folder. Or use
 zmake in build folder without options.
4. Go to ./build folder and run make. 
