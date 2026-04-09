#include <iostream>
#include "ProcessTcpServer.h"


int main(int argc,char *argv[]){
    ProcessTcpServer pts(8080);
    pts.start();


    return 0;
}