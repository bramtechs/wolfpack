# wolfpack

Simple and async vcpkg-like package manager for downloading CMake dependencies.

## Install

```shell
git clone https://github.com/bramtechs/wolfpack
cd wolfpack
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=.
cmake --build build --parallel --target install
```

Now add the generated ```bin``` folder to your $PATH.

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

By default, dependencies are pulled from Github. You can specify the repo address explicitly.

```json
{
  "version": 1,
  "libs": {
    "fmtlib/fmt": {
      "tag": "11.0.2",
	  "url": "bram@nas:/volume1/git/third-party/fmtlib/fmt"
    }
  }
}
```

Tag can also be set to a branch (like 'main' or 'master') or a commit id.

Finally, add to your project's CMakeLists.txt.

```cmake
add_subdirectory(.wolfpack/fmtlib/fmt)

# ... snip ...

target_link_libraries(${PROJECT_NAME} PRIVATE fmt)
```

wolfpack will clone repos to the `.wolfpack` directory, which you should add to your `.gitignore` file.

```shell
echo .wolfpack >> .gitignore
```

## Usage

```sh
wolfpack && cmake ...
```

To pull the latest versions of the dependencies:

```sh
wolfpack --pull && cmake ...
```

## Limitations

- Source builds of dependencies only
- Dependencies must use CMake.
