# IPC

1.Folder structure of my IPC project.
IPC
|── Component
│   |── CMakeLists.txt                    <--- Compile Component target  
│   |—— include                           <--- Expose the interface files of Component 
|   |—— Buffer
|   |   |—— CMakeLists.txt                <--- Compile Buffer target within Component
|   |—— Thread
|   |   |—— CMakeLists.txt                <--- Compile Thread target within Component
|   |—— IpcMsg
|   |   |—— CMakeLists.txt                <--- Compile IpcMsg target within Component

|—— ControlProc
│   ├── CMakeLists.txt                    <--- Compile ControlProc target

|—— DataCenterProc
│   ├── CMakeLists.txt                    <--- Compile DataCenterProc target

2.
