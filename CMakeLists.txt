cmake_minimum_required(VERSION 3.25)
project(dhome VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dhome src/main.cpp)

find_package(yaml-cpp REQUIRED)
target_link_libraries(dhome yaml-cpp ncurses)

find_package(X11 REQUIRED)
target_link_libraries(dhome X11::X11 X11::Xft)

target_include_directories(dhome PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util
    ${CMAKE_CURRENT_SOURCE_DIR}/src/net
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ai
    ${CMAKE_CURRENT_SOURCE_DIR}/src/device
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
)

install(TARGETS dhome DESTINATION bin)
install(FILES deploy/dhome.service DESTINATION lib/systemd/system)