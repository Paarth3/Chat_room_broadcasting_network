#include <iostream>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <mutex>

std::vector<int> client_sockets;
std::mutex mtx;

void handle_client(int client_socket){

}

int main(){

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1){
        std::cerr << "Failed to create the socket!" << std::endl;
        return 1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0){
        std::cerr << "Failed to bind!" << std::endl;
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 3) < 0){
        std::cerr << "Failed to listen!" << std::endl;
        close(server_socket);
        return 1;
    }

    std::cout << "Server listening to port 8080..." << std::endl;

    while (true){
        socklen_t addrlen = sizeof(address);
        int new_socket = accept(server_socket, (struct sockaddr*)&address, &addrlen);
        
        if (new_socket < 0){
            std::cerr << "Failed to accept!" << std::endl;
            continue;
        }

        mtx.lock();
        client_sockets.push_back(new_socket);
        mtx.unlock();

        std::thread t(handle_client, new_socket);
        t.detach();
    }

    return 0;
}