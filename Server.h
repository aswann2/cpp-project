#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

class Server
{
public:
	Server(int port);
	~Server();
	void start();
	void addClient();
	void receiveMessage();

private:
	int serverSocket;
	int clientSocket;
	sockaddr_in serverAddress;
};
