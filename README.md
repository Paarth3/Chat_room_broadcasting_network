# A Group Chat Application

A real-time chat application capable of handling concurrent client connections via TCP/IP. This project implements a **Server-Client architecture** where the server manages multiple active socket connections and broadcasts messages to all participants in real-time.

The project demonstrates low-level networking concepts using the **Berkeley Socket API** and **multithreading** in **C++**.

## 📖 Project Overview

This application enables real-time communication between distinct clients. It acts as a digital "group chat" where:

1. **The Server** acts as the central hub, listening for incoming connections and routing data.    
2. **The Clients** connect to the server to send messages and receive updates from other users.

It is designed to be cross-platform (Windows and Linux), ensuring low-latency communication over the network.

## 🚀 Key Technical Learnings

Building this project focused on mastering core systems programming concepts. Key competencies demonstrated include:

- **Socket Programming:** Direct manipulation of the Berkeley Socket API (`socket`, `bind`, `listen`, `accept`) to manage network endpoints.
- **Concurrency & Threading:** Utilizing `std::thread` to handle multiple clients simultaneously without blocking the main server process.
- **Resource Management:** Implementing `std::mutex` to prevent race conditions when modifying shared resources (e.g., the list of active client sockets).
- **Custom Application Protocol:** Designing a **Length-Prefix framing protocol** to handle TCP stream fragmentation. This ensures the application knows exactly how many bytes to read for each message, preventing "sticky packet" issues.
- **Memory Manipulation:** Using raw byte buffers and manual memory copying (`std::memcpy`) to serialize and deserialize integer headers for network transmission.

## ⚙️ Technical Details

### Architecture

The system uses a **broadcast pattern**. When Client A sends a message:

1. The Server's dedicated thread for Client A receives the raw bytes.
2. The Server locks the global client list using a mutex.
3. The Server iterates through all other connected sockets and writes the data to them.
4. The mutex is unlocked.

### The Communication Protocol

TCP is a stream-oriented protocol, meaning message boundaries are not inherently preserved. To solve this, I implemented a strict framing logic:

1. **Header (4 Bytes):** Every message starts with a 4-byte integer representing the size of the incoming payload.
2. **Payload (N Bytes):** The actual text message.

The receiver first loops to read exactly 4 bytes, converts the network byte order (`ntohl`) to host byte order, allocates a buffer of that specific size, and then loops again to read the full payload.

### Tech Stack

- **Language:** C++
- **Networking:** `<sys/socket.h>`, `<netinet/in.h>` (Linux) / `<winsock2.h>`, `<ws2tcpip.h>` (Windows)
- **Concurrency:** `std::thread`, `std::mutex`

## 💻 How to Run

Follow these steps to set up the environment and run the chat system locally.

### 1. Compilation

Open your terminal/command prompt to the project directory and run the command specific to your OS.

| **Platform**      | **Component** | **Command**                                 | **Note**                      |
| ----------------- | ------------- | ------------------------------------------- | ----------------------------- |
| **Linux / macOS** | Server        | `g++ ChatServer.cpp -o server -pthread`     | Requires `-pthread`           |
|                   | Client        | `g++ ChatClient.cpp -o client -pthread`     |                               |
|                   |               |                                             |                               |
| **Windows**       | Server        | `g++ ChatServer.cpp -o server.exe -lws2_32` | Requires `-lws2_32` (Winsock) |
|                   | Client        | `g++ ChatClient.cpp -o client.exe -lws2_32` |                               |

### 2. Execution

The process is the same for all platforms.

**Step 1:** Start the Server -> Run the server executable first. It will begin listening on port 8080.

- **Linux/Mac:** `./server`
- **Windows:** `.\server.exe`

**Step 2:** Connect Clients -> Open a new terminal window for each client you want to connect.

- **Linux/Mac:** `./client`
- **Windows:** `.\client.exe`

**Step 3:** Chat -> Type a message in any client window and press Enter. The message will appear instantly in all other connected client windows.

### 3. Usage Notes

- **Exit:** Type `exit` to disconnect a client gracefully.
- **Localhost:** The default configuration connects to `127.0.0.1` (localhost). To chat across different computers on the same Wi-Fi, update the IP in `ChatClient.cpp` to the server's local IP address.

---

## 🔮 Future Improvements

- **Usernames:** Implement a handshake protocol to assign unique usernames upon connection.
- **GUI:** Migrate from console I/O to a graphical interface using Qt or ImGui.
- **Private Messaging:** Update logic to allow direct Client-to-Client messaging (routing via Server).
