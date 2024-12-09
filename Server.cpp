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
        cout << "Server> Client connected!" << endl;

        thread([this, clientSocket]() {
            char buffer[1024];
            bool firstMessageReceived = false; 
            string username; 

            while (true)
            {
                memset(buffer, 0, sizeof(buffer));
                int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0)
                {
                    string leaveMessage =  username.substr(0, (username.length() / 2) - 1) + " left the server.";
                    broadcastMessage(leaveMessage, clientSocket);    

                    close(clientSocket);
                    lock_guard<mutex> lock(clientMutex);
                    clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
                    
                    cout << "Server> Client disconnected." << endl;
                    break;
                }

                if (!firstMessageReceived)
                {
                    username = buffer; 
                    string joinMessage = username.substr(0, (username.length() / 2) - 1) + " joined the server";
                    broadcastMessage(joinMessage, clientSocket); 
                    firstMessageReceived = true; 
                }
                else
                {
                    broadcastMessage(buffer, clientSocket); 
                }
            }
        }).detach();
    }
}

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
