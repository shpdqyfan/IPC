cmake_minimum_required(VERSION 3.0)
project(DcProcess)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

message(------------------------------------------------)
find_package(Boost
             1.57.0
             COMPONENTS
             REQUIRED)

message(project_name: ${PROJECT_NAME})
message(project_src_dir: ${PROJECT_SOURCE_DIR})
message(project_bin_dir: ${PROJECT_BINARY_DIR})

set(APP_NAME DcProc.bin)
set(APP_SRC DcMain.cpp)
add_executable(${APP_NAME} ${APP_SRC})

target_link_libraries(${APP_NAME} component)

include_directories("../Component/include")

#install(TARGETS DcProc.bin DESTINATION ../build/bin)

