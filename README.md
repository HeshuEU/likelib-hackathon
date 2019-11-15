## LikeLib 2.0
To run the node, a configuration file must be specified for it.
By default it is config.json file of the following format:

```
{
    "listen_address": "0.0.0.0:20203"
}
```

### Build
1. Go to ./doc folder: right now it is a staring point for scripts.
2. Run ./prepare_build.sh. It will install vcpkg packet manager to /opt folder. It may
require sudo permissions.
3. Run ./build.sh. It will execute CMake, that will generate makefiles in ./build folder.
4. Go to ./build folder and run make. 
