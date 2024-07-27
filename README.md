# wolfpack

Simple async vcpkg-like package manager for CMake C++.

Created to stop me from swearing while using existing C++ package managers.

## Install

```shell
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
      "tag": "11.0.2"
    }
  }
}
```

Tag can also be set to a branch, like 'main' or 'master'.

wolfpack will clone repos to the `.wolfpack` directory. You should add it to your `.gitignore` file.

```shell
echo .wolfpack >> .gitignore
```
Finally, add to your project's CMakeLists.txt.

```cmake
add_subdirectory(.wolfpack/fmtlib/fmt)

# (add_executable or add_library)

target_link_libraries(${PROJECT_NAME} PRIVATE fmt)
```

## Usage

```sh
wolfpack && cmake ...
```

## Limitations

- Source builds of dependencies only
- Dependencies must use CMake.
- Repos must be hosted on Github. **TODO: allow any git address**