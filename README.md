# wolfpack

NIH simple vcpkg-like package manager that downloads CMake repos.

Created to stop me from swearing when using existing C++ package managers.

## Install

```cmake
git clone https://github.com/bramtechs/wolfpack
cmake -S . -B build
cmake --build build --parallel
```

Now add to $PATH

```
TODO
```

## Configuration

Create a `wolfpack.yml` or ``wolfpack.yaml` file next to your CMakeLists.txt

```yaml
- fmtlib/fmt
  - tag=11.0.2
```

wolfpack will clone repos to `$HOME/.wolfpack/<author>/<repo>` and use them between projects.

Finally add to your projects CMakeLists.txt

```cmake
add_subdirectory(${WOLF_PACK}/fmtlib/fmt)
```

## Usage

Use `wcmake` that wraps `cmake`.

```sh
wcmake -S . -B build
```

If you don't want to use the wrapper, use this instead.

```sh
wolfpack && cmake -DWOLF_PACK="$HOME/.wolfpack" -S . -B build
```

## Limitations

- Source builds only (cmake will get slow when using lots of dependencies)
- Dependencies must use CMake.
- Repos must be hosted on Github.
