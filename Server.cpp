#include "Server.h"

/*
Set up the server
*/
Server::Server(int port)
{
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
}

/*
Close the server
*/
Server::~Server()
{
	close(serverSocket);
}

/*
Boot the server
*/
void Server::start()
{
	bind(serverSocket, (struct sockaddr*)&serverAddress, 
		sizeof(serverAddress));
	listen(serverSocket, 5);
}
