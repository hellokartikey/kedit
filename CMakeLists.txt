cmake_minimum_required(VERSION 3.27.4)

project(kedit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Curses REQUIRED)
find_package(fmt REQUIRED)

add_executable(
  ${PROJECT_NAME}
  main.cc
  kedit.cc
)

target_link_libraries(
  ${PROJECT_NAME}
  ${CURSES_LIBRARIES}
  fmt::fmt
)
