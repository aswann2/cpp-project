#include "Server.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <thread>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <mutex>
#include <algorithm>

using namespace std;

// Constructor
Server::Server(int port)
{
    // Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    // Establish server address/info
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
}

// Deconstructor
Server::~Server()
{
    close(serverSocket);
    for (int client : clientSockets)
    {
        close(client);
    }
}

void Server::start()
{
    // Bind the socket
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    // Listen for incoming connections
    listen(serverSocket, 5);
    cout << "Server established." << endl;

    while (true)
    {
        // This loop allows for the server to continually connect new user clients
        addClient();
    }
}


// Accept new user client
void Server::addClient()
{
    sockaddr_in clientAddress;
    socklen_t clientAddrLen = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddrLen);

    if (clientSocket != -1)
    {
        // Add client socket to the list
        lock_guard<mutex> lock(clientMutex);
        clientSockets.push_back(clientSocket);
        cout << "Server> Client connected!" << endl;

        // Create thread for specific client communication
        thread([this, clientSocket]() {
            char buffer[1024];
            bool firstMessageReceived = false; 
            string username; 

            while (true)
            {
                // Receive message
                memset(buffer, 0, sizeof(buffer));
                int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0)
                {
                    // Client dc; broadcast leave msg
                    string leaveMessage =  username.substr(0, (username.length() / 2) - 1) + " left the server.";
                    broadcastMessage(leaveMessage, clientSocket);    

                    // Close socket and remove from list
                    close(clientSocket);
                    lock_guard<mutex> lock(clientMutex);
                    clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
                    
                    cout << "Server> Client disconnected." << endl;
                    break;
                }

                // Join message
                if (!firstMessageReceived)
                {
                    username = buffer; 
                    string joinMessage = username.substr(0, (username.length() / 2) - 1) + " joined the server";
                    broadcastMessage(joinMessage, clientSocket); 
                    firstMessageReceived = true; 
                }
                else
                {
                    // Broadcast to all connected user clients
                    broadcastMessage(buffer, clientSocket); 
                }
            }
        }).detach();
    }
}

// Broadcast message to all clients except sender
void Server::broadcastMessage(const string &message, int senderSocket)
{
    lock_guard<mutex> lock(clientMutex);
    string timestamp = getCurrentTime();
    string formattedMessage = timestamp + " " + message;
    for (int client : clientSockets)
    {
        if (client != senderSocket)
        {
            send(client, formattedMessage.c_str(), formattedMessage.length(), 0);
        }
    }
}

// Get current time for timestamps
string Server::getCurrentTime()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S]", timeinfo);
    return string(buffer);
}
