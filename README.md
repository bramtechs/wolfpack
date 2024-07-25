# wolfpack

Simple and async vcpkg-like package manager for CMake repos.

Created to stop me from swearing while using existing C++ package managers.

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

Create a `wolfpack.json` file next to your project's CMakeLists.txt.

```json
{
  "version": 1,
  "libs": {
    "fmtlib/fmt": {
      "tag": "11.0.2" # or "tag":"master"
    }
  }
}
```

wolfpack will clone repos to `$HOME/.wolfpack/<author>/<repo>` by default and share them between projects.

Finally, add to your project's CMakeLists.txt.

```cmake
add_subdirectory(${WOLF_PACK}/fmtlib/fmt)

# (add_executable or add_library)

target_link_libraries(${PROJECT_NAME} PRIVATE fmt)
```

## Usage

TODO: Use `wcmake` that wraps `cmake`.

```sh
wpcmake -S . -B build
```

If you don't want to use the wrapper, use this instead.

```sh
wolfpack && cmake -DWOLF_PACK="$HOME/.wolfpack" -S . -B build

# or if you don't want to share dependencies between projects
wolfpack -o ".wolfpack" && cmake -DWOLF_PACK=".wolfpack" -S . -B build
```

## Limitations

- Source builds only (cmake will get slow when using lots of dependencies)
- Dependencies must use CMake.
- Repos must be hosted on Github.
- Multiple wolfpack instances cannot be run simultanuously, unless custom DWOLF_PACK directory is set.
