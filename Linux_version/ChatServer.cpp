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

    int NEEDED_REFERENCE_BYTES = 4;
    int received_reference_bytes = 0;
    char reference_buffer[NEEDED_REFERENCE_BYTES];

    while (received_reference_bytes < NEEDED_REFERENCE_BYTES){
        int reference_bytes = read(client_socket, reference_buffer + received_reference_bytes, NEEDED_REFERENCE_BYTES - received_reference_bytes);

        if (reference_bytes <= 0){
            close(client_socket);
            return;
        }

        received_reference_bytes += reference_bytes;
    }

    uint32_t network_number;
    std::memcpy(&network_number, reference_buffer, sizeof(uint32_t));

    int expected_bytes = ntohl(network_number);

    std::cout << "We expect " << expected_bytes << " bytes of data." << std::endl;

    int received_data_bytes = 0;
    std::vector<char> buffer;
    buffer.resize(expected_bytes);

    while (received_data_bytes < expected_bytes){
        int data_bytes = read(client_socket, buffer.data() + received_data_bytes, buffer.size());

        if (data_bytes <= 0){
            close(client_socket);
            return;
        }

        received_data_bytes += data_bytes;
    }

    std::cout << "Broadcasting message: " << std::string(buffer.begin(), buffer.end()) << std::endl;



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