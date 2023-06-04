#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") // 链接ws2_32.lib库

int main() {
    // 初始化Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    // 创建Socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // 设置服务器地址和端口
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; // 使用任意可用的网络接口
    serverAddress.sin_port = htons(8080); // 使用端口8080

    // 绑定Socket到特定的IP地址和端口
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // 监听连接请求
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port 8080..." << std::endl;

    // 接受客户端连接
    SOCKET clientSocket;
    sockaddr_in clientAddress{};
    int clientAddressSize = sizeof(clientAddress);
    if ((clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressSize)) == INVALID_SOCKET) {
        std::cerr << "Failed to accept connection." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected." << std::endl;

    // 接收消息并发送响应
    char buffer[4096];
    int bytesReceived;
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        // 处理收到的数据
        std::string message(buffer, bytesReceived);
        std::cout << "Received message from client: " << message << std::endl;

        // 发送响应
        std::string response = "Hello from server!";
        send(clientSocket, response.c_str(), response.size(), 0);
    }

    // 关闭连接和Socket
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
