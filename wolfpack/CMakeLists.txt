project(wolfpack C CXX)

add_executable(${PROJECT_NAME}
    wolfpack_cli.cpp
    utils.cpp
    utils.hpp
    json_config_reader.cpp
	json_config_reader.hpp
    config_reader.cpp
    config_reader.hpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    fmt
    cxxopts
    tl::expected
)

install(TARGETS ${PROJECT_NAME} RUNTIME)
