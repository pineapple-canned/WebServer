#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
// 文件操作头文件
#include <fstream>
#include <sstream>


// ② 解析请求路径
char* get_http_path(char *buffer){
    // 按照空格拆分第一行: GET /index.html HTTP/1.1
    char* method = strtok(buffer," ");
    char* path = strtok(NULL," ");

    // 如果路径是 / 根目录，则补全 index.html
    if(strcmp(path,"/") == 0){
        return (char*)"index.html";
    }

    return path;
}

// ③ 发送本地HTML文件
void send_html(int client_fd,const char* file_path){
    std::ifstream ifst(file_path);
    
    // 判断是否打开成功
    if(!ifst.is_open()){
        // 返回 404 页面
        const char* not_found = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<h1>404 Not Found</h1>";
        send(client_fd, not_found, strlen(not_found), 0);
        return;
    }

    std::stringstream file_stream;
    file_stream << ifst.rdbuf();  // 把文件内容读入流
    std::string html_content = file_stream.str();    // 转成字符串

    // 构造标准 HTTP 响应头（必须严格遵守格式）
    std::string response_header = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " + std::to_string(html_content.size()) + "\r\n"
        "\r\n";

    send(client_fd,response_header.c_str(),response_header.size(),0);
    send(client_fd,html_content.c_str(),html_content.size(),0);

}



// ① 处理客户端请求
void handle_client(int client_fd){
    char buffer[1024] = {0};
    // 接收http请求
    recv(client_fd,buffer,sizeof(buffer),0);
    std::cout << "收到HTTP请求: \n" << buffer << std::endl;

    // ② 解析http请求路径
    char *path = get_http_path(buffer);
    std::cout << "请求文件: " << path << std::endl;

    // ③ 发送本地HTML文件
    send_html(client_fd,path);

    close(client_fd);
}




int main(int argc,char *argv[]){
    // 1. 创建socket
    int server_fd = socket(AF_INET,SOCK_STREAM,0);
    if(server_fd == -1){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. 设置setsockopt
    int opt = 1;
    int ret = setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR | SO_REUSEPORT,
        &opt,
        sizeof(opt)
    );
    if(ret == -1){
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // 3. 设置ip地址+端口号
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6060);
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr);

    // 4. bind
    if(bind(server_fd,(sockaddr *)&addr,sizeof(addr)) == -1){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 5. listen监听
    listen(server_fd,5);
    std::cout << "WebServer启动成功! http://127.0.0.1:6060" << std::endl;

    // 循环接收客户端
    while(true){
        int client_fd = accept(server_fd,nullptr,nullptr);
        if(client_fd == -1){
            perror("accept failed");
            continue;
        }

        // ① 处理请求
        handle_client(client_fd);
    }

    // 最后关闭服务端
    close(server_fd);

    return 0;
}