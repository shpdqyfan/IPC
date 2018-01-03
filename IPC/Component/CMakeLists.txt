cmake_minimum_required(VERSION 3.0)
project(component)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

message(------------------------------------------------)
find_package(Boost
             1.57.0
             COMPONENTS
             REQUIRED)

message(project_name: ${PROJECT_NAME})
message(project_src_dir: ${PROJECT_SOURCE_DIR})
message(project_bin_dir: ${PROJECT_BINARY_DIR})

add_subdirectory(Thread)
add_subdirectory(Buffer)
add_subdirectory(IpcMsg)

#reference the objects using an expression of the form $<TARGET_OBJECTS:objlib> as a source
add_library(${PROJECT_NAME} 
            SHARED
            $<TARGET_OBJECTS:thread>
            $<TARGET_OBJECTS:buffer>
            $<TARGET_OBJECTS:ipcmsg>)

#install(TARGETS ${PROJECT_NAME} DESTINATION ../build/bin)

