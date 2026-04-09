#include "ThreadTcpServer.h"
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// 多线程头文件
#include <thread>
#include <mutex>


ThreadTcpServer::ThreadTcpServer(int port)
: port_(port), server_fd_(-1){
    
}



ThreadTcpServer::~ThreadTcpServer(){
    // 关闭
    if(server_fd_ > 0){
        close(server_fd_);
        std::cout << "服务器已经关闭!" << std::endl;
    }
}



void ThreadTcpServer::start_Server(){
    if(!init_Server()){
        std::cout << "服务器初始化失败!请重试!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "服务器启动成功，等待客户端请求..." << std::endl;

    while(true){
        int client_fd_ = accept(server_fd_,nullptr,nullptr);
        if(client_fd_ == -1){
            std::cout << "client_fd_有误" << std::endl;
            continue;
        }

        std::thread t(&ThreadTcpServer::handle_client,this,client_fd_);

        t.detach();

    }

}



// 服务器各项初始化
bool ThreadTcpServer::init_Server(){
    server_fd_ = socket(AF_INET,SOCK_STREAM,0);
    if(server_fd_ == -1){
        perror("socket failed!");
        return false;
    }

    // 端口复用关键参数
    int opt = 1;
    if(setsockopt(
        server_fd_,
        SOL_SOCKET,
        SO_REUSEADDR | SO_REUSEPORT,
        &opt,
        sizeof(opt)
    ) == -1){
        close(server_fd_);
        perror("setsockopt failed!");
        return false;
    }

    // ip+端口结构体
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr);
    
    // bind
    if(bind(server_fd_,(sockaddr *)&addr,sizeof(addr)) == -1){
        close(server_fd_);
        perror("bind failed!");
        return false;
    }

    // 监听
    if(listen(server_fd_,10) == -1){
        close(server_fd_);
        perror("listen failed!");
        return false;
    }


    // 初始化全部正确返回true
    return true;
}




void ThreadTcpServer::handle_client(int client_fd_){
    // 线程安全打印新连接
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "子线程处理客户端: " << client_fd_ << std::endl;
    }

    char buffer[size];
    while(true){
        memset(buffer, 0, sizeof(buffer));

        // 接收客户端消息
        ssize_t bytes_received = recv(client_fd_, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received > 0) {
            std::string received_msg(buffer);
            // 去除换行符
            if (!received_msg.empty() && received_msg.back() == '\n') {
                received_msg.pop_back();
            }

            {
                // 线程安全打印收到的消息
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "[客户端 " << client_fd_ << "] 收到: " << received_msg << std::endl;
            }

            // 回复客户端
            std::string response = "服务器已收到: " + received_msg + "\n";
            send(client_fd_, response.c_str(), response.length(), 0);

        } else if (bytes_received == 0) {
            // 客户端正常断开
            break;
        } else {
            // 接收出错
            std::lock_guard<std::mutex> lock(mtx);
            perror("recv failed!");
            break;
        }
    }

    close(client_fd_);

    {
        // 线程安全打印断开连接
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "[断开连接] 客户端 fd = " << client_fd_ << " 已断开" << std::endl;
    }

}