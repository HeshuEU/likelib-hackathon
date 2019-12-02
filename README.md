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
1. Go to ./doc folder.
2. Run: sudo ./prepare_build.sh . It will install vcpkg package manager to /opt folder.
3. Restart terminal.
4. To generate CMake files in current directory simply run `lkgen` command. It will
search for CMakeLists in current directory and its parent, and will generate CMake
files. If current and parent folders don't contain CMakeLists.txt, then the path to
the root of the project, from which ./prepare_build.sh was run, will be used to
get CMakeLists.
