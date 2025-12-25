#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>

std::vector<SOCKET> client_sockets;
std::mutex mtx;

void handle_client(int client_socket){

    while (true){
        int NEEDED_REFERENCE_BYTES = 4;
        int received_reference_bytes = 0;
        char reference_buffer[NEEDED_REFERENCE_BYTES];
    
        while (received_reference_bytes < NEEDED_REFERENCE_BYTES){
            int reference_bytes = recv(client_socket, reference_buffer + received_reference_bytes, NEEDED_REFERENCE_BYTES - received_reference_bytes, 0);
    
            if (reference_bytes <= 0){
                closesocket(client_socket);
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
            int data_bytes = recv(client_socket, buffer.data() + received_data_bytes, buffer.size() - received_data_bytes, 0);
    
            if (data_bytes <= 0){
                closesocket(client_socket);
                return;
            }
    
            received_data_bytes += data_bytes;
        }
    
        std::cout << "Broadcasting message: " << std::string(buffer.begin(), buffer.end()) << std::endl;
    
        mtx.lock();
        for (int i = 0; i < client_sockets.size(); i++){
            if (client_sockets.at(i) != client_socket){
                uint32_t network_size_number = htonl(buffer.size()); 
                send(client_sockets.at(i), (char*)&network_size_number, 4, 0);
                send(client_sockets.at(i), buffer.data(), buffer.size(), 0);
            }
        }
        mtx.unlock();
    }
}

int main(){

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        std::cerr << "Failed to Start! \n";
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == INVALID_SOCKET){
        std::cerr << "Failed to create the socket!" << std::endl;
        return 1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR){
        std::cerr << "Failed to bind!" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    
    if (listen(server_socket, 3) == SOCKET_ERROR){
        std::cerr << "Failed to listen!" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening to port 8080..." << std::endl;

    while (true){
        int addrlen = sizeof(address);
        SOCKET new_socket = accept(server_socket, (struct sockaddr*)&address, &addrlen);
        
        if (new_socket == INVALID_SOCKET){
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
