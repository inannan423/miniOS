//
// Created by OrangeJuice on 2023/6/4.
//
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
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // 设置服务器地址和端口
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器IP地址
    serverAddress.sin_port = htons(8080); // 服务器端口

    // 连接到服务器
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    // 循环发送消息给服务器
    while (true) {
        // 从用户输入中获取消息
        std::string message;
        std::cout << "Enter message (or 'exit' to quit): ";
        std::getline(std::cin, message);

        // 检查退出条件
        if (message == "exit")
            break;

        // 发送消息给服务器
        if (send(clientSocket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
            std::cerr << "Failed to send message to server." << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        // 接收服务器的响应
        char buffer[4096];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::string response(buffer, bytesReceived);
            std::cout << "Received response from server: " << response << std::endl;
        } else if (bytesReceived == 0) {
            std::cout << "Server disconnected." << std::endl;
            break;
        } else {
            std::cerr << "Failed to receive response from server." << std::endl;
            break;
        }
    }

    // 关闭连接和Socket
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
