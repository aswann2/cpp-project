#include <iostream>
#include "Server.h"
#include "Client.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <server|client>" << endl;
        return 1;
    }

    string mode = argv[1];

    if (mode == "server")
    {
        Server server(8080);

        thread serverThread([&server]() {
            server.start();
        });

        cout << "Type '/shutdown' to close the server." << endl;

        while (true) {
            string input;
            getline(cin, input);

            if (input == "/shutdown") {
                cout << "Shutting down server..." << endl;
                server.shutdown(); 
                serverThread.join(); 
                break;
            }
        }
    }
    else if (mode == "client")
    {
        string username;
        cout << "Enter your username: ";
        cin >> username;

        Client client("127.0.0.1", 8080, username);
        client.connectToServer();
        client.start();
    }
    else
    {
        cerr << "Invalid mode. Use 'server' or 'client'." << endl;
        return 1;
    }

    return 0;
}

