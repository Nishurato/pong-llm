cmake_minimum_required(VERSION 3.20)

project(Pong LANGUAGES CXX)

add_executable(Pong WIN32
    src/main.cpp
    src/Application.cpp
    src/Game.cpp
    src/Renderer.cpp
)

target_compile_features(Pong PRIVATE cxx_std_20)
target_compile_definitions(Pong PRIVATE UNICODE _UNICODE)
target_link_libraries(Pong PRIVATE user32 d2d1 dwrite)
