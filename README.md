# binlab

Executable Binary Laboratory

## Building and Testing binlab

### Generating a Buildsystem:

```
cmake [options] -S binlab -B build
cd build
```

- `-DCMAKE_BUILD_TYPE=type` Multi-configuration buildsystems (i.e. Visual Studio, Xcode) ignore this setting. Valid options are: Debug(default), Release, RelWithDebInfo, and MinSizeRel.
- `-DCMAKE_INSTALL_PREFIX=directory` Specify for full pathname of where you want the binlab to be installed (default /usr/local).

### Invoking the Buildsystem

```
cmake --build . [options]
```

- `--target <target>` The default target (i.e. cmake --build . or make) will build all of binlab.
- `--config <config>` Specify which configuration to build (for Multi-configuration buildsystems).

### Testing

```
ctest [options]
```

- `--build-config <config>` Must be specified for multi-config generators (e.g. Visual Studio).
- `--extra-verbose` Enable more verbose output from tests.

### Installing

```
cmake --install . [options]
```

- `--prefix <directory>` Override `CMAKE_INSTALL_PREFIX`
