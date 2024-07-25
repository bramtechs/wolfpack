project(wolfpack C CXX)

add_executable(${PROJECT_NAME}
    wolfpack_cli.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt cxxopts)