/*
    Date: 2017/12/26
    Author: shpdqyfan
    profile: Process which will be used for sending/receiving msg to/from 
    main process "ControlProcess" in Ipc project.
*/

#include <iostream>

#include "Thread/Thread.h"
#include "Buffer/Buffer.h"
#include "IpcMsg/IpcMsg.h"

int main()
{
    std::cout<<"Dc process start"<<std::endl;

    //start Buffer to polling request from ControlProcess
    //get request from Buffer
    //process request
    //call IpcMsg send response to ControlProcess
    //clear request
    
    return 0;
}


