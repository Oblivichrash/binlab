# binlab

binlab (Binary Laboratory) is a project for learning binaries.

Binary usually referred to as executable file in computing, which encode machine code instructions and can be interpreted by a physical CPU directly. Binaries are system specific (such as Executable and Linkable Format (ELF) for Linux and Portable Executable (PE) for Windows), it is a mirror of the operating system in many ways.

Feel free to make your contributions to this project.

## Building and Testing

### Generating a Buildsystem:

```
cmake [options] -S binlab -B build
```

- `-DCMAKE_BUILD_TYPE=type` Multi-configuration buildsystems (i.e. Visual Studio, Xcode) ignore this setting. Valid options are: Debug(default), Release, RelWithDebInfo, and MinSizeRel.
- `-DCMAKE_INSTALL_PREFIX=directory` Specify for full pathname of where you want the binlab to be installed (default /usr/local).

### Invoking the Buildsystem

```
cmake --build build [options]
```

- `--target <target>` The default target (i.e. cmake --build . or make) will build all of binlab.
- `--config <config>` Specify which configuration to build (for Multi-configuration buildsystems).

### Testing

```
cd build
ctest [options]
```

- `--build-config <config>` Must be specified for multi-config generators (e.g. Visual Studio).
- `--extra-verbose` Enable more verbose output from tests.

### Installing

```
cmake --install build [options]
```

- `--prefix <directory>` Override `CMAKE_INSTALL_PREFIX`
