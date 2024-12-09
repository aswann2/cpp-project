#include "Client.h"
#include <iostream>
#include <cstring>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

// Constructor
Client::Client(const string &serverIP, int serverPort, const string &username)
    : serverIP(serverIP), serverPort(serverPort), username(username) {}

// Establish server connection
void Client::connectToServer()
{
    // Create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        cerr << "Error creating socket." << endl;
        exit(EXIT_FAILURE);
    }

    // Set server address/info
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr);

    // Connect
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        cerr << "Error connecting to server." << endl;
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    cout << "Connected to server as " << username << "." << endl;
}

void Client::start()
{
    // Send the username to the server as the first message
    send(clientSocket, username.c_str(), username.length(), 0);

    // Thread for receiving messages
    thread receiveThread([this]() { receiveMessages(); });

    // Main thread for sending messages
    string message;
    while (true)
    {
        getline(cin, message);
        if (message == "/quit")
        {
            cout << "Exiting chat..." << endl;
            close(clientSocket);
            exit(0);
        }

        string formattedMessage = username + ": " + message;
        send(clientSocket, formattedMessage.c_str(), formattedMessage.length(), 0);
    }

    receiveThread.join();
}

void Client::receiveMessages()
{
    char buffer[1024];
    while (true)
    {
        // Clear the buffer
        memset(buffer, 0, sizeof(buffer));
        
        // Receive msg
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0)
        {
            cerr << "Disconnected from server." << endl;
            close(clientSocket);
            exit(EXIT_FAILURE);
        }

        // Display message
        cout << buffer << endl;
    }
}

