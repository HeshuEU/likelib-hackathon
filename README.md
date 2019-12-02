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
2. Run: sudo ./prepare_build.sh . It will install vcpkg packet manager to /opt folder.
3. Restart terminal session
4. Run: zmake
