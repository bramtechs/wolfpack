project(wolfpack C CXX)

add_executable(${PROJECT_NAME}
    wolfpack_cli.cpp
    utils.cpp
    utils.hpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt cxxopts nlohmann_json::nlohmann_json tl::expected)

install(TARGETS ${PROJECT_NAME} RUNTIME)