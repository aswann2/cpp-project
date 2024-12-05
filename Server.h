#include <iostream>
#include <cstring>
#include <vector>
#include <thread>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <mutex>

using namespace std;

class Server
{
public:
    Server(int port);
    ~Server();
    void start();
    void addClient();
    void broadcastMessage(const string &message, int senderSocket);
    string getCurrentTime();

    void shutdown() {
        close(serverSocket);
        for (int client : clientSockets) {
            close(client);
        }
        running = false;
    }

private:
    int serverSocket;
    sockaddr_in serverAddress;
    vector<int> clientSockets;
    mutex clientMutex;
    bool running = false;
};
