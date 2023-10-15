cmake_minimum_required(VERSION 3.0)

project(EChat) 

# add_definitions("")
link_directories(/usr/local/lib)
link_directories(/usr/lib64/mysql)

# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_FLAGS "-std=c++11 -O1")

include_directories(include)

add_subdirectory(src)

add_executable(EChat main.cpp) 

target_link_libraries(EChat echatlib)




