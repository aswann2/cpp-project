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

Server::Server(int port)
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
}

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
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 5);
    cout << "Server established." << endl;

    while (true)
    {
        addClient();
    }
}

void Server::addClient()
{
    sockaddr_in clientAddress;
    socklen_t clientAddrLen = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddrLen);

    if (clientSocket != -1)
    {
        lock_guard<mutex> lock(clientMutex);
        clientSockets.push_back(clientSocket);
        cout << "Client connected!" << endl;

        thread([this, clientSocket]() {
            char buffer[1024];
            bool firstMessageReceived = false; // Flag to track if the first message has been received
            string username; // Variable to store the username

            while (true)
            {
                memset(buffer, 0, sizeof(buffer));
                int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0)
                {
                    close(clientSocket);
                    lock_guard<mutex> lock(clientMutex);
                    clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
                    cout << "Client disconnected." << endl;
                    break;
                }

                if (!firstMessageReceived)
                {
                    username = buffer; // Store the username from the first message
                    string joinMessage = username.substr(0, (username.length() / 2) - 1)+ " joined the server.";
                    broadcastMessage(joinMessage, clientSocket); // Broadcast the join message
                    firstMessageReceived = true; // Set the flag to true
                }
                else
                {
                    broadcastMessage(buffer, clientSocket); // Broadcast subsequent messages
                }
            }
        }).detach();
    }
}

void Server::broadcastMessage(const string &message, int senderSocket)
{
    lock_guard<mutex> lock(clientMutex);
    for (int client : clientSockets)
    {
        if (client != senderSocket) 
        {
            send(client, message.c_str(), message.length(), 0);
        }
    }
}
