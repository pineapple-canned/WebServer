#include <iostream>
#include "ThreadTcpServer.h"

int main(int argc,char *argv[]){
    ThreadTcpServer tts(8080);
    tts.start_Server();

    

    return 0;
}