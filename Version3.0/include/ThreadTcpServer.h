#ifndef THREAD_TCP_SERVER_H
#define THREAD_TCP_SERVER_H

#include <mutex>


class ThreadTcpServer{
    public:
        ThreadTcpServer(int port);

        ~ThreadTcpServer();

        // 启动服务器
        void start_Server();

    private:
        int port_;          // 端口
        int server_fd_;     // 服务器文件描述符
        const int size = 1024; 
        std::mutex mtx;     // 互斥锁

        // 服务器初始化工作
        bool init_Server();

        // 子线程客户端处理请求函数
        void handle_client(int client_fd_);

};


#endif