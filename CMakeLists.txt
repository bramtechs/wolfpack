cmake_minimum_required(VERSION 3.5)
project(wolfpack_root C CXX)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.0.2
  GIT_SHALLOW 1
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG v3.2.1
  GIT_SHALLOW 1
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(cxxopts)

add_subdirectory(wolfpack)
