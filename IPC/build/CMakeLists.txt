cmake_minimum_required(VERSION 3.0)
project(IPC)

message(------------------------------------------------)
message(project_name: ${PROJECT_NAME})
message(project_src_dir: ${PROJECT_SOURCE_DIR})
message(project_bin_dir: ${PROJECT_BINARY_DIR})

add_subdirectory(../Component bin/com)
add_subdirectory(../ControlProc bin/cp)
add_subdirectory(../DataCenterProc bin/dcp)

