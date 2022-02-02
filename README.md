# binlab

Binary Laboratory

## Building binlab

### Generating a Buildsystem:

```
cmake -S binlab -B build [options]
```

- `-DCMAKE_BUILD_TYPE=type` Multi-configuration buildsystems (i.e. Visual Studio, Xcode) ignore this setting. Valid options are: Debug(default), Release, RelWithDebInfo, and MinSizeRel.
- `-DCMAKE_INSTALL_PREFIX=directory` Specify for full pathname of where you want the binlab to be installed (default /usr/local).

### Invoking the Buildsystem

```
cmake --build build [options]
```

- `--target <target>` The default target (i.e. cmake --build . or make) will build all of binlab.
- `--config <config>` Specify which configuration to build (for Multi-configuration buildsystems).

### Software Installation

```
cmake --build build --target install
```

