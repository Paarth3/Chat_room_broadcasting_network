#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <mutex>

int main(){

    int my_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (my_socket == -1){
        std::cerr << "Failed to create socket!" << std::endl;
        return 1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0){
        std::cerr << "Invalid address / Address not supported!" << std::endl;
        close(my_socket);
        return 1;
    }

    if (connect(my_socket, (struct sockaddr*)&address, sizeof(address)) == -1){
        std::cerr << "Failed to connect to the server!" << std::endl;
        close(my_socket);
        return 1;
    }

    return 0;
}