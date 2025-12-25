#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <mutex>

void receive_messages(int server_socket){

    while(true){
        int NEEDED_REFERENCE_BYTES = 4;
        int received_reference_bytes = 0;
        char reference_buffer[NEEDED_REFERENCE_BYTES];

        while (received_reference_bytes < NEEDED_REFERENCE_BYTES){
            int reference_bytes = read(server_socket, reference_buffer + received_reference_bytes, NEEDED_REFERENCE_BYTES - received_reference_bytes);

            if (reference_bytes <= 0){
                close(server_socket);
                return;
            }

            received_reference_bytes += reference_bytes;
        }

        uint32_t network_number;
        std::memcpy(&network_number, reference_buffer, sizeof(uint32_t));

        int expected_bytes = ntohl(network_number);

        int received_data_bytes = 0;
        std::vector<char> buffer;
        buffer.resize(expected_bytes);

        while (received_data_bytes < expected_bytes){
            int data_bytes = read(server_socket, buffer.data() + received_data_bytes, buffer.size() - received_data_bytes);

            if (data_bytes <= 0){
                close(server_socket);
                return;
            }

            received_data_bytes += data_bytes;
        }

        std::cout << "\r\033[K";
        std::cout << "Message received: " << std::string(buffer.begin(), buffer.end()) << std::endl;
        std::cout << "Please enter your message (or \"exit\" to end the conversation): " << std::flush;
    }
}

int main(){

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1){
        std::cerr << "Failed to create socket!" << std::endl;
        return 1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0){
        std::cerr << "Invalid address / Address not supported!" << std::endl;
        close(server_socket);
        return 1;
    }

    if (connect(server_socket, (struct sockaddr*)&address, sizeof(address)) == -1){
        std::cerr << "Failed to connect to the server!" << std::endl;
        close(server_socket);
        return 1;
    }

    std::thread t(receive_messages, server_socket);
    t.detach();

    while (true){
        std::string user_message;
        std::cout << "Please enter your message (or \"exit\" to end the conversation): ";
        std::getline(std::cin >> std::ws, user_message);

        if (user_message == "exit"){
            close(server_socket);
            break;
        }

        int message_size = user_message.length();
        int network_message_size = htonl(message_size);

        send(server_socket, &network_message_size, 4, 0);
        send(server_socket, user_message.c_str(), message_size, 0);
    }

    return 0;
}