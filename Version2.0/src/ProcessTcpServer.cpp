#include "ProcessTcpServer.h"
#include <iostream>
#include <string.h>
// 网络编程头文件
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// 多进程头文件
#include <unistd.h>
#include <sys/wait.h>


ProcessTcpServer::ProcessTcpServer(const uint16_t port)
: port_(port),listen_fd_(-1) {
}



ProcessTcpServer::~ProcessTcpServer(){
    if(listen_fd_ > 0){
        // 关闭服务端
        close(listen_fd_);
        std::cout << "服务端已经关闭!" << std::endl;
    }
}



// 初始化服务端
bool ProcessTcpServer::initServer(){
    listen_fd_ = socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd_ == -1){
        perror("socket failed!");
        return false;
    }

    // 端口复用开启
    int opt = 1;
    // 开启 SO_REUSEADDR
    int ret = setsockopt(
        listen_fd_,                      // 要配置的 socket 文件描述符
        SOL_SOCKET,                     // 配置级别：套接字层面
        SO_REUSEADDR | SO_REUSEPORT,    // 端口复用选项
        &opt,                           // 配置值：开启
        sizeof(opt)                     // 参数长度
    );
    if(ret == -1){
        perror("setsockopt failed!");
        close(listen_fd_);
        return false;
    }

    // IP+端口
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr);

    // bind绑定一下
    if(bind(listen_fd_,(struct sockaddr*)&addr,sizeof(addr)) < 0){
        perror("bind failed!");
        close(listen_fd_);
        return false;
    }

    // listend监听
    listen(listen_fd_,128);


    std::cout << "服务器初始化成功！开始监听客户端!" << std::endl;

    return true;
}



// 处理单个客户端连接(子进程执行)
void ProcessTcpServer::handleClient(int cfd){
    char buf[BUF_SIZE];
    while(true){
        memset(buf,0,sizeof(buf));
        int len = recv(cfd,buf,sizeof(buf)-1,0);
        if(len <= 0){
            std::cout << "客户端断开，fd = " << cfd << std::endl;
            break;
        }
        std::cout << "客户端fd = " << cfd << "发送: " << buf << std::endl;
        send(cfd,"server: 已收到！",20,0);
    }

    close(cfd);
    exit(EXIT_SUCCESS);
}



// 子进程信号处理
void ProcessTcpServer::signChileHandler(int signo){
    while(waitpid(-1,nullptr,WNOHANG) > 0);
}



// 启动服务器
void ProcessTcpServer::start(){
    if(!initServer()){
        std::cout << "服务器初始化失败，请重新启动!" << std::endl;
    }


    // 注册子进程退出信号
    signal(SIGCHLD,signChileHandler);

    // 循环监听
    int count = 0;
    while(true){
        int client_fd_ = accept(listen_fd_,nullptr,nullptr);
        if(client_fd_ == -1){
            perror("client accpet failed!");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if(pid > 0){
            // 主进程关闭客户端fd即可
            close(client_fd_);
        }
        else if(pid == 0){
            // 子进程关闭fd
            close(listen_fd_);

            // 子进程处理
            handleClient(client_fd_);
        }
        else{
            perror("pid failed!");
            close(client_fd_);
        }

    }




}





