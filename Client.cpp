#include "Client.h"
#include <iostream>
#include <cstring>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

Client::Client(const string &serverIP, int serverPort, const string &username)
    : serverIP(serverIP), serverPort(serverPort), username(username) {}

void Client::connectToServer()
{
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        cerr << "Error creating socket." << endl;
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr);

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        cerr << "Error connecting to server." << endl;
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    cout << "Connected to server as " << username << "." << endl;
    cout << "Please type '/quit' to disconnect." << endl;
}

void Client::start()
{
    send(clientSocket, username.c_str(), username.length(), 0);

    thread receiveThread([this]() { receiveMessages(); });

    while (true)
    {
        string message;
        getline(cin, message);

        if (message == "/quit")
        {
            cout << "Disconnected." << endl;
            close(clientSocket);
            exit(0);
        }

        send(clientSocket, message.c_str(), message.length(), 0);
    }
    receiveThread.join();
}


void Client::receiveMessages()
{
    char buffer[1024];
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0)
        {
            cerr << "Disconnected from server." << endl;
            close(clientSocket);
            exit(EXIT_FAILURE);
        }
        cout << buffer << endl; // Display received message
    }
}

