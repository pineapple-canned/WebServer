#ifndef Process_TCP_SERVER_H
#define Process_TCP_SERVER_H

#include <string>

class ProcessTcpServer{
    public:
        // 构造：指定监听端口
        ProcessTcpServer(const uint16_t port);

        // 析构：自动关闭套接字(RAII管理资源)
        ~ProcessTcpServer();

        // 启动服务器
        void start();

    private:
        // 属性
        uint16_t port_;     // 监听端口
        int listen_fd_;     // 监听套接字
        const int BUF_SIZE = 1024;

        // 初始化
        bool initServer();

        // 子进程信号处理
        static void signChileHandler(int signo);

        // 处理单个客户端连接(子进程执行)
        void handleClient(int cfd);

};





#endif